/*
 * Created by v1tr10l7 on 17.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <stb_image.h>

#include <Vortex/Core/Assertions.hpp>
#include <Vortex/Core/Profiler.hpp>
#include <Vortex/Core/Types.hpp>
#include <Vortex/Renderer/ShaderLibrary.hpp>

#include <cstring>

import Prism.Containers.Vector;
import Prism.String.Formatter;
import Prism.Utility.Path;
import Vortex.Renderer;
import Vortex.Renderer.Environment;

namespace Vortex
{
    namespace
    {
        constexpr u32 kRadianceSize    = 1024;
        constexpr u32 kIrradianceSize  = 32;
        constexpr u32 kPrefilteredSize = 256;
        constexpr u32 kPrefilteredMips = 6;
        constexpr u32 kBRDFLutSize     = 512;

        ::Ref<RHI::Texture> s_BRDFLut;

        /// IEEE 754 binary32 -> binary16, round to nearest, clamped to the
        /// half range (HDRs can exceed 65504).
        u16 ToHalf(f32 value)
        {
            u32 bits;
            std::memcpy(&bits, &value, sizeof(bits));
            u32 sign     = (bits >> 16) & 0x8000u;
            i32 exponent = static_cast<i32>((bits >> 23) & 0xffu) - 127 + 15;
            u32 mantissa = bits & 0x7fffffu;

            if (exponent <= 0)
            {
                // Too small for a normal half: flush to zero (denormals don't
                // matter for lighting data).
                return static_cast<u16>(sign);
            }
            if (exponent >= 31)
                return static_cast<u16>(sign | 0x7bffu); // max finite half
            u32 half = sign | (static_cast<u32>(exponent) << 10) | (mantissa >> 13);
            if (mantissa & 0x1000u) ++half; // round
            return static_cast<u16>(half);
        }

        u32 Groups(u32 size, u32 local) { return (size + local - 1) / local; }

        ::Ref<RHI::Pipeline> ComputePipeline(StringView name, StringView path)
        {
            auto& library = Renderer::ShaderLibrary();
            if (!library.Contains(name) && !library.Load(name, path))
                return nullptr;
            RHI::ComputePipelineDesc desc{};
            desc.Shader    = library.Get(name);
            desc.DebugName = String(name);
            return Renderer::Pipelines().Get(desc);
        }

        ::Ref<RHI::Texture> CreateCube(u32 size, u32 mips, StringView name,
                                       RHI::TextureUsage extraUsage)
        {
            RHI::TextureDesc desc{};
            desc.Type      = RHI::TextureType::eCube;
            desc.Format    = RHI::Format::eRGBA16Float;
            desc.Extent    = {size, size, 1};
            desc.Layers    = 6;
            desc.MipLevels = mips;
            desc.Usage     = RHI::TextureUsage::eSampled
                       | RHI::TextureUsage::eStorage | extraUsage;
            desc.DebugName = String(name);
            return Renderer::Device().CreateTexture(desc);
        }

        ::Ref<RHI::BindGroup> Group(RHI::Pipeline&                  pipeline,
                                    Vector<RHI::BindGroupEntry> entries)
        {
            RHI::BindGroupDesc desc{};
            desc.Layout  = pipeline.Shader()->SetLayout(0);
            desc.Entries = Move(entries);
            return Renderer::Device().CreateBindGroup(desc);
        }
    } // namespace

    ::Ref<Environment> Environment::Load(PathView hdrPath)
    {
        VtProfileFunction();
        Path path(hdrPath);
        i32  width = 0, height = 0, channels = 0;
        f32* pixels = stbi_loadf(path.Raw(), &width, &height, &channels, 4);
        if (!pixels)
        {
            VtCoreError("Environment: Can't load '{}': {}", hdrPath,
                        stbi_failure_reason());
            return nullptr;
        }

        // Half floats: linear filtering of RGBA32F isn't guaranteed.
        usize       texels = static_cast<usize>(width) * height * 4;
        Vector<u16> halves(texels);
        for (usize i = 0; i < texels; ++i) halves[i] = ToHalf(pixels[i]);
        stbi_image_free(pixels);

        auto& device = Renderer::Device();
        RHI::TextureDesc equirectDesc{};
        equirectDesc.Format    = RHI::Format::eRGBA16Float;
        equirectDesc.Extent    = {static_cast<u32>(width), static_cast<u32>(height), 1};
        equirectDesc.DebugName = Prism::Format("{} (equirect)", hdrPath);
        auto equirect = device.CreateTexture(equirectDesc, halves.Raw(),
                                             halves.Size() * sizeof(u16));

        auto toCube     = ComputePipeline("IBL.EquirectToCube",
                                          "assets/shaders/ibl/equirect_to_cube.hlsl");
        auto irradiance = ComputePipeline("IBL.Irradiance",
                                          "assets/shaders/ibl/irradiance.hlsl");
        auto prefilter  = ComputePipeline("IBL.Prefilter",
                                          "assets/shaders/ibl/prefilter.hlsl");
        if (!toCube || !irradiance || !prefilter) return nullptr;

        auto environment            = CreateRef<Environment>();
        environment->m_Radiance     = CreateCube(
            kRadianceSize, RHI::FullMipChain({kRadianceSize, kRadianceSize}),
            Prism::Format("{} (radiance)", hdrPath),
            RHI::TextureUsage::eTransferSrc | RHI::TextureUsage::eTransferDst);
        environment->m_Irradiance   = CreateCube(
            kIrradianceSize, 1, Prism::Format("{} (irradiance)", hdrPath),
            RHI::TextureUsage::eNone);
        environment->m_Prefiltered  = CreateCube(
            kPrefilteredSize, kPrefilteredMips,
            Prism::Format("{} (prefiltered)", hdrPath), RHI::TextureUsage::eNone);

        auto radiance    = environment->m_Radiance;
        auto diffuse     = environment->m_Irradiance;
        auto specular    = environment->m_Prefiltered;
        auto sampler     = Renderer::LinearClampSampler();

        // Groups outlive the recording; keep them until the work is done.
        Vector<::Ref<RHI::BindGroup>> groups;
        groups.PushBack(Group(*toCube,
                              {RHI::BindGroupEntry::SampledTexture(0, equirect, sampler),
                               RHI::BindGroupEntry::StorageTexture(1, radiance, 0)}));
        groups.PushBack(Group(*irradiance,
                              {RHI::BindGroupEntry::SampledTexture(0, radiance, sampler),
                               RHI::BindGroupEntry::StorageTexture(1, diffuse, 0)}));
        for (u32 mip = 0; mip < kPrefilteredMips; ++mip)
            groups.PushBack(Group(*prefilter,
                                  {RHI::BindGroupEntry::SampledTexture(0, radiance, sampler),
                                   RHI::BindGroupEntry::StorageTexture(1, specular, mip)}));

        Renderer::ExecuteImmediate(
            [&](RHI::CommandList& cmd)
            {
                RHI::ScopedLabel label(cmd, "Bake environment");

                // Equirect -> cube, then the radiance mip chain.
                cmd.Transition(*radiance, RHI::ResourceState::eShaderWrite);
                cmd.BindPipeline(*toCube);
                cmd.BindGroup(0, *groups[0]);
                cmd.Dispatch(Groups(kRadianceSize, 16), Groups(kRadianceSize, 16), 6);
                cmd.Transition(*radiance, RHI::ResourceState::eTransferDst);
                cmd.GenerateMips(*radiance, RHI::ResourceState::eShaderRead);

                cmd.Transition(*diffuse, RHI::ResourceState::eShaderWrite);
                cmd.BindPipeline(*irradiance);
                cmd.BindGroup(0, *groups[1]);
                cmd.Dispatch(Groups(kIrradianceSize, 8), Groups(kIrradianceSize, 8), 6);
                cmd.Transition(*diffuse, RHI::ResourceState::eShaderRead);

                cmd.Transition(*specular, RHI::ResourceState::eShaderWrite);
                cmd.BindPipeline(*prefilter);
                for (u32 mip = 0; mip < kPrefilteredMips; ++mip)
                {
                    struct
                    {
                        f32 Roughness;
                        f32 RadianceSize;
                    } params{static_cast<f32>(mip) / (kPrefilteredMips - 1),
                             static_cast<f32>(kRadianceSize)};
                    u32 size = kPrefilteredSize >> mip;
                    cmd.BindGroup(0, *groups[2 + mip]);
                    cmd.PushConstants(RHI::ShaderStage::eCompute, params);
                    cmd.Dispatch(Groups(size, 8), Groups(size, 8), 6);
                }
                cmd.Transition(*specular, RHI::ResourceState::eShaderRead);
            });

        VtCoreInfo("Environment: Baked '{}' ({}x{})", hdrPath, width, height);
        return environment;
    }

    ::Ref<Environment> Environment::Uniform(f32 r, f32 g, f32 b)
    {
        auto  environment = CreateRef<Environment>();
        auto& device      = Renderer::Device();

        u16 texel[4] = {ToHalf(r), ToHalf(g), ToHalf(b), ToHalf(1.0f)};
        auto cube    = [&](StringView name)
        {
            RHI::TextureDesc desc{};
            desc.Type      = RHI::TextureType::eCube;
            desc.Format    = RHI::Format::eRGBA16Float;
            desc.Extent    = {1, 1, 1};
            desc.Layers    = 6;
            desc.Usage     = RHI::TextureUsage::eSampled | RHI::TextureUsage::eTransferDst;
            desc.DebugName = String(name);
            auto texture   = device.CreateTexture(desc);
            for (u32 layer = 0; layer < 6; ++layer)
            {
                RHI::TextureRegion region{};
                region.Layer = layer;
                device.UploadTexture(*texture, texel, sizeof(texel), region);
            }
            return texture;
        };
        environment->m_Radiance    = cube("Uniform environment (radiance)");
        environment->m_Irradiance  = cube("Uniform environment (irradiance)");
        environment->m_Prefiltered = cube("Uniform environment (prefiltered)");
        return environment;
    }

    ::Ref<RHI::Texture> Environment::BRDFLut()
    {
        if (s_BRDFLut) return s_BRDFLut;
        VtProfileFunction();

        auto pipeline = ComputePipeline("IBL.BRDFLut", "assets/shaders/ibl/brdf_lut.hlsl");
        VtCoreAssertMsg(pipeline, "Environment: Can't build the BRDF lookup table");

        RHI::TextureDesc desc{};
        desc.Format = RHI::Format::eRGBA16Float;
        desc.Extent = {kBRDFLutSize, kBRDFLutSize, 1};
        desc.Usage  = RHI::TextureUsage::eSampled | RHI::TextureUsage::eStorage;
        desc.DebugName = "BRDF LUT";
        auto lut       = Renderer::Device().CreateTexture(desc);
        auto group     = Group(*pipeline, {RHI::BindGroupEntry::StorageTexture(0, lut, 0)});

        Renderer::ExecuteImmediate(
            [&](RHI::CommandList& cmd)
            {
                cmd.Transition(*lut, RHI::ResourceState::eShaderWrite);
                cmd.BindPipeline(*pipeline);
                cmd.BindGroup(0, *group);
                cmd.Dispatch(Groups(kBRDFLutSize, 16), Groups(kBRDFLutSize, 16), 1);
                cmd.Transition(*lut, RHI::ResourceState::eShaderRead);
            });
        s_BRDFLut = lut;
        return s_BRDFLut;
    }

    void Environment::ReleaseShared() { s_BRDFLut = nullptr; }
} // namespace Vortex

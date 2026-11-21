/*
 * Created by v1tr10l7 on 15.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Vortex/Core/Assertions.hpp>
#include <Vortex/Core/Types.hpp>

module Vortex.RHI.Device;

import Prism.Containers.Array;
import Prism.String.Formatter;

namespace Vortex::RHI
{
    namespace
    {
        constexpr usize                  kAPICount = 1;
        Array<DeviceFactory, kAPICount>  s_Factories{};
        Device*                          s_Device = nullptr;
    } // namespace

    void Device::RegisterAPI(RHI::API api, DeviceFactory factory)
    {
        usize index = static_cast<usize>(api);
        VtCoreAssertMsg(index < kAPICount, "RHI: Invalid API");
        s_Factories[index] = factory;
    }

    Expected<Scope<Device>, String> Device::Create(const DeviceDesc& desc)
    {
        if (s_Device) return Unexpected(String("RHI: A device already exists"));
        if (desc.FramesInFlight == 0)
            return Unexpected(String("RHI: FramesInFlight must be at least 1"));

        usize index = static_cast<usize>(desc.API);
        if (index >= kAPICount || !s_Factories[index])
            return Unexpected(String("RHI: No backend registered for the API"));

        return s_Factories[index](desc);
    }

    Device& Device::Get()
    {
        VtCoreAssertMsg(s_Device, "RHI: No device; call Device::Create first");
        return *s_Device;
    }
    bool Device::Exists() { return s_Device != nullptr; }

    Device::Device(const DeviceDesc& desc)
        : m_Desc(desc)
    {
        VtCoreAssertMsg(!s_Device, "RHI: A device already exists");
        s_Device = this;
    }
    Device::~Device()
    {
        if (s_Device == this) s_Device = nullptr;
    }

    ::Ref<Buffer> Device::CreateBuffer(const BufferDesc& desc, const void* data)
    {
        BufferDesc actual = desc;
        if (actual.Memory == MemoryUsage::eGpuOnly)
            actual.Usage |= BufferUsage::eTransferDst;

        auto buffer = CreateBuffer(actual);
        if (!buffer || !data) return buffer;

        if (buffer->IsHostVisible()) buffer->Write(data, actual.Size);
        else UploadBuffer(*buffer, data, actual.Size);
        return buffer;
    }

    ::Ref<Texture> Device::CreateTexture(const TextureDesc& desc,
                                         const void* data, usize size)
    {
        TextureDesc actual = desc;
        actual.Usage |= TextureUsage::eTransferDst;
        if (actual.MipLevels > 1) actual.Usage |= TextureUsage::eTransferSrc;

        auto texture = CreateTexture(actual);
        if (!texture || !data) return texture;

        if (actual.MipLevels == 1)
        {
            UploadTexture(*texture, data, size);
            return texture;
        }
        // Leave mip 0 as a transfer destination and blit the rest from it.
        UploadTexture(*texture, data, size, {}, ResourceState::eTransferDst);
        GenerateMips(*texture);
        return texture;
    }
} // namespace Vortex::RHI

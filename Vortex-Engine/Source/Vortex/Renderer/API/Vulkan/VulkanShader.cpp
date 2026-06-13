/*
 * Created by v1tr10l7 on 28.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Renderer/API/Vulkan/VulkanRenderer.hpp>
#include <Vortex/Renderer/API/Vulkan/VulkanShader.hpp>
#include <Vortex/Renderer/Renderer.hpp>

#include <libshaderc_util/file_finder.h>
#include <shaderc/shaderc.hpp>
#include <spirv-tools/libspirv.h>
#include <spirv_glsl.hpp>

namespace Vortex
{
    namespace
    {
        shaderc::Compiler       s_Compiler;
        vk::ShaderStageFlagBits StringToShaderStage(StringView stage)
        {
            if (stage == "vertex") return vk::ShaderStageFlagBits::eVertex;
            else if (stage == "fragment")
                return vk::ShaderStageFlagBits::eFragment;

            VtCoreAssertMsg(false, "unknown shader stage");
            return vk::ShaderStageFlagBits(0);
        }
        shaderc_shader_kind VulkanToShadercStage(vk::ShaderStageFlagBits stage)
        {
            if (stage == vk::ShaderStageFlagBits::eVertex)
                return shaderc_vertex_shader;
            if (stage == vk::ShaderStageFlagBits::eFragment)
                return shaderc_fragment_shader;
            return shaderc_glsl_infer_from_source;
        }
    } // namespace

    VulkanShader::VulkanShader(PathView path)
    {
        m_Name                = path.BaseName();
        String     shaderCode = ReadShaderCode(path);
        auto       sources    = Preprocess(shaderCode);
        vk::Device device     = VulkanRenderer::Device();

        for (auto& [stage, source] : sources)
        {
            Vector<u32> shaderBinary;
            CompileShaderCode(source, path, stage, shaderBinary);
            Reflect(path, stage, shaderBinary);

            vk::ShaderModuleCreateInfo shaderInfo{};
            shaderInfo.sType    = vk::StructureType::eShaderModuleCreateInfo;
            shaderInfo.codeSize = shaderBinary.Size() * sizeof(u32);
            shaderInfo.pCode = reinterpret_cast<const u32*>(shaderBinary.Raw());

            if (stage == vk::ShaderStageFlagBits::eVertex)
            {
                VkCall(device.createShaderModule(&shaderInfo, VK_NULL_HANDLE,
                                                 &m_VertexShader));
            }
            else if (stage == vk::ShaderStageFlagBits::eFragment)
                VkCall(device.createShaderModule(&shaderInfo, VK_NULL_HANDLE,
                                                 &m_FragmentShader));
        }

        u32 maxFramesInFlight = Renderer::Configuration().MaxFramesInFlight;
        for (auto&& [set, shaderDescriptorSet] : m_DescriptorSets)
        {
            if (shaderDescriptorSet.UniformDescriptors.Size())
            {
                vk::DescriptorPoolSize& typeCount
                    = m_DescriptorSets[set].PoolSizes.EmplaceBack();
                typeCount.type = vk::DescriptorType::eUniformBuffer;
                typeCount.descriptorCount
                    = shaderDescriptorSet.UniformDescriptors.Size()
                    * maxFramesInFlight;
                VtCoreTrace("size: {}", typeCount.descriptorCount);
            }
            if (shaderDescriptorSet.ImageSamplers.Size())
            {
                vk::DescriptorPoolSize& typeCount
                    = m_DescriptorSets[set].PoolSizes.EmplaceBack();
                typeCount.type = vk::DescriptorType::eCombinedImageSampler;
                typeCount.descriptorCount
                    = shaderDescriptorSet.ImageSamplers.Size()
                    * maxFramesInFlight;
                VtCoreTrace("size: {}", typeCount.descriptorCount);
            }

            Vector<vk::DescriptorSetLayoutBinding> layoutBindings;
            for (auto& [binding, uniformBuffer] :
                 shaderDescriptorSet.UniformDescriptors)
            {
                vk::DescriptorSetLayoutBinding& layoutBinding
                    = layoutBindings.EmplaceBack();
                layoutBinding.descriptorType
                    = vk::DescriptorType::eUniformBuffer;
                layoutBinding.descriptorCount = uniformBuffer.DescriptorCount;
                layoutBinding.stageFlags      = uniformBuffer.ShaderStage;
                layoutBinding.pImmutableSamplers = VK_NULL_HANDLE;
                layoutBinding.binding            = binding;

                vk::WriteDescriptorSet& set
                    = shaderDescriptorSet
                          .WriteDescriptorSets[uniformBuffer.Name];
                set                 = vk::WriteDescriptorSet();
                set.sType           = vk::StructureType::eWriteDescriptorSet;
                set.descriptorType  = layoutBinding.descriptorType;
                set.descriptorCount = layoutBinding.descriptorCount;
                set.dstBinding      = layoutBinding.binding;
            }
            for (auto& [binding, sampler] : shaderDescriptorSet.ImageSamplers)
            {
                vk::DescriptorSetLayoutBinding& layoutBinding
                    = layoutBindings.EmplaceBack();
                layoutBinding.descriptorType
                    = vk::DescriptorType::eCombinedImageSampler;
                layoutBinding.descriptorCount    = sampler.DescriptorCount;
                layoutBinding.stageFlags         = sampler.ShaderStage;
                layoutBinding.pImmutableSamplers = VK_NULL_HANDLE;
                layoutBinding.binding            = binding;

                vk::WriteDescriptorSet& set
                    = shaderDescriptorSet.WriteDescriptorSets[sampler.Name];
                set                 = vk::WriteDescriptorSet();
                set.sType           = vk::StructureType::eWriteDescriptorSet;
                set.descriptorType  = layoutBinding.descriptorType;
                set.descriptorCount = layoutBinding.descriptorCount;
                set.dstBinding      = layoutBinding.binding;
            }

            VtCoreInfo("Creating descriptor set {} with {} ubos", set,
                       shaderDescriptorSet.UniformDescriptors.Size());
            VtCoreInfo("Creating descriptor set {} with {} ubos", set,
                       shaderDescriptorSet.ImageSamplers.Size());

            vk::DescriptorSetLayoutCreateInfo createInfo{};
            createInfo.sType
                = vk::StructureType::eDescriptorSetLayoutCreateInfo;
            createInfo.pNext        = VK_NULL_HANDLE;
            createInfo.flags        = vk::DescriptorSetLayoutCreateFlags();
            createInfo.bindingCount = layoutBindings.Size();
            createInfo.pBindings    = layoutBindings.Raw();

            vk::DescriptorSetLayout& descriptorSetLayout
                = m_DescriptorSetLayouts.EmplaceBack();
            VkCall(device.createDescriptorSetLayout(&createInfo, VK_NULL_HANDLE,
                                                    &descriptorSetLayout));
        }

        for (auto& set : m_DescriptorSets)
        {
            vk::DescriptorPoolCreateInfo poolInfo{};
            poolInfo.sType   = vk::StructureType::eDescriptorPoolCreateInfo;
            poolInfo.pNext   = VK_NULL_HANDLE;
            poolInfo.flags   = vk::DescriptorPoolCreateFlags();
            poolInfo.maxSets = maxFramesInFlight;
            poolInfo.poolSizeCount             = set.Value.PoolSizes.Size();
            poolInfo.pPoolSizes                = set.Value.PoolSizes.Raw();

            vk::DescriptorPool& descriptorPool = set.Value.Pool;
            VkCall(device.createDescriptorPool(&poolInfo, VK_NULL_HANDLE,
                                               &descriptorPool));
        }

        for (auto& descriptorSet : m_DescriptorSets)
        {
            vk::DescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = vk::StructureType::eDescriptorSetAllocateInfo;
            allocInfo.descriptorPool = m_DescriptorSets[descriptorSet.Key].Pool;
            allocInfo.descriptorSetCount = maxFramesInFlight;

            Vector<vk::DescriptorSetLayout> layouts(
                maxFramesInFlight, m_DescriptorSetLayouts[descriptorSet.Key]);
            allocInfo.pSetLayouts = layouts.Raw();

            descriptorSet.Value.Sets.Resize(maxFramesInFlight);
            VkCall(device.allocateDescriptorSets(
                &allocInfo, descriptorSet.Value.Sets.Raw()));
        }
    }
    VulkanShader::VulkanShader(PathView vertexPath, PathView fragmentPath)
    {
        m_Name = vertexPath.BaseName();
        Vector<u32> vertexShaderBinary;
        Vector<u32> fragmentShaderBinary;

        auto        vertexCode   = ReadShaderCode(vertexPath);
        auto        fragmentCode = ReadShaderCode(fragmentPath);
        CompileShaderCode(vertexCode, vertexPath,
                          vk::ShaderStageFlagBits::eVertex, vertexShaderBinary);
        CompileShaderCode(fragmentCode, fragmentPath,
                          vk::ShaderStageFlagBits::eFragment,
                          fragmentShaderBinary);

        Reflect(vertexPath, vk::ShaderStageFlagBits::eVertex,
                vertexShaderBinary);
        Reflect(fragmentPath, vk::ShaderStageFlagBits::eFragment,
                fragmentShaderBinary);

        vk::ShaderModuleCreateInfo vertexShaderInfo{};
        vertexShaderInfo.sType    = vk::StructureType::eShaderModuleCreateInfo;
        vertexShaderInfo.codeSize = vertexShaderBinary.Size() * sizeof(u32);
        vertexShaderInfo.pCode
            = reinterpret_cast<const u32*>(vertexShaderBinary.Raw());

        vk::ShaderModuleCreateInfo fragmentShaderInfo{};
        fragmentShaderInfo.sType = vk::StructureType::eShaderModuleCreateInfo;
        fragmentShaderInfo.codeSize = fragmentShaderBinary.Size() * sizeof(u32);
        fragmentShaderInfo.pCode
            = reinterpret_cast<const u32*>(fragmentShaderBinary.Raw());

        vk::Device device = VulkanRenderer::Device();
        VkCall(device.createShaderModule(&vertexShaderInfo, VK_NULL_HANDLE,
                                         &m_VertexShader));
        VkCall(device.createShaderModule(&fragmentShaderInfo, VK_NULL_HANDLE,
                                         &m_FragmentShader));

        for (auto&& [set, shaderDescriptorSet] : m_DescriptorSets)
        {
            if (shaderDescriptorSet.UniformDescriptors.Size())
            {
                vk::DescriptorPoolSize& typeCount
                    = m_DescriptorSets[set].PoolSizes.EmplaceBack();
                typeCount.type = vk::DescriptorType::eUniformBuffer;
                typeCount.descriptorCount
                    = shaderDescriptorSet.UniformDescriptors.Size();
                VtCoreTrace("size: {}", typeCount.descriptorCount);
            }

            Vector<vk::DescriptorSetLayoutBinding> layoutBindings;
            for (auto& [binding, uniformBuffer] :
                 shaderDescriptorSet.UniformDescriptors)
            {
                vk::DescriptorSetLayoutBinding& layoutBinding
                    = layoutBindings.EmplaceBack();
                layoutBinding.descriptorType
                    = vk::DescriptorType::eUniformBuffer;
                layoutBinding.descriptorCount    = 1;
                layoutBinding.stageFlags         = uniformBuffer.ShaderStage;
                layoutBinding.pImmutableSamplers = nullptr;
                layoutBinding.binding            = binding;

                vk::WriteDescriptorSet& set
                    = shaderDescriptorSet
                          .WriteDescriptorSets[uniformBuffer.Name];
                set                 = vk::WriteDescriptorSet();
                set.sType           = vk::StructureType::eWriteDescriptorSet;
                set.descriptorType  = layoutBinding.descriptorType;
                set.descriptorCount = 1;
                set.dstBinding      = layoutBinding.binding;
            }

            VtCoreInfo("Creating descriptor set {} with {} ubos", set,
                       shaderDescriptorSet.UniformDescriptors.Size());

            vk::DescriptorSetLayoutCreateInfo createInfo{};
            createInfo.sType
                = vk::StructureType::eDescriptorSetLayoutCreateInfo;
            createInfo.pNext        = VK_NULL_HANDLE;
            createInfo.flags        = vk::DescriptorSetLayoutCreateFlags();
            createInfo.bindingCount = layoutBindings.Size();
            createInfo.pBindings    = layoutBindings.Raw();

            VkCall(device.createDescriptorSetLayout(
                &createInfo, VK_NULL_HANDLE, &m_DescriptorSetLayouts[set]));
        }
    }
    VulkanShader::~VulkanShader()
    {
        vk::Device device = VulkanRenderer::Device();
        for (auto& set : m_DescriptorSets)
            device.destroyDescriptorPool(set.Value.Pool);
        for (auto layout : m_DescriptorSetLayouts)
            device.destroyDescriptorSetLayout(layout, VK_NULL_HANDLE);
        device.destroyShaderModule(m_VertexShader, VK_NULL_HANDLE);
        device.destroyShaderModule(m_FragmentShader, VK_NULL_HANDLE);
    }

    void VulkanShader::SetUniform(const String&        name,
                                  ::Ref<UniformBuffer> uniform)
    {
        auto                    buffer = uniform.As<VulkanUniformBuffer>();
        vk::Device              device = VulkanRenderer::Device();

        vk::WriteDescriptorSet& descriptorWrite
            = m_DescriptorSets[0].WriteDescriptorSets[name];

        for (usize i = 0; i < Renderer::Configuration().MaxFramesInFlight; i++)
        {
            descriptorWrite.dstSet          = m_DescriptorSets[0].Sets[i];
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
            descriptorWrite.pBufferInfo    = &buffer->BufferInfos()[i];

            device.updateDescriptorSets(1, &descriptorWrite, 0, VK_NULL_HANDLE);
        }
    }
    void VulkanShader::SetUniform(const String& name, ::Ref<Texture2D> texture)
    {
        vk::Device              device = VulkanRenderer::Device();
        vk::WriteDescriptorSet& descriptorWrite
            = m_DescriptorSets[0].WriteDescriptorSets[name];

        auto vulkanTexture     = texture.As<VulkanTexture2D>();
        u32  maxFramesInFlight = Renderer::Configuration().MaxFramesInFlight;
        for (usize i = 0; i < maxFramesInFlight; i++)
        {
            descriptorWrite.dstSet          = m_DescriptorSets[0].Sets[i];
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType
                = vk::DescriptorType::eCombinedImageSampler;
            descriptorWrite.pImageInfo = &vulkanTexture->ImageInfo();

            device.updateDescriptorSets(1, &descriptorWrite, 0, VK_NULL_HANDLE);
        }
    }

    UnorderedMap<vk::ShaderStageFlagBits, String>
    VulkanShader::Preprocess(StringView shaderCode)
    {
        UnorderedMap<vk::ShaderStageFlagBits, String> shaderStages;

        static constexpr auto                         npos = String::NPos;
        auto getShaderStage = [](StringView source) -> StringView
        {
            static constexpr StringView STAGE_DIRECTIVE_STRING
                = "#pragma shader_stage";

            usize stageDirectiveStart = source.Find(STAGE_DIRECTIVE_STRING);
            VtCoreAssert(stageDirectiveStart != npos);

            usize stageNameStart
                = stageDirectiveStart + STAGE_DIRECTIVE_STRING.Size();
            while (!StringUtils::IsAlpha(source[stageNameStart])
                   && stageNameStart < source.Size())
                ++stageNameStart;

            usize stageNameEnd = stageNameStart;
            while (StringUtils::IsAlpha(source[stageNameEnd])
                   && stageNameEnd < source.Size())
                ++stageNameEnd;

            return source.Substr(stageNameStart, stageNameEnd - stageNameStart);
        };

        static constexpr StringView VERSION_DIRECTIVE_STRING = "#version";
        usize stageStart = shaderCode.Find(VERSION_DIRECTIVE_STRING);
        while (stageStart != npos)
        {

            StringView stageString
                = getShaderStage(shaderCode.Substr(stageStart));

            usize nextStageStart
                = shaderCode.Find(VERSION_DIRECTIVE_STRING, stageStart + 1);

            vk::ShaderStageFlagBits currentStage
                = StringToShaderStage(stageString);

            auto shaderStage
                = (nextStageStart != npos)
                    ? shaderCode.Substr(stageStart, nextStageStart - stageStart)
                    : shaderCode.Substr(stageStart);

            shaderStages[currentStage] = shaderStage;
            stageStart                 = nextStageStart;
        }

        return shaderStages;
    }
    void VulkanShader::Reflect(StringView path, vk::ShaderStageFlagBits stage,
                               const Vector<u32>& shaderBinary)
    {
        spirv_cross::Compiler compiler(shaderBinary.Raw(), shaderBinary.Size());
        auto                  resources = compiler.get_shader_resources();
        (void)path;

        VtCoreTrace("===========================");
        VtCoreTrace(" Vulkan Shader Reflection");
        VtCoreTrace(" {0}", path);
        VtCoreTrace("===========================");

        VtCoreTrace("Uniform Buffers:");
        std::set<int> sets;
        std::set<int> bindings;

        for (usize i = 0; i < resources.uniform_buffers.size(); i++)
        {
            const auto& resource   = resources.uniform_buffers[i];

            const auto& name       = resource.name;
            auto&       bufferType = compiler.get_type(resource.type_id);
            u32         set        = compiler.get_decoration(resource.id,
                                                             spv::DecorationDescriptorSet);
            u32         binding
                = compiler.get_decoration(resource.id, spv::DecorationBinding);
            u32 descriptorCount = 1;
            for (auto size : bufferType.array) descriptorCount *= size;
            i32 memberCount = bufferType.member_types.size();
            u32 size        = compiler.get_declared_struct_size(bufferType);

            bindings.insert(binding);
            sets.insert(set);
            auto&             shaderDescriptorSet = m_DescriptorSets[set];

            UniformDescriptor uniform;
            uniform.Name            = StringView(name.data(), name.size());
            uniform.BindingPoint    = binding;
            uniform.DescriptorCount = descriptorCount;
            uniform.Size            = size;
            uniform.ShaderStage     = stage;
            shaderDescriptorSet.UniformDescriptors[binding] = uniform;

            (void)memberCount;
            VtCoreTrace("name: {}", name);
            VtCoreTrace("set: {}, binding: {}", set, binding);
            VtCoreTrace("memberCount: {}", memberCount);
            VtCoreTrace("size: {}", size);
            VtCoreTrace("descriptorCount: {}", descriptorCount);
            VtCoreTrace("-------------------");
        }

        for (const auto& resource : resources.sampled_images)
        {
            const auto& name = resource.name;
            auto&       type = compiler.get_type(resource.type_id);
            u32         set  = compiler.get_decoration(resource.id,
                                                       spv::DecorationDescriptorSet);
            u32         binding
                = compiler.get_decoration(resource.id, spv::DecorationBinding);
            u32 descriptorCount = 1;
            for (auto size : type.array) descriptorCount *= size;

            bindings.insert(binding);
            sets.insert(set);
            auto&        shaderDescriptorSet = m_DescriptorSets[set];
            ImageSampler sampler;
            sampler.Name            = StringView(name.data(), name.size());
            sampler.BindingPoint    = binding;
            sampler.DescriptorCount = descriptorCount;
            sampler.ShaderStage     = stage;

            shaderDescriptorSet.ImageSamplers[binding] = sampler;
            VtCoreTrace("name: {}", name);
            VtCoreTrace("set: {}, binding: {}", set, binding);
            VtCoreTrace("descriptorCount: {}", descriptorCount);
            VtCoreTrace("-------------------");
        }

        VtCoreInfo("total sets: {}", sets.size());
        VtCoreInfo("total bindings: {}", bindings.size());
    }

    String VulkanShader::ReadShaderCode(StringView path)
    {
        std::ifstream ifs(path.Raw());
        VtCoreAssert(ifs.is_open());

        std::stringstream source;
        source << ifs.rdbuf();

        auto code = source.str();
        return String(code.data(), code.size());
    }

    void VulkanShader::CompileShaderCode(StringView source, StringView path,
                                         vk::ShaderStageFlagBits stage,
                                         Vector<u32>&            outputBinary)
    {
        shaderc::CompileOptions options;
        // options.SetSourceLanguage(shaderc_source_language_hlsl);
        options.SetTargetEnvironment(shaderc_target_env_vulkan,
                                     shaderc_env_version_vulkan_1_2);
        // options.AddMacroDefinition("pack_matrix", "row_major");

#ifdef VT_DIST
        options.SetOptimizationLevel(shaderc_optimization_level_performance);
#endif
        // Explicitly forces shaderc's backend to preserve standard row-major
        // memory mapping

        shaderc::SpvCompilationResult module = s_Compiler.CompileGlslToSpv(
            source.Raw(), VulkanToShadercStage(stage), path.Raw(), options);

        VtCoreAssertMsg(module.GetCompilationStatus()
                            == shaderc_compilation_status_success,
                        module.GetErrorMessage());

        outputBinary = Vector<u32>(module.cbegin(), module.cend());
    }
}; // namespace Vortex

/*
 * Created by v1tr10l7 on 28.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/API/Vulkan/VulkanContext.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanShader.hpp"

#include <libshaderc_util/file_finder.h>
#include <shaderc/shaderc.hpp>
#include <spirv-tools/libspirv.h>
#include <spirv_cross/spirv_glsl.hpp>

namespace Vortex
{
    namespace
    {
        shaderc::Compiler       s_Compiler;
        vk::ShaderStageFlagBits StringToShaderStage(std::string_view stage)
        {
            if (stage == "vertex") return vk::ShaderStageFlagBits::eVertex;
            else if (stage == "fragment")
                return vk::ShaderStageFlagBits::eFragment;

            VtCoreAssertMsg(false, "unknown shader stage");
            return vk::ShaderStageFlagBits(0);
        }
    } // namespace

    VulkanShader::VulkanShader(std::string_view path)
    {
        std::string shaderCode = ReadShaderCode(path);
        auto        sources    = Preprocess(shaderCode);
        vk::Device  device     = VulkanContext::GetDevice();

        for (auto& [stage, source] : sources)
        {
            std::vector<u32> shaderBinary;
            CompileShaderCode(source, path, shaderBinary);
            Reflect(path, stage, shaderBinary);

            vk::ShaderModuleCreateInfo shaderInfo{};
            shaderInfo.sType    = vk::StructureType::eShaderModuleCreateInfo;
            shaderInfo.codeSize = shaderBinary.size() * sizeof(u32);
            shaderInfo.pCode
                = reinterpret_cast<const u32*>(shaderBinary.data());

            if (stage == vk::ShaderStageFlagBits::eVertex)
            {
                VkCall(device.createShaderModule(&shaderInfo, VK_NULL_HANDLE,
                                                 &m_VertexShader));
            }
            else if (stage == vk::ShaderStageFlagBits::eFragment)
                VkCall(device.createShaderModule(&shaderInfo, VK_NULL_HANDLE,
                                                 &m_FragmentShader));
        }

        for (auto&& [set, shaderDescriptorSet] : m_DescriptorSets)
        {
            if (shaderDescriptorSet.UniformDescriptors.size())
            {
                vk::DescriptorPoolSize& typeCount
                    = m_DescriptorSets[set].PoolSizes.emplace_back();
                typeCount.type = vk::DescriptorType::eUniformBuffer;
                typeCount.descriptorCount
                    = shaderDescriptorSet.UniformDescriptors.size()
                    * VT_MAX_FRAMES_IN_FLIGHT;
                VtCoreTrace("size: {}", typeCount.descriptorCount);
            }

            std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
            for (auto& [binding, uniformBuffer] :
                 shaderDescriptorSet.UniformDescriptors)
            {
                vk::DescriptorSetLayoutBinding& layoutBinding
                    = layoutBindings.emplace_back();
                layoutBinding.descriptorType
                    = vk::DescriptorType::eUniformBuffer;
                layoutBinding.descriptorCount    = 1;
                layoutBinding.stageFlags         = uniformBuffer.ShaderStage;
                layoutBinding.pImmutableSamplers = VK_NULL_HANDLE;
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
                       shaderDescriptorSet.UniformDescriptors.size());

            vk::DescriptorSetLayoutCreateInfo createInfo{};
            createInfo.sType
                = vk::StructureType::eDescriptorSetLayoutCreateInfo;
            createInfo.pNext        = VK_NULL_HANDLE;
            createInfo.flags        = vk::DescriptorSetLayoutCreateFlags();
            createInfo.bindingCount = layoutBindings.size();
            createInfo.pBindings    = layoutBindings.data();

            vk::DescriptorSetLayout& descriptorSetLayout
                = m_DescriptorSetLayouts.emplace_back();
            VkCall(device.createDescriptorSetLayout(&createInfo, VK_NULL_HANDLE,
                                                    &descriptorSetLayout));
        }

        for (auto& set : m_DescriptorSets)
        {
            vk::DescriptorPoolCreateInfo poolInfo{};
            poolInfo.sType   = vk::StructureType::eDescriptorPoolCreateInfo;
            poolInfo.pNext   = VK_NULL_HANDLE;
            poolInfo.flags   = vk::DescriptorPoolCreateFlags();
            poolInfo.maxSets = VT_MAX_FRAMES_IN_FLIGHT;
            poolInfo.poolSizeCount             = set.second.PoolSizes.size();
            poolInfo.pPoolSizes                = set.second.PoolSizes.data();

            vk::DescriptorPool& descriptorPool = set.second.Pool;
            VkCall(device.createDescriptorPool(&poolInfo, VK_NULL_HANDLE,
                                               &descriptorPool));
        }

        for (auto& descriptorSet : m_DescriptorSets)
        {
            vk::DescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = vk::StructureType::eDescriptorSetAllocateInfo;
            allocInfo.descriptorPool
                = m_DescriptorSets[descriptorSet.first].Pool;
            allocInfo.descriptorSetCount = VT_MAX_FRAMES_IN_FLIGHT;

            std::vector<vk::DescriptorSetLayout> layouts(
                VT_MAX_FRAMES_IN_FLIGHT,
                m_DescriptorSetLayouts[descriptorSet.first]);
            allocInfo.pSetLayouts = layouts.data();

            descriptorSet.second.Sets.resize(VT_MAX_FRAMES_IN_FLIGHT);
            VkCall(device.allocateDescriptorSets(
                &allocInfo, descriptorSet.second.Sets.data()));
        }
    }
    VulkanShader::VulkanShader(std::string_view vertexPath,
                               std::string_view fragmentPath)
    {
        std::vector<u32> vertexShaderCode;
        std::vector<u32> fragmentShaderCode;
        CompileShaderCode(ReadShaderCode(vertexPath), vertexPath,
                          vertexShaderCode);
        CompileShaderCode(ReadShaderCode(fragmentPath), fragmentPath,
                          fragmentShaderCode);

        Reflect(vertexPath, vk::ShaderStageFlagBits::eVertex, vertexShaderCode);
        Reflect(fragmentPath, vk::ShaderStageFlagBits::eFragment,
                fragmentShaderCode);

        vk::ShaderModuleCreateInfo vertexShaderInfo{};
        vertexShaderInfo.sType    = vk::StructureType::eShaderModuleCreateInfo;
        vertexShaderInfo.codeSize = vertexShaderCode.size() * sizeof(u32);
        vertexShaderInfo.pCode
            = reinterpret_cast<const u32*>(vertexShaderCode.data());

        vk::ShaderModuleCreateInfo fragmentShaderInfo{};
        fragmentShaderInfo.sType = vk::StructureType::eShaderModuleCreateInfo;
        fragmentShaderInfo.codeSize = fragmentShaderCode.size() * sizeof(u32);
        fragmentShaderInfo.pCode
            = reinterpret_cast<const u32*>(fragmentShaderCode.data());

        vk::Device device = VulkanContext::GetDevice();
        VkCall(device.createShaderModule(&vertexShaderInfo, VK_NULL_HANDLE,
                                         &m_VertexShader));
        VkCall(device.createShaderModule(&fragmentShaderInfo, VK_NULL_HANDLE,
                                         &m_FragmentShader));

        for (auto&& [set, shaderDescriptorSet] : m_DescriptorSets)
        {
            if (shaderDescriptorSet.UniformDescriptors.size())
            {
                VkDescriptorPoolSize& typeCount
                    = m_DescriptorSets[set].PoolSizes.emplace_back();
                typeCount.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                typeCount.descriptorCount
                    = shaderDescriptorSet.UniformDescriptors.size();
                VtCoreTrace("size: {}", typeCount.descriptorCount);
            }

            std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
            for (auto& [binding, uniformBuffer] :
                 shaderDescriptorSet.UniformDescriptors)
            {
                vk::DescriptorSetLayoutBinding& layoutBinding
                    = layoutBindings.emplace_back();
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
                       shaderDescriptorSet.UniformDescriptors.size());

            vk::DescriptorSetLayoutCreateInfo createInfo{};
            createInfo.sType
                = vk::StructureType::eDescriptorSetLayoutCreateInfo;
            createInfo.pNext        = VK_NULL_HANDLE;
            createInfo.flags        = vk::DescriptorSetLayoutCreateFlags();
            createInfo.bindingCount = layoutBindings.size();
            createInfo.pBindings    = layoutBindings.data();

            VkCall(device.createDescriptorSetLayout(
                &createInfo, VK_NULL_HANDLE, &m_DescriptorSetLayouts[set]));
        }
    }
    VulkanShader::~VulkanShader()
    {
        vk::Device device = VulkanContext::GetDevice();
        for (auto& set : m_DescriptorSets)
            device.destroyDescriptorPool(set.second.Pool);
        for (auto layout : m_DescriptorSetLayouts)
            device.destroyDescriptorSetLayout(layout, VK_NULL_HANDLE);
        device.destroyShaderModule(m_VertexShader, VK_NULL_HANDLE);
        device.destroyShaderModule(m_FragmentShader, VK_NULL_HANDLE);
    }

    void VulkanShader::SetUniform(const std::string&       name,
                                  Ref<VulkanUniformBuffer> buffer)
    {
        vk::Device              device = VulkanContext::GetDevice();
        vk::WriteDescriptorSet& descriptorWrite
            = m_DescriptorSets[0].WriteDescriptorSets[name];

        for (usize i = 0; i < VT_MAX_FRAMES_IN_FLIGHT; i++)
        {
            descriptorWrite.dstSet          = m_DescriptorSets[0].Sets[i];
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
            descriptorWrite.pBufferInfo    = &buffer->m_BufferInfos[i];

            device.updateDescriptorSets(1, &descriptorWrite, 0, VK_NULL_HANDLE);
        }
    }

    std::unordered_map<vk::ShaderStageFlagBits, std::string>
    VulkanShader::Preprocess(std::string_view shaderCode)
    {
        std::unordered_map<vk::ShaderStageFlagBits, std::string> shaderStages;

        static constexpr auto npos = std::string::npos;
        auto getShaderStage = [](std::string_view source) -> std::string_view
        {
            static constexpr std::string_view STAGE_DIRECTIVE_STRING
                = "#pragma shader_stage";

            usize stageDirectiveStart = source.find(STAGE_DIRECTIVE_STRING);
            VtCoreAssert(stageDirectiveStart != npos);

            usize stageNameStart
                = stageDirectiveStart + STAGE_DIRECTIVE_STRING.length();
            while (!std::isalpha(source[stageNameStart])
                   && stageNameStart < source.size())
                ++stageNameStart;

            usize stageNameEnd = stageNameStart;
            while (std::isalpha(source[stageNameEnd])
                   && stageNameEnd < source.size())
                ++stageNameEnd;

            return source.substr(stageNameStart, stageNameEnd - stageNameStart);
        };

        static constexpr std::string_view VERSION_DIRECTIVE_STRING = "#version";
        usize stageStart = shaderCode.find(VERSION_DIRECTIVE_STRING);
        while (stageStart != npos)
        {

            std::string_view stageString
                = getShaderStage(shaderCode.substr(stageStart));

            usize nextStageStart
                = shaderCode.find(VERSION_DIRECTIVE_STRING, stageStart + 1);

            vk::ShaderStageFlagBits currentStage
                = StringToShaderStage(stageString);

            shaderStages[currentStage]
                = (nextStageStart != npos)
                    ? shaderCode.substr(stageStart, nextStageStart - stageStart)
                    : shaderCode.substr(stageStart);

            stageStart = nextStageStart;
        }

        return shaderStages;
    }
    void VulkanShader::Reflect(std::string_view        path,
                               vk::ShaderStageFlagBits stage,
                               const std::vector<u32>& shaderBinary)
    {
        spirv_cross::Compiler compiler(shaderBinary);
        auto                  resources = compiler.get_shader_resources();

        VtCoreTrace("===========================");
        VtCoreTrace(" Vulkan Shader Reflection");
        VtCoreTrace(" {0}", path.data());
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
            uniform.Name                                    = name;
            uniform.BindingPoint                            = binding;
            uniform.DescriptorCount                         = descriptorCount;
            uniform.Size                                    = size;
            uniform.ShaderStage                             = stage;
            shaderDescriptorSet.UniformDescriptors[binding] = uniform;

            VtCoreTrace("name: {}", name);
            VtCoreTrace("set: {}, binding: {}", set, binding);
            VtCoreTrace("memberCount: {}", memberCount);
            VtCoreTrace("size: {}", size);
            VtCoreTrace("descriptorCount: {}", descriptorCount);
            VtCoreTrace("-------------------");
        }

        VtCoreInfo("total sets: {}", sets.size());
        VtCoreInfo("total bindings: {}", bindings.size());
    }

    std::string VulkanShader::ReadShaderCode(std::string_view path)
    {
        std::ifstream ifs(path.data());
        VtCoreAssert(ifs.is_open());

        std::stringstream source;
        source << ifs.rdbuf();

        return source.str();
    }

    void VulkanShader::CompileShaderCode(std::string_view  source,
                                         std::string_view  path,
                                         std::vector<u32>& outputBinary)
    {
        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_vulkan,
                                     shaderc_env_version_vulkan_1_2);

#ifdef VT_DIST
        options.SetOptimizationLevel(shaderc_optimization_level_performance);
#endif

        shaderc::SpvCompilationResult module = s_Compiler.CompileGlslToSpv(
            source.data(), shaderc_glsl_infer_from_source, path.data(),
            options);

        VtCoreAssertMsg(module.GetCompilationStatus()
                            == shaderc_compilation_status_success,
                        module.GetErrorMessage());

        outputBinary = std::vector<u32>(module.cbegin(), module.cend());
    }
}; // namespace Vortex

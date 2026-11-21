/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Renderer/RHI/Vulkan/VulkanCommon.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanDevice.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanShader.hpp>

#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/DirStackFileIncluder.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <shaderc/shaderc.hpp>
#include <spirv_cross.hpp>

#include <fstream>
#include <sstream>
#include <string>
#include <string_view>

import Prism.String.Formatter;
import Prism.Utility.Path;

namespace Vortex::Vulkan
{
    using namespace RHI;
    using Deletion = DeferredDeletion::Type;

    //------------------------------------------------------------------------
    // Set layouts -----------------------------------------------------------
    //------------------------------------------------------------------------

    VulkanBindGroupLayout::VulkanBindGroupLayout(const BindGroupLayoutDesc& desc)
        : BindGroupLayout(desc)
    {
        Vector<vk::DescriptorSetLayoutBinding> bindings;
        for (const auto& entry : desc.Entries)
        {
            vk::DescriptorSetLayoutBinding& binding = bindings.EmplaceBack();
            binding.binding         = entry.Binding;
            binding.descriptorType  = ToVk(entry.Type);
            binding.descriptorCount = entry.Count;
            binding.stageFlags      = ToVk(entry.Stages);
        }

        vk::DescriptorSetLayoutCreateInfo info{};
        info.bindingCount = bindings.Size();
        info.pBindings    = bindings.Raw();
        VkCall(VulkanDevice::Get().Handle().createDescriptorSetLayout(
            &info, nullptr, &m_Layout));
    }
    VulkanBindGroupLayout::~VulkanBindGroupLayout()
    {
        VulkanDevice::Get().Defer(Deletion::eDescriptorSetLayout, m_Layout);
    }

    //------------------------------------------------------------------------
    // Compilation -----------------------------------------------------------
    //------------------------------------------------------------------------

    namespace
    {
        constexpr StringView kStageDirective = "#pragma shader_stage(";
        constexpr StringView kVersionDirective = "#version";

        ShaderStage StageFromName(StringView name)
        {
            if (name == "vertex") return ShaderStage::eVertex;
            if (name == "fragment") return ShaderStage::eFragment;
            if (name == "compute") return ShaderStage::eCompute;
            if (name == "geometry") return ShaderStage::eGeometry;
            if (name == "tesscontrol") return ShaderStage::eTessControl;
            if (name == "tesseval") return ShaderStage::eTessEval;
            return ShaderStage::eNone;
        }
        shaderc_shader_kind ShadercKind(ShaderStage stage)
        {
            switch (stage)
            {
                case ShaderStage::eVertex: return shaderc_vertex_shader;
                case ShaderStage::eFragment: return shaderc_fragment_shader;
                case ShaderStage::eCompute: return shaderc_compute_shader;
                case ShaderStage::eGeometry: return shaderc_geometry_shader;
                case ShaderStage::eTessControl:
                    return shaderc_tess_control_shader;
                case ShaderStage::eTessEval: return shaderc_tess_evaluation_shader;
                default: break;
            }
            return shaderc_glsl_infer_from_source;
        }

        Expected<String, String> ReadFile(const Path& path)
        {
            std::ifstream file(path.Raw(), std::ios::binary);
            if (!file.is_open())
                return Unexpected(
                    Prism::Format("Vulkan: Can't open shader '{}'", path.Raw()));
            std::stringstream stream;
            stream << file.rdbuf();
            auto str = stream.str();
            return String(str.data(), str.size());
        }

        /// Splits a multi-stage file at each `#version`; the stage of each
        /// part comes from its `#pragma shader_stage(...)`.
        Expected<Vector<ShaderSource>, String> SplitStages(StringView code,
                                                           StringView name)
        {
            Vector<ShaderSource> sources;
            usize                start = code.Find(kVersionDirective);
            if (start == StringView::NPos)
                return Unexpected(Prism::Format(
                    "Vulkan: Shader '{}' has no #version directive", name));

            while (start != StringView::NPos)
            {
                usize next = code.Find(kVersionDirective, start + 1);
                StringView part
                    = next == StringView::NPos
                        ? code.Substr(start)
                        : code.Substr(start, next - start);

                usize directive = part.Find(kStageDirective);
                if (directive == StringView::NPos)
                    return Unexpected(Prism::Format(
                        "Vulkan: Shader '{}': a stage lacks "
                        "#pragma shader_stage(...)",
                        name));
                usize nameStart = directive + kStageDirective.Size();
                usize nameEnd   = part.Find(')', nameStart);
                if (nameEnd == StringView::NPos)
                    return Unexpected(Prism::Format(
                        "Vulkan: Shader '{}': unterminated shader_stage", name));

                StringView  stageName = part.Substr(nameStart, nameEnd - nameStart);
                ShaderStage stage     = StageFromName(stageName);
                if (stage == ShaderStage::eNone)
                    return Unexpected(Prism::Format(
                        "Vulkan: Shader '{}': unknown stage '{}'", name,
                        stageName));
                for (const auto& existing : sources)
                    if (existing.Stage == stage)
                        return Unexpected(Prism::Format(
                            "Vulkan: Shader '{}' defines stage '{}' twice",
                            name, stageName));

                sources.PushBack({stage, String(part), String(name)});
                start = next;
            }
            return sources;
        }

        bool IsGlslFile(const Path& path)
        {
            std::string_view file(path.Raw(), path.Size());
            return file.ends_with(".glsl");
        }

        /// An HLSL file is compiled once per `#pragma <stage> <entry>` line,
        /// whole, with that entry point. The pragmas are blanked out (they'd
        /// be unknown to the compiler), keeping line numbers.
        Expected<Vector<ShaderSource>, String> SplitHlslStages(StringView code,
                                                               StringView name)
        {
            std::string           text(code.Raw(), code.Size());
            Vector<ShaderSource>  sources;
            Vector<ShaderStage>   stages;
            Vector<std::string>   entries;

            for (usize start = 0; start < text.size();)
            {
                usize end = text.find('\n', start);
                if (end == std::string::npos) end = text.size();
                std::string_view line(text.data() + start, end - start);

                usize first = line.find_first_not_of(" \t");
                if (first != std::string_view::npos
                    && line.substr(first).starts_with("#pragma"))
                {
                    std::string_view rest = line.substr(first + 7);
                    auto             word = [&rest]() -> std::string_view
                    {
                        usize begin = rest.find_first_not_of(" \t\r");
                        if (begin == std::string_view::npos) return {};
                        usize finish = rest.find_first_of(" \t\r", begin);
                        if (finish == std::string_view::npos) finish = rest.size();
                        std::string_view token = rest.substr(begin, finish - begin);
                        rest                   = rest.substr(finish);
                        return token;
                    };
                    std::string_view stageName = word();
                    ShaderStage      stage     = StageFromName(
                        StringView(stageName.data(), stageName.size()));
                    if (stage != ShaderStage::eNone)
                    {
                        std::string_view entry = word();
                        if (entry.empty())
                            return Unexpected(Prism::Format(
                                "Vulkan: Shader '{}': `#pragma {}` names no entry "
                                "point",
                                name, StringView(stageName.data(), stageName.size())));
                        for (auto existing : stages)
                            if (existing == stage)
                                return Unexpected(Prism::Format(
                                    "Vulkan: Shader '{}' defines stage '{}' twice", name,
                                    StringView(stageName.data(), stageName.size())));
                        stages.PushBack(stage);
                        entries.PushBack(std::string(entry));
                        for (usize i = start; i < end; ++i)
                            if (text[i] != '\r') text[i] = ' ';
                    }
                }
                start = end + 1;
            }

            if (stages.Empty())
                return Unexpected(Prism::Format(
                    "Vulkan: Shader '{}' names no stage (`#pragma vertex VSMain`, "
                    "`#pragma fragment PSMain`, `#pragma compute CSMain`...)",
                    name));

            String processed(text.data(), text.size());
            for (usize i = 0; i < stages.Size(); ++i)
            {
                ShaderSource& source = sources.EmplaceBack();
                source.Stage         = stages[i];
                source.Code          = processed;
                source.Name          = String(name);
                source.EntryPoint    = String(entries[i].data(), entries[i].size());
            }
            return sources;
        }

        EShLanguage GlslangStage(ShaderStage stage)
        {
            switch (stage)
            {
                case ShaderStage::eVertex: return EShLangVertex;
                case ShaderStage::eFragment: return EShLangFragment;
                case ShaderStage::eCompute: return EShLangCompute;
                case ShaderStage::eGeometry: return EShLangGeometry;
                case ShaderStage::eTessControl: return EShLangTessControl;
                case ShaderStage::eTessEval: return EShLangTessEvaluation;
                default: break;
            }
            return EShLangCount;
        }

        /// HLSL goes through glslang directly: shaderc can't ask it to merge
        /// textures with their samplers.
        Expected<Vector<u32>, String> CompileHlsl(const ShaderSource& source,
                                                  const ShaderDesc&   desc)
        {
            // glslang's per-process tables, for as long as shaders compile.
            struct Process
            {
                Process() { glslang::InitializeProcess(); }
                ~Process() { glslang::FinalizeProcess(); }
            };
            static Process process;

            EShLanguage stage = GlslangStage(source.Stage);
            std::string name  = source.Name.Empty()
                                  ? std::string("shader")
                                  : std::string(source.Name.Raw(), source.Name.Size());
            std::string entry = source.EntryPoint.Empty()
                                  ? std::string("main")
                                  : std::string(source.EntryPoint.Raw(),
                                                source.EntryPoint.Size());
            if (stage == EShLangCount)
                return Unexpected(Prism::Format(
                    "Vulkan: Shader '{}': unsupported stage for HLSL", source.Name));

            std::string preamble;
            for (const auto& define : desc.Defines)
            {
                preamble += "#define ";
                preamble.append(define.Key.Raw(), define.Key.Size());
                preamble += ' ';
                preamble.append(define.Value.Raw(), define.Value.Size());
                preamble += '\n';
            }

            glslang::TShader shader(stage);
            const char*      code   = source.Code.Raw();
            const int        length = static_cast<int>(source.Code.Size());
            const char*      names  = name.c_str();
            shader.setStringsWithLengthsAndNames(&code, &length, &names, 1);
            shader.setPreamble(preamble.c_str());
            // Pipelines start every stage at "main": the SPIR-V entry point
            // is named that, whatever the HLSL function is called.
            shader.setEntryPoint("main");
            shader.setSourceEntryPoint(entry.c_str());
            shader.setEnvInput(glslang::EShSourceHlsl, stage, glslang::EShClientVulkan,
                               100);
            shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
            shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_6);
            // Texture + SamplerState pairs become combined image samplers,
            // the binding model of the rest of the engine.
            shader.setTextureSamplerTransformMode(
                EShTexSampTransUpgradeTextureRemoveSampler);

            // #include "x" is looked up next to the including file.
            DirStackFileIncluder includer;
            const auto messages = static_cast<EShMessages>(
                EShMsgSpvRules | EShMsgVulkanRules | EShMsgReadHlsl);
            if (!shader.parse(GetDefaultResources(), 100, false, messages, includer))
                return Unexpected(Prism::Format(
                    "Vulkan: Failed to compile '{}' ({}):\n{}", StringView(name.c_str()),
                    StringView(entry.c_str()), StringView(shader.getInfoLog())));

            glslang::TProgram program;
            program.addShader(&shader);
            if (!program.link(messages))
                return Unexpected(Prism::Format(
                    "Vulkan: Failed to link '{}' ({}):\n{}", StringView(name.c_str()),
                    StringView(entry.c_str()), StringView(program.getInfoLog())));

            glslang::SpvOptions options;
            options.generateDebugInfo = true;
#ifdef VT_DIST
            options.generateDebugInfo = false;
            options.disableOptimizer  = false;
#endif
            std::vector<unsigned int> spirv;
            spv::SpvBuildLogger       logger;
            glslang::GlslangToSpv(*program.getIntermediate(stage), spirv, &logger,
                                  &options);
            if (spirv.empty())
                return Unexpected(Prism::Format(
                    "Vulkan: '{}' ({}) produced no SPIR-V:\n{}", StringView(name.c_str()),
                    StringView(entry.c_str()), StringView(logger.getAllMessages().c_str())));
            return Vector<u32>(spirv.data(), spirv.data() + spirv.size());
        }

        Expected<Vector<u32>, String> CompileGlsl(const ShaderSource& source,
                                                  const ShaderDesc&   desc)
        {
            static shaderc::Compiler compiler;

            shaderc::CompileOptions options;
            options.SetTargetEnvironment(shaderc_target_env_vulkan,
                                         shaderc_env_version_vulkan_1_3);
            options.SetGenerateDebugInfo();
#ifdef VT_DIST
            options.SetOptimizationLevel(shaderc_optimization_level_performance);
#endif
            for (const auto& define : desc.Defines)
                options.AddMacroDefinition(define.Key.Raw(), define.Key.Size(),
                                           define.Value.Raw(),
                                           define.Value.Size());

            // The sized overload: `source.Code` is one stage of a larger file
            // and isn't NUL-terminated at the stage boundary.
            String name   = source.Name.Empty() ? String("shader") : source.Name;
            auto   result = compiler.CompileGlslToSpv(
                source.Code.Raw(), source.Code.Size(), ShadercKind(source.Stage),
                name.Raw(), "main", options);
            if (result.GetCompilationStatus()
                != shaderc_compilation_status_success)
            {
                auto message = result.GetErrorMessage();
                return Unexpected(Prism::Format(
                    "Vulkan: Failed to compile '{}':\n{}", name,
                    StringView(message.data(), message.size())));
            }
            return Vector<u32>(result.cbegin(), result.cend());
        }

        //--------------------------------------------------------------------
        // Reflection --------------------------------------------------------
        //--------------------------------------------------------------------

        struct LayoutBuilder
        {
            ShaderLayout Layout;
            String       Error;

            void Add(u32 set, BindingLayoutEntry entry)
            {
                while (Layout.Sets.Size() <= set) Layout.Sets.EmplaceBack();
                auto& entries = Layout.Sets[set].Entries;
                for (auto& existing : entries)
                {
                    if (existing.Binding != entry.Binding) continue;
                    if (existing.Type != entry.Type && Error.Empty())
                        Error = Prism::Format(
                            "set {} binding {} has different types in "
                            "different stages",
                            set, entry.Binding);
                    // The same binding in several stages: merge visibility.
                    existing.Stages |= entry.Stages;
                    if (entry.Size > existing.Size) existing.Size = entry.Size;
                    return;
                }
                entries.PushBack(Move(entry));
            }

            void AddPushConstants(ShaderStage stage, u32 offset, u32 size)
            {
                // GLSL allows one push constant block per stage; stages share
                // one range so a single vkCmdPushConstants covers them all.
                if (Layout.PushConstants.Empty())
                {
                    Layout.PushConstants.PushBack({stage, offset, size});
                    return;
                }
                auto& range = Layout.PushConstants[0];
                u32   end   = range.Offset + range.Size;
                if (offset < range.Offset) range.Offset = offset;
                if (offset + size > end) end = offset + size;
                range.Size = end - range.Offset;
                range.Stages |= stage;
            }
        };

        void Reflect(const Vector<u32>& spirv, ShaderStage stage,
                     LayoutBuilder& builder)
        {
            spirv_cross::Compiler compiler(spirv.Raw(), spirv.Size());
            auto                  resources = compiler.get_shader_resources();

            auto add = [&](const spirv_cross::Resource& resource,
                           BindingType type, bool buffer)
            {
                const auto& spirType = compiler.get_type(resource.type_id);
                u32 count = 1;
                for (u32 dimension : spirType.array) count *= dimension;

                BindingLayoutEntry entry;
                entry.Binding = compiler.get_decoration(resource.id,
                                                        spv::DecorationBinding);
                entry.Type    = type;
                entry.Count   = count;
                entry.Stages  = stage;
                const auto& name = buffer && !resource.name.empty()
                                     ? resource.name
                                     : compiler.get_name(resource.id);
                entry.Name = String(name.data(), name.size());
                if (buffer)
                {
                    const auto& base = compiler.get_type(resource.base_type_id);
                    // Runtime-sized trailing arrays have no declared size.
                    bool sized = base.member_types.empty()
                              || compiler.get_type(base.member_types.back())
                                     .array.empty()
                              || compiler.get_type(base.member_types.back())
                                         .array.back()
                                     != 0;
                    entry.Size
                        = sized ? static_cast<u32>(
                                      compiler.get_declared_struct_size(base))
                                : 0;
                }
                u32 set = compiler.get_decoration(resource.id,
                                                  spv::DecorationDescriptorSet);
                builder.Add(set, Move(entry));
            };

            for (const auto& r : resources.uniform_buffers)
                add(r, BindingType::eUniformBuffer, true);
            for (const auto& r : resources.storage_buffers)
                add(r, BindingType::eStorageBuffer, true);
            for (const auto& r : resources.sampled_images)
                add(r, BindingType::eSampledTexture, false);
            for (const auto& r : resources.separate_images)
                add(r, BindingType::eTexture, false);
            for (const auto& r : resources.separate_samplers)
                add(r, BindingType::eSampler, false);
            for (const auto& r : resources.storage_images)
                add(r, BindingType::eStorageTexture, false);

            for (const auto& r : resources.push_constant_buffers)
            {
                auto ranges = compiler.get_active_buffer_ranges(r.id);
                if (ranges.empty()) continue;
                usize begin = ranges[0].offset;
                usize end   = 0;
                for (const auto& range : ranges)
                {
                    if (range.offset < begin) begin = range.offset;
                    if (range.offset + range.range > end)
                        end = range.offset + range.range;
                }
                builder.AddPushConstants(stage, static_cast<u32>(begin),
                                         static_cast<u32>(end - begin));
            }
        }
    } // namespace

    //------------------------------------------------------------------------
    // Shader ----------------------------------------------------------------
    //------------------------------------------------------------------------

    Expected<::Ref<VulkanShader>, String>
    VulkanShader::Create(const ShaderDesc& desc)
    {
        Vector<ShaderSource> sources  = desc.Sources;
        String               name     = desc.DebugName;
        ShaderLanguage       language = desc.Language;
        if (sources.Empty())
        {
            if (desc.File.Empty())
                return Unexpected(
                    String("Vulkan: ShaderDesc has neither a file nor sources"));
            auto code = ReadFile(desc.File);
            if (!code) return Unexpected(code.Error());
            if (name.Empty()) name = String(desc.File.Raw());
            if (IsGlslFile(desc.File)) language = ShaderLanguage::eGLSL;
            // Sources are named after the file, so HLSL includes resolve
            // next to it.
            String file(desc.File.Raw());
            auto   split = language == ShaderLanguage::eHLSL
                             ? SplitHlslStages(*code, file)
                             : SplitStages(*code, file);
            if (!split) return Unexpected(split.Error());
            sources = Move(*split);
        }

        auto&         device = VulkanDevice::Get();
        auto          shader = CreateRef<VulkanShader>();
        LayoutBuilder builder;
        shader->m_Name = name;

        for (const auto& source : sources)
        {
            auto spirv = language == ShaderLanguage::eHLSL ? CompileHlsl(source, desc)
                                                           : CompileGlsl(source, desc);
            if (!spirv) return Unexpected(spirv.Error());
            Reflect(*spirv, source.Stage, builder);

            vk::ShaderModuleCreateInfo info{};
            info.codeSize = spirv->Size() * sizeof(u32);
            info.pCode    = spirv->Raw();
            Module module{source.Stage, VK_NULL_HANDLE};
            VkCall(device.Handle().createShaderModule(&info, nullptr,
                                                      &module.Handle));
            SetDebugName(device.Handle(), module.Handle, name);
            shader->m_Modules.PushBack(module);
            shader->m_Stages |= source.Stage;
        }
        if (!builder.Error.Empty())
            return Unexpected(
                Prism::Format("Vulkan: Shader '{}': {}", name, builder.Error));

        shader->m_Layout = Move(builder.Layout);
        for (const auto& set : shader->m_Layout.Sets)
        {
            if (set.Entries.Empty())
            {
                shader->m_SetLayouts.PushBack(nullptr);
                shader->m_SetLayoutHandles.PushBack(device.EmptySetLayout());
                continue;
            }
            auto layout = CreateRef<VulkanBindGroupLayout>(set);
            shader->m_SetLayoutHandles.PushBack(layout->Handle());
            shader->m_SetLayouts.PushBack(layout);
        }
        return shader;
    }

    VulkanShader::~VulkanShader()
    {
        auto& device = VulkanDevice::Get();
        for (auto& module : m_Modules)
            device.Defer(Deletion::eShaderModule, module.Handle);
    }

    ::Ref<BindGroupLayout> VulkanShader::SetLayout(u32 set) const
    {
        if (set >= m_SetLayouts.Size()) return nullptr;
        return m_SetLayouts[set];
    }
} // namespace Vortex::Vulkan

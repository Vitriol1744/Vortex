/*
 * Created by v1tr10l7 on 20.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Nexus/EditorContext.hpp>
#include <Nexus/UI/Windows/ConsoleWindow.hpp>
#include <Vortex/Core/Core.hpp>
#include <Vortex/Renderer/ImGui/ImGui.hpp>
#include <Vortex/Renderer/ShaderLibrary.hpp>
#include <Vortex/Renderer/TextureAsset.hpp>
#include <Vortex/Scene/Components.hpp>
#include <Vortex/Script/ScriptEngine.hpp>
#include <Vortex/Window/Window.hpp>

#include <imgui.h>

#include <cfloat>
#include <cmath>
#include <cstdio>
#include <filesystem>

import Nexus.Inspector;
import Prism.Containers.Vector;
import Prism.String.String;
import Prism.String.StringView;
import Prism.Utility.Optional;
import Prism.Utility.Path;
import Vortex.Renderer;
import Vortex.Renderer.Material;

namespace Vortex::Editor::Inspector
{
    namespace
    {
        constexpr ImVec4 kAccent  = {0.153f, 0.725f, 0.949f, 1.00f};
        constexpr ImVec4 kGood    = {0.45f, 0.85f, 0.52f, 1.00f};
        constexpr ImVec4 kWarning = {1.00f, 0.76f, 0.30f, 1.00f};
        constexpr ImVec4 kBad     = {1.00f, 0.45f, 0.45f, 1.00f};
        constexpr ImVec4 kMuted   = {0.60f, 0.62f, 0.66f, 1.00f};

        // Layout ----------------------------------------------------------

        /// Label | value rows. The label column is fixed so panels line up.
        bool BeginProps(const char* id, f32 labelWidth = 124.0f)
        {
            if (!ImGui::BeginTable(id, 2, ImGuiTableFlags_SizingFixedFit)) return false;
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelWidth);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
            return true;
        }
        void EndProps() { ImGui::EndTable(); }

        /// Starts a row: the label (with an optional tooltip), then the
        /// cursor in the value cell with items filling it.
        void Prop(const char* label, const char* tooltip = nullptr,
                  ImVec4 color = {0, 0, 0, 0})
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            if (color.w > 0.0f) ImGui::TextColored(color, "%s", label);
            else ImGui::TextUnformatted(label);
            if (tooltip && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
                ImGui::SetTooltip("%s", tooltip);
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-FLT_MIN);
        }
        void PropText(const char* label, const char* value, ImVec4 color = {0, 0, 0, 0})
        {
            Prop(label);
            if (color.w > 0.0f) ImGui::TextColored(color, "%s", value);
            else ImGui::TextUnformatted(value);
        }

        /// A small rounded label on the current line.
        void Pill(const char* text, ImVec4 color, bool sameLine = true)
        {
            if (sameLine) ImGui::SameLine();
            ImVec2      size    = ImGui::CalcTextSize(text);
            ImVec2      padding = {6.0f, 1.0f};
            ImVec2      pos     = ImGui::GetCursorScreenPos();
            const auto& style   = ImGui::GetStyle();
            pos.y += (ImGui::GetFrameHeight() - size.y - padding.y * 2.0f) * 0.5f;
            ImVec4 fill = color;
            fill.w      = 0.18f;
            ImGui::GetWindowDrawList()->AddRectFilled(
                pos, {pos.x + size.x + padding.x * 2, pos.y + size.y + padding.y * 2},
                ImGui::GetColorU32(fill), 8.0f);
            ImGui::GetWindowDrawList()->AddText({pos.x + padding.x, pos.y + padding.y},
                                                ImGui::GetColorU32(color), text);
            ImGui::Dummy({size.x + padding.x * 2, ImGui::GetFrameHeight()});
            IgnoreUnused(style);
        }

        /// A read-only, selectable (copyable) text field.
        void ReadOnly(const char* id, StringView text)
        {
            String copy(text);
            ImGui::SetNextItemWidth(-FLT_MIN);
            UI::InputText(id, copy, ImGuiInputTextFlags_ReadOnly);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal) && !copy.Empty())
                ImGui::SetTooltip("%s", copy.Raw());
        }

        void FormatBytes(char* out, usize size, u64 bytes)
        {
            constexpr f64 kKiB  = 1024.0;
            f64           value = static_cast<f64>(bytes);
            if (value >= kKiB * kKiB) std::snprintf(out, size, "%.1f MiB", value / (kKiB * kKiB));
            else if (value >= kKiB) std::snprintf(out, size, "%.1f KiB", value / kKiB);
            else std::snprintf(out, size, "%llu B", static_cast<unsigned long long>(bytes));
        }
        void FormatCount(char* out, usize size, u64 count)
        {
            if (count >= 1000000)
                std::snprintf(out, size, "%.2fM", static_cast<f64>(count) / 1e6);
            else if (count >= 10000)
                std::snprintf(out, size, "%.1fK", static_cast<f64>(count) / 1e3);
            else std::snprintf(out, size, "%llu", static_cast<unsigned long long>(count));
        }

        /// File name of a path, for compact display.
        StringView FileName(StringView path)
        {
            usize slash = path.Size();
            while (slash > 0 && path[slash - 1] != '/' && path[slash - 1] != '\\') --slash;
            return path.Substr(slash);
        }

        Optional<Path> PickFile()
        {
            auto result = Window::OpenFile();
            if (!result || result->Empty()) return NullOpt;
            return result;
        }

        // RHI names -------------------------------------------------------

        const char* FormatName(RHI::Format format)
        {
            using F = RHI::Format;
            switch (format)
            {
                case F::eR8Unorm: return "R8 Unorm";
                case F::eRG8Unorm: return "RG8 Unorm";
                case F::eRGBA8Unorm: return "RGBA8 Unorm";
                case F::eRGBA8Srgb: return "RGBA8 sRGB";
                case F::eBGRA8Unorm: return "BGRA8 Unorm";
                case F::eBGRA8Srgb: return "BGRA8 sRGB";
                case F::eR16Float: return "R16 Float";
                case F::eRG16Float: return "RG16 Float";
                case F::eRGBA16Float: return "RGBA16 Float";
                case F::eR32Float: return "R32 Float";
                case F::eRG32Float: return "RG32 Float";
                case F::eRGBA32Float: return "RGBA32 Float";
                case F::eRGB10A2Unorm: return "RGB10A2 Unorm";
                case F::eRG11B10Float: return "RG11B10 Float";
                case F::eD16Unorm: return "D16 Unorm";
                case F::eD32Float: return "D32 Float";
                case F::eD24UnormS8Uint: return "D24 S8";
                case F::eD32FloatS8Uint: return "D32 S8";
                case F::eBC1RGBAUnorm: return "BC1 Unorm";
                case F::eBC1RGBASrgb: return "BC1 sRGB";
                case F::eBC3RGBAUnorm: return "BC3 Unorm";
                case F::eBC3RGBASrgb: return "BC3 sRGB";
                case F::eBC4RUnorm: return "BC4 Unorm";
                case F::eBC5RGUnorm: return "BC5 Unorm";
                case F::eBC6HRGBFloat: return "BC6H Float";
                case F::eBC7RGBAUnorm: return "BC7 Unorm";
                case F::eBC7RGBASrgb: return "BC7 sRGB";
                default: return "Other";
            }
        }
        const char* BindingTypeName(RHI::BindingType type)
        {
            switch (type)
            {
                case RHI::BindingType::eUniformBuffer: return "Uniform Buffer";
                case RHI::BindingType::eStorageBuffer: return "Storage Buffer";
                case RHI::BindingType::eSampledTexture: return "Texture + Sampler";
                case RHI::BindingType::eTexture: return "Texture";
                case RHI::BindingType::eSampler: return "Sampler";
                case RHI::BindingType::eStorageTexture: return "Storage Texture";
            }
            return "?";
        }
        /// "VS PS" style list of the stages in `stages`.
        String StageList(RHI::ShaderStage stages)
        {
            struct Name
            {
                RHI::ShaderStage Stage;
                const char*      Short;
            };
            constexpr Name kNames[] = {
                {RHI::ShaderStage::eVertex, "VS"},       {RHI::ShaderStage::eFragment, "PS"},
                {RHI::ShaderStage::eCompute, "CS"},      {RHI::ShaderStage::eGeometry, "GS"},
                {RHI::ShaderStage::eTessControl, "HS"},  {RHI::ShaderStage::eTessEval, "DS"},
            };
            String list;
            for (const auto& name : kNames)
            {
                if (!(static_cast<u32>(stages) & static_cast<u32>(name.Stage))) continue;
                if (!list.Empty()) list += " ";
                list += name.Short;
            }
            return list.Empty() ? String("-") : list;
        }
        bool IsTextureBinding(RHI::BindingType type)
        {
            return type == RHI::BindingType::eSampledTexture
                || type == RHI::BindingType::eTexture;
        }

        /// GPU memory of a texture's full mip chain.
        u64 TextureMemory(const RHI::Texture& texture)
        {
            u64          total  = 0;
            RHI::Extent3D extent = texture.Extent();
            for (u32 mip = 0; mip < texture.MipLevels(); ++mip)
            {
                total += RHI::FormatSize(texture.Format(), extent) * texture.Layers();
                extent.Width  = extent.Width > 1 ? extent.Width / 2 : 1;
                extent.Height = extent.Height > 1 ? extent.Height / 2 : 1;
                extent.Depth  = extent.Depth > 1 ? extent.Depth / 2 : 1;
            }
            return total;
        }

        /// A texture thumbnail, with a larger preview on hover.
        void Thumbnail(const ::Ref<RHI::Texture>& texture, f32 size)
        {
            if (!texture)
            {
                ImGui::Dummy({size, size});
                return;
            }
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddRectFilled(
                pos, {pos.x + size, pos.y + size},
                ImGui::GetColorU32(ImGuiCol_FrameBg), 4.0f);
            UI::Image(texture, {size, size});
            if (ImGui::BeginItemTooltip())
            {
                f32 aspect  = texture->Height()
                                ? static_cast<f32>(texture->Width())
                                      / static_cast<f32>(texture->Height())
                                : 1.0f;
                f32 preview = 256.0f;
                UI::Image(texture, {preview * (aspect > 1.0f ? 1.0f : aspect),
                                    preview / (aspect > 1.0f ? aspect : 1.0f)});
                ImGui::Text("%u x %u  %s  %u mips", texture->Width(), texture->Height(),
                            FormatName(texture->Format()), texture->MipLevels());
                ImGui::EndTooltip();
            }
        }

        // Scripts ---------------------------------------------------------

        /// C# sources of the game, found once per script reload.
        struct ScriptSource
        {
            String ClassName;
            String Path;
        };
        const Vector<ScriptSource>& ScriptSources()
        {
            static Vector<ScriptSource> sources;
            static u64                  generation = ~0ull;
            if (generation == ScriptEngine::Generation()) return sources;
            generation = ScriptEngine::Generation();
            sources.Clear();

            namespace fs = std::filesystem;
            std::error_code error;
            const fs::path  root = "Sandbox";
            if (!fs::exists(root, error)) return sources;
            for (fs::recursive_directory_iterator it(root, error), end; it != end;
                 it.increment(error))
            {
                if (error) break;
                const auto& path = it->path();
                if (it->is_directory(error)
                    && (path.filename() == "obj" || path.filename() == "bin"))
                {
                    it.disable_recursion_pending();
                    continue;
                }
                if (path.extension() != ".cs") continue;
                sources.PushBack({String(path.stem().string().c_str()),
                                  String(path.generic_string().c_str())});
            }
            return sources;
        }
        /// The .cs file named after the class (Unity's convention), if any.
        StringView SourceOf(const ScriptClassInfo& info)
        {
            for (const auto& source : ScriptSources())
                if (source.ClassName == info.Name) return source.Path;
            return {};
        }

        bool IsOverridable(ScriptFieldType type) { return type != ScriptFieldType::eNone; }

        /// The value widget of one field. Returns true when edited.
        bool FieldWidget(const char* id, ScriptFieldType type, u8* data, String& text)
        {
            switch (type)
            {
                case ScriptFieldType::eBool:
                {
                    i32  value   = 0;
                    std::memcpy(&value, data, sizeof(value));
                    bool checked = value != 0;
                    if (!ImGui::Checkbox(id, &checked)) return false;
                    value = checked ? 1 : 0;
                    std::memcpy(data, &value, sizeof(value));
                    return true;
                }
                case ScriptFieldType::eInt32:
                    return ImGui::DragScalar(id, ImGuiDataType_S32, data, 0.2f);
                case ScriptFieldType::eUInt32:
                    return ImGui::DragScalar(id, ImGuiDataType_U32, data, 0.2f);
                case ScriptFieldType::eInt64:
                    return ImGui::DragScalar(id, ImGuiDataType_S64, data, 0.2f);
                case ScriptFieldType::eFloat:
                    return ImGui::DragScalar(id, ImGuiDataType_Float, data, 0.05f, nullptr,
                                             nullptr, "%.3f");
                case ScriptFieldType::eDouble:
                    return ImGui::DragScalar(id, ImGuiDataType_Double, data, 0.05f, nullptr,
                                             nullptr, "%.4f");
                case ScriptFieldType::eVector2:
                    return ImGui::DragScalarN(id, ImGuiDataType_Float, data, 2, 0.05f,
                                              nullptr, nullptr, "%.3f");
                case ScriptFieldType::eVector3:
                    return ImGui::DragScalarN(id, ImGuiDataType_Float, data, 3, 0.05f,
                                              nullptr, nullptr, "%.3f");
                case ScriptFieldType::eVector4:
                    return ImGui::DragScalarN(id, ImGuiDataType_Float, data, 4, 0.05f,
                                              nullptr, nullptr, "%.3f");
                case ScriptFieldType::eString: return UI::InputText(id, text);
                default: return false;
            }
        }

        /// Short type name for display ("float", "Vector3"...).
        const char* FieldTypeLabel(const ScriptFieldInfo& field)
        {
            switch (field.Type)
            {
                case ScriptFieldType::eBool: return "bool";
                case ScriptFieldType::eInt32: return "int";
                case ScriptFieldType::eUInt32: return "uint";
                case ScriptFieldType::eInt64: return "long";
                case ScriptFieldType::eFloat: return "float";
                case ScriptFieldType::eDouble: return "double";
                case ScriptFieldType::eVector2: return "Vector2";
                case ScriptFieldType::eVector3: return "Vector3";
                case ScriptFieldType::eVector4: return "Vector4";
                case ScriptFieldType::eString: return "string";
                default: return field.TypeName.Raw();
            }
        }

        /// "MoveSpeed" -> "Move Speed", "m_Health" -> "Health".
        String NiceName(StringView name)
        {
            if (name.StartsWith("m_")) name = name.Substr(2);
            String nice;
            for (usize i = 0; i < name.Size(); ++i)
            {
                char c = name[i];
                if (i > 0 && c >= 'A' && c <= 'Z' && name[i - 1] >= 'a' && name[i - 1] <= 'z')
                    nice += ' ';
                if (i == 0 && c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');
                nice += c;
            }
            return nice;
        }

        void ClassPicker(ScriptComponent& script, const ScriptClassInfo* current)
        {
            static char filter[64] = "";
            const char* preview = script.ClassName.Empty() ? "(none)" : script.ClassName.Raw();
            if (!current && !script.ClassName.Empty())
                ImGui::PushStyleColor(ImGuiCol_Text, kBad);
            bool open = ImGui::BeginCombo("##ScriptClass", preview,
                                          ImGuiComboFlags_HeightLarge);
            if (!current && !script.ClassName.Empty()) ImGui::PopStyleColor();
            if (!open) return;

            if (ImGui::IsWindowAppearing())
            {
                filter[0] = '\0';
                ImGui::SetKeyboardFocusHere();
            }
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputTextWithHint("##Filter", "Search classes...", filter, sizeof(filter));
            ImGui::Separator();

            const auto& classes = ScriptEngine::Classes();
            if (classes.Empty())
                ImGui::TextDisabled("No Vortex.Actor subclasses in %s",
                                    ScriptEngine::AppAssemblyPath().Raw());
            StringView lastNamespace = "\x01";
            for (const auto& info : classes)
            {
                StringView needle(filter);
                bool       matches = needle.Empty();
                for (usize i = 0; !matches && i + needle.Size() <= info.FullName.Size(); ++i)
                {
                    usize j = 0;
                    while (j < needle.Size()
                           && (info.FullName[i + j] | 0x20) == (needle[j] | 0x20))
                        ++j;
                    matches = j == needle.Size();
                }
                if (!matches) continue;
                if (info.Namespace.View() != lastNamespace)
                {
                    lastNamespace = info.Namespace.View();
                    ImGui::TextColored(kMuted, "%s",
                                       info.Namespace.Empty() ? "(global)" : info.Namespace.Raw());
                }
                bool selected = info.FullName == script.ClassName;
                ImGui::PushID(info.FullName.Raw());
                ImGui::Indent(8.0f);
                if (ImGui::Selectable(info.Name.Raw(), selected))
                {
                    if (script.ClassName != info.FullName) script.Fields.Clear();
                    script.ClassName = info.FullName;
                }
                ImGui::SameLine();
                ImGui::TextDisabled("%zu fields", static_cast<size_t>(info.Fields.Size()));
                ImGui::Unindent(8.0f);
                if (selected) ImGui::SetItemDefaultFocus();
                ImGui::PopID();
            }
            ImGui::EndCombo();
        }

        void ScriptFields(Actor& actor, ScriptComponent& script, const ScriptClassInfo& info,
                          bool live)
        {
            u64 entityID = static_cast<u64>(static_cast<entt::entity>(actor));
            usize overridden = 0;
            for (const auto& field : info.Fields)
                if (script.FindField(field.Name)) ++overridden;

            char title[96];
            std::snprintf(title, sizeof(title), "Fields (%zu)",
                          static_cast<size_t>(info.Fields.Size()));
            ImGui::SeparatorText(title);
            if (live) Pill("LIVE", kGood);
            else if (overridden)
            {
                std::snprintf(title, sizeof(title), "%zu edited",
                              static_cast<size_t>(overridden));
                Pill(title, kAccent);
                ImGui::SameLine();
                if (ImGui::SmallButton("Reset all")) script.Fields.Clear();
            }
            if (info.Fields.Empty())
            {
                ImGui::TextDisabled("The class has no public fields.");
                return;
            }

            if (!ImGui::BeginTable("##ScriptFields", 3, ImGuiTableFlags_SizingFixedFit))
                return;
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 124.0f);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Reset", ImGuiTableColumnFlags_WidthFixed,
                                    ImGui::GetFrameHeight());
            for (const auto& field : info.Fields)
            {
                ImGui::PushID(field.Name.Raw());
                ScriptFieldValue* edited = live ? nullptr : script.FindField(field.Name);
                char tooltip[160];
                std::snprintf(tooltip, sizeof(tooltip), "%s %s%s", FieldTypeLabel(field),
                              field.Name.Raw(),
                              edited ? "\nEdited for this actor (applied before OnCreate)"
                                     : "");
                Prop(NiceName(field.Name).Raw(), tooltip,
                     edited ? kAccent : ImVec4{0, 0, 0, 0});

                if (!IsOverridable(field.Type))
                {
                    ImGui::TextDisabled("%s (not editable)", field.TypeName.Raw());
                    ImGui::TableSetColumnIndex(2);
                    ImGui::PopID();
                    continue;
                }

                alignas(8) u8 value[16]{};
                String        text;
                if (live)
                {
                    if (!ScriptEngine::GetInstanceField(entityID, field, value, &text))
                        std::memcpy(value, field.Default, sizeof(value));
                }
                else if (edited)
                {
                    std::memcpy(value, edited->Data, sizeof(value));
                    text = edited->Text;
                }
                else
                {
                    std::memcpy(value, field.Default, sizeof(value));
                    text = field.DefaultText;
                }

                if (FieldWidget("##value", field.Type, value, text))
                {
                    if (live) ScriptEngine::SetInstanceField(entityID, field, value, text);
                    else
                    {
                        if (!edited)
                        {
                            ScriptFieldValue& added = script.Fields.EmplaceBack();
                            added.Name = field.Name;
                            added.Type = field.Type;
                            edited     = &added;
                        }
                        std::memcpy(edited->Data, value, sizeof(value));
                        edited->Text = text;
                    }
                }

                ImGui::TableSetColumnIndex(2);
                if (edited)
                {
                    if (ImGui::Button("x", {ImGui::GetFrameHeight(), 0.0f}))
                        script.ResetField(field.Name);
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Reset to the class default");
                }
                ImGui::PopID();
            }
            ImGui::EndTable();

            // Edits whose field no longer exists (renamed or retyped in C#).
            usize stale = 0;
            for (const auto& value : script.Fields)
            {
                bool found = false;
                for (const auto& field : info.Fields)
                    found = found || (field.Name == value.Name && field.Type == value.Type);
                if (!found) ++stale;
            }
            if (stale && !live)
            {
                ImGui::TextColored(kWarning, "%zu edited value(s) match no field anymore.",
                                   static_cast<size_t>(stale));
                ImGui::SameLine();
                if (ImGui::SmallButton("Remove"))
                {
                    for (usize i = 0; i < script.Fields.Size();)
                    {
                        bool found = false;
                        for (const auto& field : info.Fields)
                            found = found
                                 || (field.Name == script.Fields[i].Name
                                     && field.Type == script.Fields[i].Type);
                        if (found) ++i;
                        else script.Fields.Erase(script.Fields.begin() + i);
                    }
                }
            }
        }

        void ScriptBuildBar(EditorContext& context)
        {
            auto& scripts  = context.Scripts;
            bool  running  = context.Scenes.IsRunning();
            ImGui::Spacing();
            ImGui::BeginDisabled(scripts.IsBuilding());
            if (ImGui::Button("Build Scripts")) context.Commands.Execute("Scripts.Build");
            ImGui::EndDisabled();
            ImGui::SameLine();
            // Reloading destroys the running instances: edit mode only.
            ImGui::BeginDisabled(running || scripts.IsBuilding());
            if (ImGui::Button("Reload")) ScriptEngine::ReloadAssembly();
            ImGui::EndDisabled();
            if (running && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                ImGui::SetTooltip("Stop the scene to reload scripts");

            if (context.Preferences.AutoCompileScripts)
            {
                Pill("auto", kMuted);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Scripts rebuild when a source changes "
                                      "(Settings > General)");
            }
            if (scripts.HasPendingChanges()) Pill("Changes detected", kWarning);
            if (scripts.IsReloadPending())
            {
                Pill("Reloads on Stop", kAccent);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("A new build is ready; it is loaded when the "
                                      "scene stops");
            }

            switch (scripts.State())
            {
                case ScriptBuildState::eBuilding:
                {
                    static const char* kDots[] = {"", ".", "..", "..."};
                    char text[32];
                    std::snprintf(text, sizeof(text), "Building%s",
                                  kDots[static_cast<int>(ImGui::GetTime() * 3.0) % 4]);
                    Pill(text, kWarning);
                    break;
                }
                case ScriptBuildState::eSucceeded: Pill("Build OK", kGood); break;
                case ScriptBuildState::eFailed:
                    Pill("Build failed", kBad);
                    if (ImGui::BeginItemTooltip())
                    {
                        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 40.0f);
                        StringView output = scripts.Output().View();
                        // The tail holds the compiler errors.
                        if (output.Size() > 1500) output = output.Substr(output.Size() - 1500);
                        ImGui::TextUnformatted(output.Raw(), output.Raw() + output.Size());
                        ImGui::PopTextWrapPos();
                        ImGui::EndTooltip();
                    }
                    break;
                default: break;
            }
        }
    } // namespace

    //------------------------------------------------------------------------
    // Script component ------------------------------------------------------
    //------------------------------------------------------------------------

    void ScriptComponentPanel(Actor& actor, ScriptComponent& script, EditorContext& context)
    {
        bool loaded = ScriptEngine::IsLoaded();
        u64  id     = static_cast<u64>(static_cast<entt::entity>(actor));
        bool live   = context.Scenes.IsRunning() && ScriptEngine::HasInstance(id);
        const ScriptClassInfo* info
            = loaded ? ScriptEngine::FindClass(script.ClassName) : nullptr;

        // Status line: what state the script is in, at a glance.
        if (!loaded) Pill("Scripting not loaded", kBad, false);
        else if (live) Pill("Running", kGood, false);
        else if (context.Scenes.IsRunning()) Pill("Not running", kWarning, false);
        else if (info) Pill("Ready", kAccent, false);
        else if (script.ClassName.Empty()) Pill("No class", kWarning, false);
        else Pill("Class not found", kBad, false);
        if (info && !info->BaseName.Empty() && info->BaseName != "Vortex.Actor")
        {
            String base = String("extends ") + info->BaseName;
            Pill(base.Raw(), kMuted);
        }

        if (BeginProps("##ScriptProps"))
        {
            Prop("Class", "The Vortex.Actor subclass this actor runs");
            if (loaded) ClassPicker(script, info);
            else
            {
                String name = script.ClassName;
                if (UI::InputText("##ScriptClassName", name)) script.ClassName = name;
            }

            if (info)
            {
                StringView source = SourceOf(*info);
                Prop("Source", "The C# file named after the class");
                if (source.Empty()) ImGui::TextDisabled("%s.cs not found", info->Name.Raw());
                else
                {
                    f32 button = ImGui::CalcTextSize("Open").x
                               + ImGui::GetStyle().FramePadding.x * 2.0f;
                    ImGui::SetNextItemWidth(-button - ImGui::GetStyle().ItemSpacing.x);
                    String path(source);
                    UI::InputText("##ScriptSource", path, ImGuiInputTextFlags_ReadOnly);
                    ImGui::SameLine();
                    if (ImGui::Button("Open"))
                        ConsoleWindow::OpenFileInIDE(IDEType::eVSCode, path, 1);
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Open in VS Code");
                }
            }
            Prop("Assembly");
            ImGui::TextUnformatted(ScriptEngine::AppAssemblyPath().Raw());
            Pill(loaded ? "loaded" : "not loaded", loaded ? kGood : kBad);
            if (live)
            {
                char text[32];
                std::snprintf(text, sizeof(text), "%llu", static_cast<unsigned long long>(id));
                PropText("Actor ID", text, kMuted);
            }
            EndProps();
        }

        if (!loaded)
            ImGui::TextWrapped("The .NET runtime or the game assembly failed to load. Build "
                               "the scripts, then reload.");
        else if (!info && !script.ClassName.Empty())
            ImGui::TextColored(kBad, "'%s' isn't a Vortex.Actor subclass of the assembly. "
                                     "Rebuild the scripts or pick another class.",
                               script.ClassName.Raw());
        if (info) ScriptFields(actor, script, *info, live);

        ScriptBuildBar(context);
    }

    //------------------------------------------------------------------------
    // Textures ----------------------------------------------------------------
    //------------------------------------------------------------------------

    void TexturePanel(const ::Ref<RHI::Texture>& texture)
    {
        if (!texture)
        {
            ImGui::TextDisabled("No texture");
            return;
        }
        // Preview, fit to the panel width (at most 192 px).
        f32 width  = ImGui::GetContentRegionAvail().x;
        f32 side   = width < 192.0f ? width : 192.0f;
        f32 aspect = texture->Height() ? static_cast<f32>(texture->Width())
                                             / static_cast<f32>(texture->Height())
                                       : 1.0f;
        ImVec2 size = aspect >= 1.0f ? ImVec2{side, side / aspect} : ImVec2{side * aspect, side};
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (width - size.x) * 0.5f);
        UI::Image(texture, size);

        char buffer[64];
        if (BeginProps("##TextureProps"))
        {
            const auto& name = texture->Desc().DebugName;
            Prop("Source");
            ReadOnly("##TextureSource", name.Empty() ? StringView("(generated)") : name.View());
            std::snprintf(buffer, sizeof(buffer), "%u x %u", texture->Width(), texture->Height());
            PropText("Size", buffer);
            PropText("Format", FormatName(texture->Format()));
            std::snprintf(buffer, sizeof(buffer), "%u", texture->MipLevels());
            PropText("Mip Levels", buffer);
            if (texture->Layers() > 1)
            {
                std::snprintf(buffer, sizeof(buffer), "%u%s", texture->Layers(),
                              texture->Layers() == 6 ? " (cube)" : "");
                PropText("Layers", buffer);
            }
            FormatBytes(buffer, sizeof(buffer), TextureMemory(*texture));
            PropText("GPU Memory", buffer);
            EndProps();
        }
    }

    //------------------------------------------------------------------------
    // Shaders ---------------------------------------------------------------
    //------------------------------------------------------------------------

    void ShaderPanel(RHI::Shader& shader)
    {
        auto&  library = Renderer::ShaderLibrary();
        String name    = library.NameOf(shader);
        Path   source  = name.Empty() ? Path{} : library.SourceFile(name);
        char   buffer[96];

        if (BeginProps("##ShaderProps"))
        {
            PropText("Name", name.Empty() ? String(shader.Name()).Raw() : name.Raw());
            Prop("Source");
            if (source.Empty()) ImGui::TextDisabled("(built in memory)");
            else
            {
                f32 buttons = ImGui::CalcTextSize("OpenReload").x
                            + ImGui::GetStyle().FramePadding.x * 4.0f
                            + ImGui::GetStyle().ItemSpacing.x * 2.0f;
                ImGui::SetNextItemWidth(-buttons);
                String path(StringView(source.Raw(), source.Size()));
                UI::InputText("##ShaderSource", path, ImGuiInputTextFlags_ReadOnly);
                ImGui::SameLine();
                if (ImGui::Button("Open"))
                    ConsoleWindow::OpenFileInIDE(IDEType::eVSCode, path, 1);
                ImGui::SameLine();
                if (ImGui::Button("Reload")) library.Reload(name);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Recompile; materials using it follow");
            }
            Prop("Stages");
            ImGui::TextUnformatted(StageList(shader.Stages()).Raw());
            const auto& layout = shader.Layout();
            for (const auto& range : layout.PushConstants)
            {
                std::snprintf(buffer, sizeof(buffer), "%u bytes @ %u  (%s)", range.Size,
                              range.Offset, StageList(range.Stages).Raw());
                PropText("Push Constants", buffer);
            }
            EndProps();
        }

        const auto& sets  = shader.Layout().Sets;
        usize       total = 0;
        for (const auto& set : sets) total += set.Entries.Size();
        std::snprintf(buffer, sizeof(buffer), "Bindings (%zu)", static_cast<size_t>(total));
        ImGui::SeparatorText(buffer);
        if (total == 0)
        {
            ImGui::TextDisabled("No resources");
            return;
        }
        if (ImGui::BeginTable("##ShaderBindings", 5,
                              ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV
                                  | ImGuiTableFlags_SizingFixedFit))
        {
            ImGui::TableSetupColumn("Set.Binding");
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("Stages");
            ImGui::TableSetupColumn("Size");
            ImGui::TableHeadersRow();
            for (u32 set = 0; set < sets.Size(); ++set)
            {
                for (const auto& entry : sets[set].Entries)
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    // Set 0 is the renderer's; materials own the rest.
                    ImGui::TextColored(set == 0 ? kMuted : kAccent, "%u.%u", set, entry.Binding);
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(entry.Name.Raw());
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(BindingTypeName(entry.Type));
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(StageList(entry.Stages).Raw());
                    ImGui::TableNextColumn();
                    if (entry.Size) ImGui::Text("%u B", entry.Size);
                    else ImGui::TextDisabled("-");
                }
            }
            ImGui::EndTable();
        }
    }

    //------------------------------------------------------------------------
    // Materials -------------------------------------------------------------
    //------------------------------------------------------------------------

    void MaterialPanel(Material& material, const char* id, MeshComponent* owner)
    {
        ImGui::PushID(id);
        auto& library = Renderer::ShaderLibrary();
        auto  shader  = material.Shader();
        String shaderName = shader ? library.NameOf(*shader) : String{};

        if (BeginProps("##MaterialProps"))
        {
            Prop("Shader", "The shader the material draws with; textures carry over "
                           "to bindings of the same name");
            if (ImGui::BeginCombo("##Shader", shaderName.Empty()
                                                  ? (shader ? String(shader->Name()).Raw()
                                                            : "(none)")
                                                  : shaderName.Raw()))
            {
                for (const auto& name : library.Names())
                {
                    auto candidate = library.Find(name);
                    // Only surface shaders: a material set (1+) and a
                    // vertex stage.
                    if (!candidate || candidate->Layout().Sets.Size() < 2
                        || !(static_cast<u32>(candidate->Stages())
                             & static_cast<u32>(RHI::ShaderStage::eVertex)))
                        continue;
                    if (ImGui::Selectable(name.Raw(), name == shaderName))
                        material.SetShader(candidate);
                    if (name == shaderName) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            auto& surface = material.Surface;
            Prop("Albedo", "Base color, multiplied with the albedo texture");
            ImGui::ColorEdit4("##Albedo", surface.Albedo,
                              ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaBar);
            Prop("Metallic");
            ImGui::SliderFloat("##Metallic", &surface.Metallic, 0.0f, 1.0f, "%.2f");
            Prop("Roughness");
            ImGui::SliderFloat("##Roughness", &surface.Roughness, 0.0f, 1.0f, "%.2f");
            EndProps();
        }

        // Texture slots -------------------------------------------------
        auto  bindings = material.Bindings();
        usize textures = 0;
        for (const auto& binding : bindings) textures += IsTextureBinding(binding.Type);
        char  title[48];
        std::snprintf(title, sizeof(title), "Textures (%zu)", static_cast<size_t>(textures));
        ImGui::SeparatorText(title);
        if (textures == 0) ImGui::TextDisabled("The shader samples no material textures.");

        const auto white = Renderer::WhiteTexture();
        for (const auto& binding : bindings)
        {
            if (!IsTextureBinding(binding.Type)) continue;
            ImGui::PushID(binding.Name.Raw());
            const f32 thumb = ImGui::GetFrameHeight() * 2.0f + ImGui::GetStyle().ItemSpacing.y;
            bool      empty = !binding.Texture || binding.Texture == white;

            Thumbnail(empty ? white : binding.Texture, thumb);
            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::TextUnformatted(NiceName(binding.Name).Raw());
            ImGui::SameLine();
            ImGui::TextDisabled("set %u.%u", binding.Set, binding.Binding);
            if (empty) ImGui::TextDisabled("None (white)");
            else
            {
                StringView source = binding.Texture->Desc().DebugName.View();
                ImGui::TextDisabled("%s  %ux%u", String(FileName(source)).Raw(),
                                    binding.Texture->Width(), binding.Texture->Height());
                if (ImGui::IsItemHovered() && !source.Empty())
                    ImGui::SetTooltip("%s", String(source).Raw());
            }
            ImGui::EndGroup();

            // Browse / clear, right-aligned on the slot's first line.
            f32 buttons = ImGui::CalcTextSize("...").x + ImGui::CalcTextSize("x").x
                        + ImGui::GetStyle().FramePadding.x * 4.0f
                        + ImGui::GetStyle().ItemSpacing.x;
            ImGui::SameLine(ImGui::GetContentRegionMax().x - buttons);
            if (ImGui::Button("..."))
            {
                if (auto path = PickFile())
                {
                    auto asset = CreateRef<TextureAsset>(*path);
                    material.Set(binding.Name, asset->Texture());
                    if (owner && binding.Name == "texSampler") owner->Texture = asset;
                }
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Load a texture");
            ImGui::SameLine();
            ImGui::BeginDisabled(empty);
            if (ImGui::Button("x"))
            {
                material.Set(binding.Name, white);
                if (owner && binding.Name == "texSampler") owner->Texture = nullptr;
            }
            ImGui::EndDisabled();
            ImGui::PopID();
        }

        // Buffers are set from code; list them so nothing is a mystery.
        for (const auto& binding : bindings)
        {
            if (IsTextureBinding(binding.Type) || binding.Type == RHI::BindingType::eSampler)
                continue;
            ImGui::BulletText("%s  (%s%s)", binding.Name.Raw(), BindingTypeName(binding.Type),
                              binding.Buffer ? "" : ", unset");
        }

        if (shader && ImGui::TreeNodeEx("Shader", ImGuiTreeNodeFlags_SpanAvailWidth))
        {
            ShaderPanel(*shader);
            ImGui::TreePop();
        }
        ImGui::PopID();
    }

    //------------------------------------------------------------------------
    // Meshes ----------------------------------------------------------------
    //------------------------------------------------------------------------

    void MeshPanel(Mesh& mesh)
    {
        char buffer[64];
        u64  vertices  = mesh.Vertices().Size();
        u64  triangles = mesh.IndexCount() / 3;

        // Stat tiles: the numbers that matter for cost, at a glance.
        if (ImGui::BeginTable("##MeshStats", 4, ImGuiTableFlags_SizingStretchSame))
        {
            auto tile = [&](const char* label, u64 value)
            {
                ImGui::TableNextColumn();
                FormatCount(buffer, sizeof(buffer), value);
                ImGui::PushFont(nullptr, ImGui::GetStyle().FontSizeBase * 1.2f);
                ImGui::TextUnformatted(buffer);
                ImGui::PopFont();
                ImGui::TextDisabled("%s", label);
            };
            tile("vertices", vertices);
            tile("triangles", triangles);
            tile("submeshes", mesh.Submeshes().Size());
            tile("materials", mesh.Materials().Size());
            ImGui::EndTable();
        }

        if (BeginProps("##MeshProps"))
        {
            const auto& box  = mesh.AABB();
            Math::Vec3  size = box.Max - box.Min;
            std::snprintf(buffer, sizeof(buffer), "%.2f x %.2f x %.2f", size.x, size.y, size.z);
            PropText("Bounds", buffer);
            u64 memory = vertices * sizeof(Mesh::Vertex) + mesh.IndexCount() * sizeof(u32);
            FormatBytes(buffer, sizeof(buffer), memory);
            PropText("GPU Memory", buffer);
            Prop("Skinning");
            if (mesh.IsSkinned())
            {
                std::snprintf(buffer, sizeof(buffer), "%zu bones",
                              static_cast<size_t>(mesh.Skeleton().Bones.Size()));
                ImGui::TextUnformatted(buffer);
                Pill("skinned", kAccent);
            }
            else ImGui::TextDisabled("Static");
            if (!mesh.Animations().Empty())
            {
                std::snprintf(buffer, sizeof(buffer), "%zu clips",
                              static_cast<size_t>(mesh.Animations().Size()));
                PropText("Animations", buffer);
            }
            EndProps();
        }

        const auto& submeshes = mesh.Submeshes();
        std::snprintf(buffer, sizeof(buffer), "Submeshes (%zu)###Submeshes",
                      static_cast<size_t>(submeshes.Size()));
        if (submeshes.Size() > 1 && ImGui::TreeNodeEx(buffer, ImGuiTreeNodeFlags_SpanAvailWidth))
        {
            f32 height = ImGui::GetTextLineHeightWithSpacing()
                       * static_cast<f32>(submeshes.Size() < 10 ? submeshes.Size() + 1 : 11);
            if (ImGui::BeginTable("##Submeshes", 3,
                                  ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY
                                      | ImGuiTableFlags_SizingFixedFit,
                                  {0.0f, height}))
            {
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Triangles");
                ImGui::TableSetupColumn("Material");
                ImGui::TableHeadersRow();
                ImGuiListClipper clipper;
                clipper.Begin(static_cast<int>(submeshes.Size()));
                while (clipper.Step())
                {
                    for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
                    {
                        const auto& submesh = submeshes[static_cast<usize>(row)];
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        const String& label = !submesh.MeshName.Empty() ? submesh.MeshName
                                                                        : submesh.NodeName;
                        ImGui::TextUnformatted(label.Empty() ? "(unnamed)" : label.Raw());
                        ImGui::TableNextColumn();
                        FormatCount(buffer, sizeof(buffer), submesh.IndexCount / 3);
                        ImGui::TextUnformatted(buffer);
                        ImGui::TableNextColumn();
                        const auto& materials = mesh.Materials();
                        if (submesh.MaterialIndex < materials.Size()
                            && !materials[submesh.MaterialIndex].Name.Empty())
                            ImGui::TextUnformatted(materials[submesh.MaterialIndex].Name.Raw());
                        else ImGui::Text("#%u", submesh.MaterialIndex);
                    }
                }
                ImGui::EndTable();
            }
            ImGui::TreePop();
        }
    }

    //------------------------------------------------------------------------
    // Mesh component --------------------------------------------------------
    //------------------------------------------------------------------------

    void MeshComponentPanel(Actor& actor, MeshComponent& component, EditorContext& context)
    {
        IgnoreUnused(actor, context);

        // The mesh asset slot.
        if (BeginProps("##MeshAsset"))
        {
            Prop("Mesh", "The model file (anything assimp reads)");
            f32 buttons = ImGui::CalcTextSize("...").x + ImGui::CalcTextSize("x").x
                        + ImGui::GetStyle().FramePadding.x * 4.0f
                        + ImGui::GetStyle().ItemSpacing.x * 2.0f;
            ImGui::SetNextItemWidth(-buttons);
            String path = component.Mesh
                            ? String(StringView(component.Mesh->FilePath().Raw(),
                                                component.Mesh->FilePath().Size()))
                            : String("None");
            UI::InputText("##MeshPath", path, ImGuiInputTextFlags_ReadOnly);
            if (ImGui::IsItemHovered() && component.Mesh) ImGui::SetTooltip("%s", path.Raw());
            ImGui::SameLine();
            if (ImGui::Button("...##Mesh"))
                if (auto picked = PickFile()) component.Mesh = CreateRef<Mesh>(*picked);
            ImGui::SameLine();
            ImGui::BeginDisabled(!component.Mesh);
            if (ImGui::Button("x##Mesh")) component.Mesh = nullptr;
            ImGui::EndDisabled();
            EndProps();
        }
        if (component.Mesh) MeshPanel(*component.Mesh);
        else ImGui::TextDisabled("No mesh: nothing is drawn.");

        // Materials: one per file material, or the component's own.
        if (!component.SubmeshMaterials.Empty())
        {
            char title[48];
            std::snprintf(title, sizeof(title), "Materials (%zu)",
                          static_cast<size_t>(component.SubmeshMaterials.Size()));
            ImGui::SeparatorText(title);
            const auto* infos = component.Mesh ? &component.Mesh->Materials() : nullptr;
            for (usize i = 0; i < component.SubmeshMaterials.Size(); ++i)
            {
                auto& material = component.SubmeshMaterials[i];
                if (!material) continue;
                char label[160];
                const char* name = infos && i < infos->Size() && !(*infos)[i].Name.Empty()
                                     ? (*infos)[i].Name.Raw()
                                     : "Material";
                std::snprintf(label, sizeof(label), "%s  #%zu###Material%zu", name,
                              static_cast<size_t>(i), static_cast<size_t>(i));
                if (ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_SpanAvailWidth))
                {
                    MaterialPanel(*material, label);
                    ImGui::TreePop();
                }
            }
            return;
        }

        ImGui::SeparatorText("Material");
        if (component.Material) MaterialPanel(*component.Material, "##Material", &component);
        else
        {
            ImGui::TextDisabled("No material");
            ImGui::SameLine();
            if (ImGui::SmallButton("Create"))
                component.Material = Material::Create(Renderer::ShaderLibrary().Get("PBR"));
        }
    }
} // namespace Vortex::Editor::Inspector

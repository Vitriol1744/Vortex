/*
 * Created by v1tr10l7 on 30.05.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Renderer/Export.hpp>

#include <imgui.h>

import Prism.Memory.Ref;
import Prism.String.String;
import Prism.Utility.Path;
import Vortex.RHI;

namespace Vortex
{
    namespace UI
    {
        /// ImTextureID for an RHI texture, valid for this frame. The texture
        /// must rest in eShaderRead (scene targets and assets do).
        VT_RENDERER_API ImTextureID TextureID(const ::Ref<RHI::Texture>& texture);
        /// ImGui::Image of an RHI texture; size defaults to the texture's.
        VT_RENDERER_API void        Image(const ::Ref<RHI::Texture>& texture,
                                          ImVec2 size = {0, 0},
                                          ImVec2 uv0 = {0, 0}, ImVec2 uv1 = {1, 1});
        VT_RENDERER_API bool ImageButton(const char*                 id,
                                         const ::Ref<RHI::Texture>& texture,
                                         ImVec2 size, ImVec4 tint = {1, 1, 1, 1});

        VT_RENDERER_API bool          InputText(const char* label, String& str,
                                ImGuiInputTextFlags    flags    = 0,
                                ImGuiInputTextCallback callback = nullptr,
                                void*                  userData = nullptr);
        VT_RENDERER_API bool          InputText(const char* label, Path& str,
                                ImGuiInputTextFlags    flags    = 0,
                                ImGuiInputTextCallback callback = nullptr,
                                void*                  userData = nullptr);

        VT_RENDERER_API bool          InputTextMultiline(const char* label, String& path,
                                         const ImVec2&          size = ImVec2(0, 0),
                                         ImGuiInputTextFlags    flags = 0,
                                         ImGuiInputTextCallback callback = nullptr,
                                         void*                  userData = nullptr);
        VT_RENDERER_API bool          InputTextMultiline(const char* label, Path& path,
                                         const ImVec2&          size = ImVec2(0, 0),
                                         ImGuiInputTextFlags    flags = 0,
                                         ImGuiInputTextCallback callback = nullptr,
                                         void*                  userData = nullptr);

        VT_RENDERER_API bool InputTextWithHint(const char* label, const char* hint, String& str,
                               ImGuiInputTextFlags    flags    = 0,
                               ImGuiInputTextCallback callback = nullptr,
                               void*                  userData = nullptr);
        VT_RENDERER_API bool InputTextWithHint(const char* label, const char* hint, Path& path,
                               ImGuiInputTextFlags    flags    = 0,
                               ImGuiInputTextCallback callback = nullptr,
                               void*                  userData = nullptr);
    }; // namespace UI
}; // namespace Vortex

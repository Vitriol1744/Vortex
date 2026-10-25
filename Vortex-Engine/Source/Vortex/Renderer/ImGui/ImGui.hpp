/*
 * Created by v1tr10l7 on 30.05.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Renderer/RHI/SwapChain.hpp>
#include <Vortex/Renderer/RHI/Texture2D.hpp>

#include <imgui.h>

namespace Vortex
{
    namespace UI
    {
        void*         GetTexture(Ref<Texture2D> texture);
        void*         TextureFromFramebuffer(Ref<Framebuffer> framebuffer);
        Vector<void*> GetTextures(Ref<SwapChain> swapChain);

        void          RemoveTexture(void* texture);

        bool          InputText(const char* label, String& str,
                                ImGuiInputTextFlags    flags    = 0,
                                ImGuiInputTextCallback callback = nullptr,
                                void*                  userData = nullptr);
        bool          InputText(const char* label, Path& str,
                                ImGuiInputTextFlags    flags    = 0,
                                ImGuiInputTextCallback callback = nullptr,
                                void*                  userData = nullptr);

        bool          InputTextMultiline(const char* label, String& path,
                                         const ImVec2&          size = ImVec2(0, 0),
                                         ImGuiInputTextFlags    flags = 0,
                                         ImGuiInputTextCallback callback = nullptr,
                                         void*                  userData = nullptr);
        bool          InputTextMultiline(const char* label, Path& path,
                                         const ImVec2&          size = ImVec2(0, 0),
                                         ImGuiInputTextFlags    flags = 0,
                                         ImGuiInputTextCallback callback = nullptr,
                                         void*                  userData = nullptr);

        bool InputTextWithHint(const char* label, const char* hint, String& str,
                               ImGuiInputTextFlags    flags    = 0,
                               ImGuiInputTextCallback callback = nullptr,
                               void*                  userData = nullptr);
        bool InputTextWithHint(const char* label, const char* hint, Path& path,
                               ImGuiInputTextFlags    flags    = 0,
                               ImGuiInputTextCallback callback = nullptr,
                               void*                  userData = nullptr);
    }; // namespace UI
}; // namespace Vortex

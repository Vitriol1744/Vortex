/*
 * Created by v1tr10l7 on 30.05.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Renderer/ImGui/ImGui.hpp>
#include <Vortex/Renderer/ImGui/ImGuiLayer.hpp>

namespace Vortex
{
    namespace UI
    {
        namespace
        {

            struct InputTextCallback_UserData
            {
                String*                Str;
                ImGuiInputTextCallback ChainCallback;
                void*                  ChainCallbackUserData;
            };

            static i32 StringInputTextCallback(ImGuiInputTextCallbackData* data)
            {
                auto* userData
                    = static_cast<InputTextCallback_UserData*>(data->UserData);

                if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
                {
                    String* str = userData->Str;
                    IM_ASSERT(data->Buf == str->Raw());

                    // Resize internal storage to fit new character count
                    str->Resize(static_cast<usize>(data->BufTextLen));

                    // Re-bind the buffer pointer back to ImGui
                    data->Buf = const_cast<char*>(str->Raw());
                }
                else if (userData->ChainCallback)
                {
                    // Forward event to user-provided callback if present
                    data->UserData = userData->ChainCallbackUserData;
                    return userData->ChainCallback(data);
                }

                return 0;
            }
        } // namespace

        ImTextureID TextureID(const ::Ref<RHI::Texture>& texture)
        {
            return ImGuiLayer::VortexBackend::Get().TextureID(texture);
        }
        void Image(const ::Ref<RHI::Texture>& texture, ImVec2 size, ImVec2 uv0,
                   ImVec2 uv1)
        {
            if (!texture) return;
            if (size.x <= 0.0f || size.y <= 0.0f)
                size = ImVec2(static_cast<f32>(texture->Width()),
                              static_cast<f32>(texture->Height()));
            ImGui::Image(ImTextureRef(TextureID(texture)), size, uv0, uv1);
        }
        bool ImageButton(const char* id, const ::Ref<RHI::Texture>& texture,
                         ImVec2 size, ImVec4 tint)
        {
            return ImGui::ImageButton(id, ImTextureRef(TextureID(texture)), size,
                                      {0, 0}, {1, 1}, {0, 0, 0, 0}, tint);
        }

        bool InputText(const char* label, String& str,
                       ImGuiInputTextFlags    flags,
                       ImGuiInputTextCallback callback, void* userData)
        {
            IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
            flags |= ImGuiInputTextFlags_CallbackResize;

            InputTextCallback_UserData cbUserData;
            cbUserData.Str                   = &str;
            cbUserData.ChainCallback         = callback;
            cbUserData.ChainCallbackUserData = userData;

            return ImGui::InputText(label, const_cast<char*>(str.Raw()),
                                    str.Capacity() + 1, flags,
                                    StringInputTextCallback, &cbUserData);
        }

        bool InputText(const char* label, Path& path, ImGuiInputTextFlags flags,
                       ImGuiInputTextCallback callback, void* userData)
        {
            return InputText(label, path.String(), flags, callback, userData);
        }

        bool InputTextMultiline(const char* label, String& str,
                                const ImVec2& size, ImGuiInputTextFlags flags,
                                ImGuiInputTextCallback callback, void* userData)
        {
            IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
            flags |= ImGuiInputTextFlags_CallbackResize;

            InputTextCallback_UserData cbUserData;
            cbUserData.Str                   = &str;
            cbUserData.ChainCallback         = callback;
            cbUserData.ChainCallbackUserData = userData;

            return ImGui::InputTextMultiline(
                label, const_cast<char*>(str.Raw()), str.Capacity() + 1, size,
                flags, StringInputTextCallback, &cbUserData);
        }
        bool InputTextMultiline(const char* label, Path& path,
                                const ImVec2& size, ImGuiInputTextFlags flags,
                                ImGuiInputTextCallback callback, void* userData)
        {
            return InputTextMultiline(label, path.String(), size, flags,
                                      callback, userData);
        }

        bool InputTextWithHint(const char* label, const char* hint, String& str,
                               ImGuiInputTextFlags    flags,
                               ImGuiInputTextCallback callback, void* userData)
        {
            IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
            flags |= ImGuiInputTextFlags_CallbackResize;

            InputTextCallback_UserData cbUserData;
            cbUserData.Str                   = &str;
            cbUserData.ChainCallback         = callback;
            cbUserData.ChainCallbackUserData = userData;

            return ImGui::InputTextWithHint(
                label, hint, const_cast<char*>(str.Raw()), str.Capacity() + 1,
                flags, StringInputTextCallback, &cbUserData);
        }
        bool InputTextWithHint(const char* label, const char* hint, Path& path,
                               ImGuiInputTextFlags    flags,
                               ImGuiInputTextCallback callback, void* userData)
        {
            return InputTextWithHint(label, hint, path.String(), flags,
                                     callback, userData);
        }
    }; // namespace UI
}; // namespace Vortex

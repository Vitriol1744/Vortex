/*
 * Created by v1tr10l7 on 30.05.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/UnorderedMap.hpp>

#include <Vortex/Renderer/ImGui/ImGui.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanSwapChain.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanTexture2D.hpp>
#include <Vortex/Renderer/RHI/Vulkan/imgui_impl_vulkan.h>

namespace Vortex
{
    namespace UI
    {
        namespace
        {
            UnorderedMap<upointer, ImTextureID> s_TextureCache;

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

        void* GetTexture(Ref<Texture2D> texture)
        {
            auto it = s_TextureCache.Find(upointer(texture.Raw()));
            if (it != s_TextureCache.end())
                return reinterpret_cast<void*>(it->Value);

            auto vkTexture     = texture.As<VulkanTexture2D>();
            auto imageView     = vkTexture->ImageView();
            auto descriptorSet = ImGui_ImplVulkan_AddTexture(
                imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            s_TextureCache[upointer(texture.Raw())]
                = reinterpret_cast<ImTextureID>(descriptorSet);

            return descriptorSet;
        }
        void* TextureFromFramebuffer(Ref<Framebuffer> framebuffer)
        {
            auto vkFramebuffer = framebuffer.As<VulkanFramebuffer>();
            return ImGui_ImplVulkan_AddTexture(
                vkFramebuffer->Sampler(), vkFramebuffer->ImageView(),
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }

        Vector<void*> GetTextures(Ref<SwapChain> swapChain)
        {
            auto          vkSwapChain = swapChain.As<VulkanSwapChain>();
            Vector<void*> descriptorSets;

            return {};
        }

        void RemoveTexture(void* texture)
        {
            if (!texture) return;

            ImGui_ImplVulkan_RemoveTexture(
                static_cast<VkDescriptorSet>(texture));
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

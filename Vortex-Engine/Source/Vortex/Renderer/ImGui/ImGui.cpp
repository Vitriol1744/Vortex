/*
 * Created by v1tr10l7 on 30.05.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/UnorderedMap.hpp>

#include <Vortex/Renderer/API/Vulkan/VulkanTexture2D.hpp>
#include <Vortex/Renderer/API/Vulkan/VulkanSwapChain.hpp>
#include <Vortex/Renderer/API/Vulkan/imgui_impl_vulkan.h>
#include <Vortex/Renderer/ImGui/ImGui.hpp>

namespace Vortex
{
    namespace UI
    {
        UnorderedMap<upointer, ImTextureID> s_TextureCache;

        void*                               GetTexture(Ref<Texture2D> texture)
        {
            auto it = s_TextureCache.Find(upointer(texture.Raw()));
            if (it != s_TextureCache.end()) return it->Value;

            auto vkTexture     = texture.As<VulkanTexture2D>();
            auto sampler       = vkTexture->Sampler();
            auto imageView     = vkTexture->ImageView();
            auto descriptorSet = ImGui_ImplVulkan_AddTexture(
                sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            s_TextureCache[upointer(texture.Raw())] = descriptorSet;

            return descriptorSet;
        }
        Vector<void*> GetTextures(Ref<SwapChain> swapChain)
        {
            auto vkSwapChain = swapChain.As<VulkanSwapChain>();
            Vector<void*> descriptorSets;

            return {};
        }
    }; // namespace UI
}; // namespace Vortex

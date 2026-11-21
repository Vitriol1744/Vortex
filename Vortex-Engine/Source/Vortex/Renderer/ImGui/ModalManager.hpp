/*
 * Created by v1tr10l7 on 10.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

import Prism.Memory.Ref;
import Prism.Utility.Delegate;
#include <Vortex/Core/Core.hpp>

#include <imgui.h>

namespace Vortex::UI
{
    class Modal : public RefCounted<Modal>
    {
      public:
        virtual ~Modal()                         = default;
        virtual const char*      Title() const   = 0;
        virtual bool             OnImGuiRender() = 0;
        virtual ImGuiWindowFlags Flags() const
        {
            return ImGuiWindowFlags_AlwaysAutoResize;
        }
        virtual ImVec2 MinSize() const { return {600.0f, 0.0f}; }
    };

    class ModalManager
    {
      public:
        template <typename T, typename... Args>
        static Ref<T> Open(Args&&... args)
        {
            static_assert(Prism::IsBaseOfV<Modal, T>,
                          "T must derive from Modal");
            auto modal = CreateRef<T>(Forward<Args>(args)...);
            s_PendingQueue.PushBack(modal);
            return modal;
        }

        // Open a quick inline lambda modal for simple popups/alerts
        static void ShowLambda(String title, Delegate<bool()> renderFunc,
                               ImVec2 minSize = {400.0f, 0.0f})
        {
            class LambdaModal : public Modal
            {
              public:
                LambdaModal(String t, Delegate<bool()> f, ImVec2 s)
                    : m_Title(Move(t))
                    , m_RenderFunc(Move(f))
                    , m_MinSize(s)
                {
                }

                const char* Title() const override { return m_Title.Raw(); }
                ImVec2      MinSize() const override { return m_MinSize; }
                bool        OnImGuiRender() override
                {
                    return m_RenderFunc ? m_RenderFunc() : false;
                }

              private:
                String           m_Title;
                Delegate<bool()> m_RenderFunc;
                ImVec2           m_MinSize;
            };

            IgnoreUnused(
                Open<LambdaModal>(Move(title), Move(renderFunc), minSize));
        }

        static void RenderModals()
        {
            for (auto& modal : s_PendingQueue)
                s_ActiveStack.PushBack({modal, true});
            s_PendingQueue.Clear();

            // Render active stack top-to-bottom
            for (auto it = s_ActiveStack.begin(); it != s_ActiveStack.end();)
            {
                auto& node = *it;

                if (node.ShouldOpenPopup)
                {
                    ImGui::OpenPopup(node.Instance->Title());
                    node.ShouldOpenPopup = false;
                }

                ImVec2 center = ImGui::GetMainViewport()->GetCenter();
                ImGui::SetNextWindowPos(center, ImGuiCond_Appearing,
                                        ImVec2(0.5f, 0.5f));
                ImGui::SetNextWindowSizeConstraints(node.Instance->MinSize(),
                                                    ImVec2(FLT_MAX, FLT_MAX));

                bool isOpen    = true;
                bool keepAlive = true;

                if (ImGui::BeginPopupModal(node.Instance->Title(), &isOpen,
                                           node.Instance->Flags()))
                {
                    keepAlive = node.Instance->OnImGuiRender();
                    ImGui::EndPopup();
                }

                // If user closed via window 'X' or returned false from render
                // function, drop from memory
                if (!isOpen || !keepAlive) it = s_ActiveStack.Erase(it);
                else ++it;
            }
        }

      private:
        struct ModalNode
        {
            Ref<Modal> Instance;
            bool       ShouldOpenPopup = true;
        };

        inline static Vector<Ref<Modal>> s_PendingQueue;
        inline static Vector<ModalNode>  s_ActiveStack;
    };
} // namespace Vortex::UI

/*
 * Created by v1tr10l7 on 15.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <imgui.h>

namespace Vortex
{
    class Panel
    {
      public:
        Panel() = default;
        explicit inline Panel(std::string_view name)
            : m_Name(name)
        {
        }
        virtual ~Panel() = default;

        inline void Draw()
        {
            if (!Enabled) return;
            ImGui::Begin(m_Name.data(), &Enabled);
            OnImGuiRender();
            ImGui::End();
        }

        inline std::string_view GetName() const { return m_Name; }

        bool                    Enabled = true;

      protected:
        std::string  m_Name = "Unnamed";

        virtual void OnImGuiRender() {}
    };
}; // namespace Vortex

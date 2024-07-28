/*
 * Created by v1tr10l7 on 28.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Export.hpp"

namespace Vortex
{
    class VT_API Layer
    {
      public:
        Layer(std::string_view name = "Layer")
            : m_Name(name)
        {
        }
        virtual ~Layer() = default;

        virtual void            OnAttach() {}
        virtual void            OnDetach() {}

        virtual void            OnUpdate() {}
        virtual void            OnRender() {}
        virtual void            OnImGuiRender() {}

        inline std::string_view GetName() const { return m_Name; }

      private:
        std::string m_Name = "Layer";
    };
}; // namespace Vortex

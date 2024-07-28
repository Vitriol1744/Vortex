/*
 * Created by v1tr10l7 on 28.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Assertions.hpp"
#include "Vortex/Core/Layer.hpp"

namespace Vortex
{
    class LayerStack final
    {
      public:
        LayerStack()  = default;
        ~LayerStack() = default;

        void              PushOverlay(Ref<Layer> overlay);
        void              PushLayer(Ref<Layer> layer);

        Ref<Layer>        PopOverlay(Ref<Layer> overlay);
        Ref<Layer>        PopLayer(Ref<Layer> layer);

        inline usize      GetSize() const { return m_Layers.size(); }

        inline Ref<Layer> operator[](usize index)
        {
            VtCoreAssert(index >= 0 && index <= m_Layers.size());
            return m_Layers[index];
        }
        inline const Ref<Layer> operator[](usize index) const
        {
            VtCoreAssert(index >= 0 && index <= m_Layers.size());
            return m_Layers[index];
        }

      private:
        std::vector<Ref<Layer>> m_Layers;
        usize                   m_InsertIndex = 0;
    };
}; // namespace Vortex

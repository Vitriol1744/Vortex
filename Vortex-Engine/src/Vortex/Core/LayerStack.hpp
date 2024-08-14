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
            VtCoreAssert(index <= m_Layers.size());
            return m_Layers[index];
        }
        inline const Ref<Layer> operator[](usize index) const
        {
            VtCoreAssert(index <= m_Layers.size());
            return m_Layers[index];
        }

        std::vector<Ref<Layer>>::iterator begin() { return m_Layers.begin(); }
        std::vector<Ref<Layer>>::iterator end() { return m_Layers.end(); }
        std::vector<Ref<Layer>>::reverse_iterator rbegin()
        {
            return m_Layers.rbegin();
        }
        std::vector<Ref<Layer>>::reverse_iterator rend()
        {
            return m_Layers.rend();
        }

        std::vector<Ref<Layer>>::const_iterator begin() const
        {
            return m_Layers.begin();
        }
        std::vector<Ref<Layer>>::const_iterator end() const
        {
            return m_Layers.end();
        }
        std::vector<Ref<Layer>>::const_reverse_iterator rbegin() const
        {
            return m_Layers.rbegin();
        }
        std::vector<Ref<Layer>>::const_reverse_iterator rend() const
        {
            return m_Layers.rend();
        }

      private:
        std::vector<Ref<Layer>> m_Layers;
        usize                   m_InsertIndex = 0;
    };
}; // namespace Vortex

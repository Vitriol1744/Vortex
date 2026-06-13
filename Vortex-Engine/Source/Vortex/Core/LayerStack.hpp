/*
 * Created by v1tr10l7 on 28.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Assertions.hpp>
#include <Vortex/Core/Layer.hpp>

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

        void              Clear() { m_Layers.Clear(); }

        inline usize      Size() const { return m_Layers.Size(); }

        inline Ref<Layer> operator[](usize index)
        {
            VtCoreAssert(index <= m_Layers.Size());
            return m_Layers[index];
        }
        inline const Ref<Layer> operator[](usize index) const
        {
            VtCoreAssert(index <= m_Layers.Size());
            return m_Layers[index];
        }

        Vector<Ref<Layer>>::Iterator        begin() { return m_Layers.begin(); }
        Vector<Ref<Layer>>::Iterator        end() { return m_Layers.end(); }
        Vector<Ref<Layer>>::ReverseIterator rbegin()
        {
            return m_Layers.rbegin();
        }
        Vector<Ref<Layer>>::ReverseIterator rend() { return m_Layers.rend(); }

        Vector<Ref<Layer>>::ConstIterator   begin() const
        {
            return m_Layers.begin();
        }
        Vector<Ref<Layer>>::ConstIterator end() const { return m_Layers.end(); }
        Vector<Ref<Layer>>::ConstReverseIterator rbegin() const
        {
            return m_Layers.rbegin();
        }
        Vector<Ref<Layer>>::ConstReverseIterator rend() const
        {
            return m_Layers.rend();
        }

      private:
        Vector<Ref<Layer>> m_Layers;
        usize              m_InsertIndex = 0;
    };
}; // namespace Vortex

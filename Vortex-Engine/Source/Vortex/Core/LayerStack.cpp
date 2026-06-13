/*
 * Created by v1tr10l7 on 28.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Algorithm/Find.hpp>

#include <Vortex/Core/LayerStack.hpp>

namespace Vortex
{
    void LayerStack::PushLayer(Ref<Layer> layer)
    {
        m_Layers.Emplace(m_Layers.begin() + m_InsertIndex, layer);
        m_InsertIndex++;

        layer->OnAttach();
    }
    void LayerStack::PushOverlay(Ref<Layer> overlay)
    {
        m_Layers.EmplaceBack(overlay);
        overlay->OnAttach();
    }

    Ref<Layer> LayerStack::PopOverlay(Ref<Layer> overlay)
    {
        auto it = Find(m_Layers.begin(), m_Layers.end(), overlay);
        if (it != m_Layers.end()) m_Layers.Erase(it);

        overlay->OnDetach();
        return overlay;
    }
    Ref<Layer> LayerStack::PopLayer(Ref<Layer> layer)
    {
        auto it = Find(m_Layers.begin(), m_Layers.end(), layer);
        if (it != m_Layers.end())
        {
            m_Layers.Erase(it);
            m_InsertIndex--;
        }

        layer->OnDetach();
        return layer;
    }
}; // namespace Vortex

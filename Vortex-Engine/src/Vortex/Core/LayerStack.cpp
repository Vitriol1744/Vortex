/*
 * Created by v1tr10l7 on 28.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Core/LayerStack.hpp"

namespace Vortex
{
    void LayerStack::PushLayer(Ref<Layer> layer)
    {
        m_Layers.emplace(m_Layers.begin() + m_InsertIndex, layer);
        m_InsertIndex++;

        layer->OnAttach();
    }
    void LayerStack::PushOverlay(Ref<Layer> overlay)
    {
        m_Layers.emplace_back(overlay);
        overlay->OnAttach();
    }

    Ref<Layer> LayerStack::PopOverlay(Ref<Layer> overlay)
    {
        auto it = std::find(m_Layers.begin(), m_Layers.end(), overlay);
        if (it != m_Layers.end()) m_Layers.erase(it);

        overlay->OnDetach();
        return overlay;
    }
    Ref<Layer> LayerStack::PopLayer(Ref<Layer> layer)
    {
        auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
        if (it != m_Layers.end())
        {
            m_Layers.erase(it);
            m_InsertIndex--;
        }

        layer->OnDetach();
        return layer;
    }
}; // namespace Vortex

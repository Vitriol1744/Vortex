//
// Created by Vitriol1744 on 20.08.2021.
//
#include "LayerStack.hpp"

namespace Vortex::Graphics
{
    LayerStack::~LayerStack()
    {
        for (auto layer : layers)
        {
            layer->OnDetach();
            delete layer;
        }
    }

    void LayerStack::PushOverlay(Layer* overlay)
    {
        layers.emplace_back(overlay);
    }
    void LayerStack::PushLayer(Layer* layer)
    {
        layers.emplace(layers.begin() + layerInsertIndex, layer);
        layerInsertIndex++;
    }

    void LayerStack::PopOverlay(Layer* overlay)
    {
        auto it = std::find(layers.begin() + layerInsertIndex, layers.end(), overlay);
        if (it != layers.end())
        {
            overlay->OnDetach();
            layers.erase(it);
        }
    }
    void LayerStack::PopLayer(Layer* layer)
    {
        auto it = std::find(layers.begin(), layers.begin() + layerInsertIndex, layer);
        if (it != layers.begin() + layerInsertIndex)
        {
            layer->OnDetach();
            layers.erase(it);
            layerInsertIndex--;
        }
    }
}

//
// Created by Vitriol1744 on 20.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Graphics/Layer.hpp"

#include <vector>

namespace Vortex::Graphics
{
    class VT_API LayerStack
    {
        public:
            LayerStack() = default;
            ~LayerStack();

            void PushOverlay(Layer* overlay);
            void PushLayer(Layer* layer);

            void PopOverlay(Layer* overlay);
            void PopLayer(Layer* layer);

            inline std::vector<Layer*>::iterator begin() { return layers.begin(); }
            inline std::vector<Layer*>::iterator end() { return layers.end(); }
            inline std::vector<Layer*>::reverse_iterator rbegin() { return layers.rbegin(); }
            inline std::vector<Layer*>::reverse_iterator rend() { return layers.rend(); }

            inline std::vector<Layer*>::const_iterator begin() const { return layers.begin(); }
            inline std::vector<Layer*>::const_iterator end()	const { return layers.end(); }
            inline std::vector<Layer*>::const_reverse_iterator rbegin() const { return layers.rbegin(); }
            inline std::vector<Layer*>::const_reverse_iterator rend() const { return layers.rend(); }

        private:
            std::vector<Layer*> layers;
            uint32 layerInsertIndex = 0;
    };
}
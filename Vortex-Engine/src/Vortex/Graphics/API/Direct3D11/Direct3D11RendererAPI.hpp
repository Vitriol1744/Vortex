//
// Created by Vitriol1744 on 18.09.2022.
//
#ifndef VORTEX_DIRECT3D11RENDERERAPI_HPP
#define VORTEX_DIRECT3D11RENDERERAPI_HPP

#include "Vortex/Graphics/API/IRendererAPI.hpp"

#include <d3d11.h>

namespace Vortex::Graphics
{
    class Direct3D11RendererAPI : public IRendererAPI
    {
        public:
            virtual void InitializeImpl() override;

        private:
            ID3D11Device* device = nullptr;
            ID3D11DeviceContext* deviceContext = nullptr;
    };
}
#endif //VORTEX_DIRECT3D11RENDERERAPI_HPP

//
// Created by Vitriol1744 on 18.09.2022.
//
#include "Direct3D11RendererAPI.hpp"

namespace Vortex::Graphics
{

    void Direct3D11RendererAPI::InitializeImpl()
    {
        HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &device, nullptr, &deviceContext);


    }
}
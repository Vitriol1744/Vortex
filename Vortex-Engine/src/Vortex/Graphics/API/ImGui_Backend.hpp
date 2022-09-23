//
// Created by Vitriol1744 on 21.09.2022.
//
#ifndef VORTEX_IMGUI_BACKEND_HPP
#define VORTEX_IMGUI_BACKEND_HPP

#include "Vortex/Core/Core.hpp"

#include "Vortex/Graphics/Window/IWindow.hpp"

namespace Vortex::Graphics::ImGui
{
    void Initialize(Ref<IWindow> mainWindow);
    void Shutdown();

    void Begin();
    void End();
}

#endif //VORTEX_IMGUI_BACKEND_HPP

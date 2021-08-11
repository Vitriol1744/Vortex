//
// Created by Vitriol1744 on 23.06.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Window/IWindow.hpp"

#include <utility>
#include <vector>

namespace Vortex
{
    class VT_API WindowManager : public Singleton<WindowManager>
    {
        public:
            WindowManager() = default;
            ~WindowManager() = default;

            inline void Bind(Ref<IWindow> window) { boundWindow = window; boundWindow->ActivateContext(); }
            VT_NODISCARD inline Ref<IWindow> GetWindow() const { return boundWindow; }
            
            Ref<IWindow> NewWindow(int32 width, int32 height, std::string_view title, Ref<IWindow> share = nullptr);

            VT_NODISCARD inline Ref<IWindow> GetBound() const { return boundWindow; }

        private:
            Ref<IWindow> boundWindow = nullptr;
            std::vector<Ref<IWindow>> windows;
    };

    template class VT_API Singleton<WindowManager>;
}



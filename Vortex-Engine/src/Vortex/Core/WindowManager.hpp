//
// Created by Vitriol1744 on 23.06.2021.
//
#pragma once

#include "Core.hpp"

#include "Window/IWindow.hpp"

#include <utility>
#include <vector>

namespace Vortex
{
    class VT_API WindowManager : public Singleton<WindowManager>
    {
        public:
            WindowManager() = default;
            ~WindowManager() = default;

            inline void Bind(Ref<IWindow> window) { boundWindow = std::move(window); boundWindow->ActivateContext(); }
            [[nodiscard]] inline Ref<IWindow> GetWindow() const { return boundWindow; }

            Ref<IWindow> NewWindow(int32 width, int32 height, std::wstring_view title);

        private:
            Ref<IWindow> boundWindow = nullptr;
            std::vector<Ref<IWindow>> windows;
    };
}



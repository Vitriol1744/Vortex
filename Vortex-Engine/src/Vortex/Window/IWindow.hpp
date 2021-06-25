//
// Created by Vitriol1744 on 22.06.2021.
//
#pragma once

#include "Core/Core.hpp"

#include <string>

#undef CreateWindow

namespace Vortex
{
    class VT_API IWindow
    {
        public:
            friend class WindowManager;

            IWindow() = default;
            virtual ~IWindow() = default;

            virtual void Update() = 0;
            virtual void SwapBuffers() = 0;

            virtual bool IsOpen() const = 0;

            virtual void SetTitle(std::wstring_view title) = 0;
            virtual void ActivateContext() = 0;

            [[nodiscard]] static Ref<IWindow> CreateWindow(int width, int height, std::wstring_view title);
    };
}



//
// Created by Vitriol1744 on 29.06.2021.
//

#pragma once

#include "Window/IWindow.hpp"
#include "Core/Core.hpp"
#include "Graphics/IGraphicsContext.hpp"

#include <string>
#include <X11/Xlib.h>

namespace Vortex
{
    class VT_API WindowImpl : public IWindow
    {
        public:
            friend class GL46Context;

            WindowImpl(int32 width, int32 height, std::wstring_view title);
            ~WindowImpl() override;

            void Update() override;
            void Present() override;

            VT_NODISCARD inline bool IsOpen() const override { return isOpen; }

            void SetTitle(std::wstring_view title) override;
            void ActivateContext() override;

        private:
            static uint32 windowsCount;
            static Display* display;

            bool isOpen = false;
            Window window;
            Scope<IGraphicsContext> context;

            static void Initialize();
            static void Shutdown();
    };
}



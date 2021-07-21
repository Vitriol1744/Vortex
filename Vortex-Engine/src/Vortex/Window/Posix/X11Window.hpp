//
// Created by Vitriol1744 on 29.06.2021.
//

#pragma once

#include "Window/IWindow.hpp"
#include "Core/Core.hpp"
#include "Graphics/API/IGraphicsContext.hpp"

#include <string>
#include <unordered_map>
#include <X11/Xlib.h>

namespace Vortex
{
    class VT_API WindowImpl final : public IWindow
    {
        public:
            WindowImpl(int32 width, int32 height, std::wstring_view title, Ref<IWindow> share = nullptr);
            virtual ~WindowImpl() override;

            virtual void Update() override; // Polls Events
            virtual void Present() override; // Presents Backbuffer to the Screen

            virtual inline Scope<Graphics::IGraphicsContext>& GetGraphicsContext() override { return context; }
            inline static Display* GetDisplay() { return display; }

            VT_NODISCARD virtual inline bool IsOpen() const noexcept override { return isOpen; }
            VT_NODISCARD virtual inline bool IsKeyPressed(Input::KeyCode keycode) const noexcept override { return keys[static_cast<uint32>(keycode)]; }
            VT_NODISCARD virtual inline bool IsMouseButtonPressed(Input::MouseCode mousecode) const noexcept override { return buttons[static_cast<uint32>(mousecode)]; }

            virtual void HideCursor() const noexcept override;
            virtual void SetTitle(std::string_view title) const noexcept override;
            virtual void SetTitle(std::wstring_view title) const noexcept override;
            virtual void ActivateContext() const override;

        private:
            static uint32 windowsCount;
            static Display* display;
            static XIM inputMethod;

            bool isOpen = true;
            Window window;
            XIC inputContext;
            Scope<Graphics::IGraphicsContext> context;

            bool keys[static_cast<uint32>(Input::KeyCode::KeysCount)];
            bool buttons[static_cast<uint32>(Input::MouseCode::ButtonsCount)];

            static void Initialize();
            static void Shutdown();

            inline static std::unordered_map<Window, WindowImpl*>* GetWindowsMap()
            {
                static auto windowsMap = new std::unordered_map<Window, WindowImpl*>();

                return windowsMap;
            }

            static void HandleEvent(XEvent event);
    };
}



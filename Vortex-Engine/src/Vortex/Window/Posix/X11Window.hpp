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
    struct WindowData
    {
        uint32 width;
        uint32 height;
        std::wstring title;
        Math::Vec2u position;
        bool isOpen = true;
        Graphics::IGraphicsContext* graphicsContext;
    };
    
    class VT_API WindowImpl final : public IWindow
    {
        public:
            WindowImpl(int32 width, int32 height, std::wstring_view title, Ref<IWindow> share = nullptr);
            virtual ~WindowImpl() override;

            virtual void Update() override; // Polls Events
            virtual void Present() override; // Presents BackBuffer to the Screen

            virtual inline Graphics::IGraphicsContext* GetGraphicsContext() override { return data.graphicsContext; }
            inline static Display* Display() { return display; }
        
            VT_NODISCARD virtual inline bool IsKeyPressed(Input::KeyCode keycode) const noexcept override { return keys[static_cast<uint32>(keycode)]; }
            VT_NODISCARD virtual inline bool IsMouseButtonPressed(Input::MouseCode mousecode) const noexcept override { return buttons[static_cast<uint32>(mousecode)]; }
            
            VT_NODISCARD virtual inline bool IsOpen() const noexcept override { return data.isOpen; }
            VT_NODISCARD virtual inline Math::Vec2u Position() const noexcept override { return data.position; }
            
            virtual void ShowCursor() const noexcept override;
            virtual void HideCursor() const noexcept override;
            virtual void SetFullscreen(bool fullscreen) const noexcept override;
            virtual void SetIcon(std::string_view path) const noexcept override;
            virtual void SetTitle(std::string_view title) const noexcept override;
            virtual void SetTitle(std::wstring_view title) const noexcept override;
            virtual void SetPosition(uint32 x, uint32 y) const override;
            virtual void SetVisible(bool visible) const noexcept override;

            virtual void ActivateContext() const override;

        private:
            static uint32 windowsCount;
            static ::Display* display;
            static XIM inputMethod;
            static Cursor blankCursor;

            Window window;
            XIC inputContext;
            WindowData data;

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



//
// Created by Vitriol1744 on 22.06.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"
#include "Vortex/Core/IEvent.hpp"

#include "Vortex/Input/KeyCode.hpp"
#include "Vortex/Input/MouseCode.hpp"

#include "Vortex/Graphics/API/IGraphicsContext.hpp"

#include "Vortex/Utility/TextureLoader.hpp"

#include <string>

#undef CreateWindow

namespace Vortex
{
    class VT_API IWindow : public NonCopyable<IWindow>
    {
        public:
            friend class WindowManager;

            IWindow() = default;
            virtual ~IWindow() = default;

            virtual void Update() = 0;
            virtual void Present() = 0;

            virtual Graphics::IGraphicsContext* GetGraphicsContext() = 0;
        
            VT_NODISCARD virtual bool IsKeyPressed(Input::KeyCode keycode) const noexcept = 0;
            VT_NODISCARD virtual bool IsMouseButtonPressed(Input::MouseCode mousecode) const noexcept = 0;
            VT_NODISCARD virtual Math::Vec2 MousePosition() const noexcept = 0;
            
            VT_NODISCARD virtual bool Focus() const noexcept = 0;
            VT_NODISCARD virtual bool IsOpen() const noexcept { return data.isOpen; }
            VT_NODISCARD virtual Math::Vec2u Position() const noexcept { return data.position; }
            VT_NODISCARD virtual Math::Vec2u Size() const noexcept { return Math::Vec2u(data.width, data.height); }

            virtual void ShowCursor() const noexcept = 0;
            virtual void HideCursor() const noexcept = 0;
            virtual void RequestFocus() const = 0;
            virtual void SetFullscreen(bool fullscreen) = 0;
            virtual void SetIcon(std::string_view path, int32 width, int32 height) const = 0;
            //TODO: Icon Loading:
            //virtual void SetIcon(const Utility::Pixel* pixels, int32 width, int32 height) const = 0;
            virtual void SetTitle(std::string_view title) const noexcept = 0;
            virtual void SetTitle(std::wstring_view title) const noexcept = 0;
            virtual void SetPosition(Math::Vec2u position) const { SetPosition(position.x, position.y); }
            virtual void SetPosition(uint32 x, uint32 y) const = 0;
            virtual void SetResizable(bool resizable) = 0;
            virtual void SetSize(Math::Vec2u size) { SetSize(size.x, size.y); }
            virtual void SetSize(uint32 width, uint32 height) = 0;
            virtual void SetWidth(uint32 width) { SetSize(width, data.height); }
            virtual void SetHeight(uint32 height) { SetSize(data.width, height); }
            virtual void SetVisible(bool visible) const = 0;

            VT_NODISCARD static Ref<IWindow> CreateWindow(uint32 width, uint32 height, std::wstring_view title, Ref<IWindow> share = nullptr);
            VT_NODISCARD static Ref<IWindow> CreateWindow(uint32 width, uint32 height, uint32 bitsPerPixel, std::wstring_view title, Ref<IWindow> share = nullptr);

            enum WindowResizedEventType
            {
                Restored = 0,
                Minimized = 1,
                Maximized = 2
            };

            // Window Events
            // Vortex::input::KeyCode keycode, uint32 repeatCount
            Event<Input::KeyCode, uint32> keyPressedEvent;
            // Vortex::Input::KeyCode keycode
            Event<Input::KeyCode> keyReleasedEvent;
            // uint32 character
            Event<uint32> keyTypedEvent;
            // Vortex::Input::MouseCode mouseCode, bool doubleclick
            Event<Input::MouseCode, bool> mouseButtonPressedEvent;
            // Vortex::Input::MouseCode mouseCode
            Event<Input::MouseCode> mouseButtonReleasedEvent;
            // Vortex::Math::Vec2 wheel
            Event<Math::Vec2> mouseScrolledEvent;
            // Vortex::Math::Vec2 mousePosition
            Event<Math::Vec2> mouseMovedEvent;
            // Vortex::IWindow::WindowResizedEventType windowResizedEventType, Vortex::Math::Vec2u windowSize;
            Event<WindowResizedEventType, Math::Vec2u> windowResizedEvent;
            // bool focus
            Event<bool> focusChangedEvent;

        protected:
            struct WindowData
            {
                uint32 width = 800;
                uint32 height = 600;
                uint16 bitsPerPixel = 32;
                std::wstring title = L"Vortex";
                Math::Vec2u position = { 0, 0 };
                Math::Vec2 mousePosition = { 0, 0 };
                bool isOpen = true;
                bool fullscreen = false;
                bool resizable = false;
                bool visible = true;
                Graphics::IGraphicsContext* graphicsContext = nullptr;
            };

            mutable WindowData data;

        private:
            virtual void ActivateContext() const = 0;
    };
}



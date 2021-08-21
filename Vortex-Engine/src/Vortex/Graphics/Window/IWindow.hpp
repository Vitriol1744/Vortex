//
// Created by Vitriol1744 on 22.06.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"
#include "Vortex/Core/Event.hpp"

#include "Vortex/Core/Input/KeyCode.hpp"
#include "Vortex/Core/Input/MouseCode.hpp"

#include "Vortex/Graphics/API/IGraphicsContext.hpp"

#include "Vortex/Utility/TextureLoader.hpp"

#include <string>

#undef CreateWindow

namespace Vortex
{
    enum class WindowStyle : uint8
    {
        None,
        Titlebar,
        MaximizeButton,
        MinimizeButton,
        CloseButton
    };
    inline WindowStyle operator|(const WindowStyle& left, const WindowStyle& right)
    {
        return static_cast<WindowStyle>(static_cast<uint8>(left) | static_cast<uint8>(right));
    }
    inline uint8 operator&(const WindowStyle& left, const WindowStyle& right)
    {
        return static_cast<uint8>(left) & static_cast<uint8>(right);
    }

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
            VT_NODISCARD virtual std::string Title() const noexcept { return data.title; }
            VT_NODISCARD virtual bool IsOpen() const noexcept { return data.isOpen; }
            VT_NODISCARD virtual bool IsHovered() const noexcept { return data.mouseHovered; }
            VT_NODISCARD virtual Math::Vec2u Position() const noexcept { return data.position; }
            VT_NODISCARD virtual Math::Vec2u Size() const noexcept { return Math::Vec2u(data.width, data.height); }
            VT_NODISCARD virtual uint32 Width() const noexcept { return data.width; }
            VT_NODISCARD virtual uint32 Height() const noexcept { return data.height; }

            virtual void ShowCursor() const noexcept = 0;
            virtual void HideCursor() const noexcept = 0;
            virtual void RequestFocus() const = 0;
            virtual void SetFullscreen(bool fullscreen) = 0;
            virtual void SetIcon(std::string_view path, int32 width, int32 height) const = 0;
            //TODO: Icon Loading:
            //virtual void SetIcon(const Utility::Pixel* pixels, int32 width, int32 height) const = 0;
            virtual void SetTitle(std::string_view title) const noexcept = 0;
            virtual void SetPosition(Math::Vec2u position) const { SetPosition(position.x, position.y); }
            virtual void SetPosition(uint32 x, uint32 y) const = 0;
            virtual void SetResizable(bool resizable) = 0;
            virtual void SetSize(Math::Vec2u size) { SetSize(size.x, size.y); }
            virtual void SetSize(uint32 width, uint32 height) = 0;
            //TODO: Windows: Window: Implement SetStyle function in windows implementation and make it pure virtual!
            virtual void SetStyle(WindowStyle style) { };
            virtual void SetWidth(uint32 width) { SetSize(width, data.height); }
            virtual void SetHeight(uint32 height) { SetSize(data.width, height); }
            virtual void SetVisible(bool visible) const = 0;

            VT_NODISCARD static Ref<IWindow> CreateWindow(uint32 width, uint32 height, std::string_view title, Ref<IWindow> share = nullptr);
            VT_NODISCARD static Ref<IWindow> CreateWindow(uint32 width, uint32 height, uint32 bitsPerPixel, std::string_view title, Ref<IWindow> share = nullptr);
            
            // Window Events
            // Vortex::input::KeyCode keycode, uint32 repeatCount
            Event<IWindow*, Input::KeyCode, uint32> keyPressedEvent;
            // Vortex::Input::KeyCode keycode
            Event<IWindow*, Input::KeyCode> keyReleasedEvent;
            // uint32 character
            Event<IWindow*, uint32> keyTypedEvent;
            // Vortex::Input::MouseCode mouseCode, bool doubleclick
            Event<IWindow*, Input::MouseCode, bool> mouseButtonPressedEvent;
            // Vortex::Input::MouseCode mouseCode
            Event<IWindow*, Input::MouseCode> mouseButtonReleasedEvent;
            // Vortex::Math::Vec2 wheel
            Event<IWindow*, Math::Vec2> mouseScrolledEvent;
            // Vortex::Math::Vec2 mousePosition
            Event<IWindow*, Math::Vec2> mouseMovedEvent;
            //
            Event<IWindow*> windowClosedEvent;
            //
            Event<IWindow*, Math::Vec2> windowMovedEvent;
            // Vortex::IWindow::WindowResizedEventType windowResizedEventType, Vortex::Math::Vec2u windowSize;
            Event<IWindow*, Math::Vec2u> windowResizedEvent;
            // bool focus
            Event<IWindow*, bool> focusChangedEvent;

        protected:
            struct WindowData
            {
                uint32 width = 800;
                uint32 height = 600;
                uint16 bitsPerPixel = 32;
                std::string title = "Vortex";
                Math::Vec2u position = { 0, 0 };
                Math::Vec2 mousePosition = { 0, 0 };
                bool isOpen = true;
                bool fullscreen = false;
                bool resizable = true;
                bool visible = true;
                bool mouseHovered = false;
                Graphics::IGraphicsContext* graphicsContext = nullptr;
                Graphics::IGraphicsContext* sharedContext = nullptr;
            };

            mutable WindowData data;

        public:
            virtual void ActivateContext() const = 0;
    };
}



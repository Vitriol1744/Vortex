//
// Created by Vitriol1744 on 29.06.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Graphics/API/IGraphicsContext.hpp"
#include "Graphics/Window/IWindow.hpp"

#include "Vortex/Platform/X11/X11.hpp"

#include <string>
#include <unordered_map>
#undef CursorShape

namespace Vortex
{
    class VT_API WindowImpl final : public IWindow
    {
        public:
            WindowImpl(int32 width, int32 height, uint32 bitsPerPixel, std::string_view title, Ref<IWindow> share = nullptr);
            virtual ~WindowImpl() override;

            static void PollEvents();
            virtual void Present() override; // Presents BackBuffer to the Screen

            virtual inline Graphics::IGraphicsContext* GetGraphicsContext() override { return data.graphicsContext; }
            inline static Display* GetDisplay() { return display; }
        
            VT_NODISCARD virtual inline bool IsKeyPressed(Input::KeyCode keycode) const noexcept override { return keys[static_cast<uint32>(keycode)]; }
            VT_NODISCARD virtual inline bool IsMouseButtonPressed(Input::MouseCode mousecode) const noexcept override { return buttons[static_cast<uint32>(mousecode)]; }
            VT_NODISCARD virtual inline Math::Vec2 GetMousePosition() const noexcept override { return data.mousePosition; }

            VT_NODISCARD virtual NativeWindowHandleType GetNativeWindowHandle() const noexcept override { return window; }
            VT_NODISCARD virtual inline bool IsFocused() const noexcept override
            {
                Window focused;
                int revert;

                XGetInputFocus(display, &focused, &revert);
                return focused == window;
            }
            VT_NODISCARD virtual inline bool IsOpen() const noexcept override { return data.isOpen; }
            VT_NODISCARD virtual inline Math::Vec2u GetPosition() const noexcept override { return data.position; }
            VT_NODISCARD virtual inline Math::Vec2u GetSize() const noexcept override { return { data.width, data.height }; }

            virtual void ShowCursor() const noexcept override;
            virtual void HideCursor() const noexcept override;
            virtual void RequestFocus() const override;
            virtual void SetCursor(CursorShape shape) override { }
            virtual void SetCursor(const Ref<Utility::Pixel> pixels, int32 width, int32 height) override { }
            virtual void SetFullscreen(bool fullscreen) override;
            virtual void SetIcon(std::string_view path, int32 width, int32 height) noexcept override;
            //TODO: Linux: SetIcon
            virtual void SetIcon(const Utility::Pixel* pixels, int32 width, int32 height) override { }
            virtual void SetTitle(std::string_view title) const noexcept override;
            virtual void SetPosition(uint32 x, uint32 y) const override;
            virtual void SetResizable(bool resizable) override;
            virtual void SetSize(uint32 width, uint32 height) override;
            virtual void SetStyle(WindowStyle style) override;
            virtual void SetVisible(bool visible) const noexcept override;

            virtual void ActivateContext() const override;

        private:
            static uint32 windowsCount;
            static ::Display* display;
            static XIM inputMethod;
            static Cursor blankCursor;

            Window rootWindow;
            Window window;
            Screen* screen;
            XIC inputContext;
            ::Time lastUserActivityTime;

            bool keys[static_cast<uint32>(Input::KeyCode::KeysCount)];
            bool buttons[static_cast<uint32>(Input::MouseCode::ButtonsCount)];

            void SetWMUserTime(::Time time);
            void Recreate(int32 width, int32 height, int8 bitsPerPixel, strview title, Graphics::IGraphicsContext* sharedContext);
            void Create(int32 width, int32 height, int8 bitsPerPixel, strview title, Graphics::IGraphicsContext* sharedContext);
            void CreateGraphicsContext(uint8 bitsPerPixel, Graphics::IGraphicsContext* sharedContext);
            void PostInitialize();
            void Destroy();

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



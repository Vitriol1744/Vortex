//
// Created by Vitriol1744 on 07.09.2022.
//
#ifndef VORTEX_IWINDOW_HPP
#define VORTEX_IWINDOW_HPP

#include "Vortex/Core/Core.hpp"

#include "Vortex/Core/Input/KeyCode.hpp"
#include "Vortex/Core/Input/MouseCode.hpp"

#include "Vortex/Core/Math/Math.hpp"

#include "Vortex/Graphics/API/IGraphicsContext.hpp"

#undef CreateWindow

namespace Vortex
{
    using NativeWindowHandle = uintptr_t;

    struct VideoMode
    {
        VideoMode() : width(800), height(600), bitsPerPixel(32) { }

        uint32 width;
        uint32 height;
        uint8 bitsPerPixel;
    };
    class Monitor
    {
        public:
            struct GammaRamp
            {
                uint16 red;
                uint16 green;
                uint16 blue;
                uint32 size;
            };

            inline Math::Vec2i GetVirtualPosition() const noexcept { return virtualPosition; }
            inline const Math::Rectangle& GetWorkArea() const noexcept { return workArea; }
            inline Math::Vec2u GetPhysicalSize() const noexcept { return physicalSize; }
            inline Math::Vec2f GetContentScale() const noexcept { return contentScale; }
            inline std::string_view GetName() const noexcept { return name; }
            inline const std::vector<VideoMode>& GetVideoModes() const noexcept { return fullscreenVideoModes; }
            inline const VideoMode& GetCurrentVideoMode() const noexcept { return currentVideoMode; }
            inline const GammaRamp& GetGammaRamp() const noexcept { return gammaRamp; }
            //TODO: Monitor: void SetGammaRamp();

            //TODO: static Monitor* GetPrimaryMonitor();
            //TODO: static std::vector<Monitor>& GetMonitors();
        private:
            uint32 id;
            std::string name;
            VideoMode currentVideoMode;
            std::vector<VideoMode> fullscreenVideoModes;
            Math::Vec2u physicalSize;
            Math::Vec2f contentScale;
            Math::Vec2i virtualPosition;
            Math::Rectangle workArea;
            GammaRamp gammaRamp;
    };
    enum class WindowStyle : uint8
    {
        eNone            = 0,
        eTitlebar        = VT_BIT(0),
        eMaximizeButton  = VT_BIT(1),
        eMinimizeButton  = VT_BIT(2),
    };
    inline WindowStyle operator|(const WindowStyle& left, const WindowStyle& right)
    {
        return static_cast<WindowStyle>(static_cast<uint8>(left) | static_cast<uint8>(right));
    }
    inline uint8 operator&(const WindowStyle& left, const WindowStyle& right)
    {
        return static_cast<uint8>(left) & static_cast<uint8>(right);
    }

    enum class CursorShape
    {
        eNone,
        eArrow,
        eIBeam,
        eCrosshair,
        eGrabbed,
        eHResize,
        eVResize,
    };

    class VT_API IWindow : public NonCopyable<IWindow>
    {
        public:
            IWindow() = default;
            virtual ~IWindow() = default;

            static void PollEvents();
            virtual void Present() = 0;

            virtual void ActivateContext() const = 0;
            inline Graphics::IGraphicsContext* GetGraphicsContext() { return data.graphicsContext; };

            VT_NODISCARD virtual bool IsKeyPressed(Input::KeyCode keycode) const noexcept = 0;
            VT_NODISCARD virtual bool IsMouseButtonPressed(Input::MouseCode mouse_code) const noexcept = 0;
            VT_NODISCARD virtual Math::Vec2 GetMousePosition() const noexcept = 0;

            VT_NODISCARD virtual NativeWindowHandle GetNativeHandle() const noexcept = 0;
            VT_NODISCARD virtual bool IsFocused() const noexcept = 0;
            VT_NODISCARD virtual std::string GetTitle() const noexcept { return data.title; }
            VT_NODISCARD virtual bool IsOpen() const noexcept { return data.isOpen; }
            VT_NODISCARD virtual bool IsMinimized() const noexcept { return data.minimized; }
            VT_NODISCARD virtual bool IsHovered() const noexcept { return data.mouseHovered; }
            VT_NODISCARD virtual Math::Vec2u GetPosition() const noexcept { return data.position; }
            VT_NODISCARD virtual Math::Vec2u GetSize() const noexcept { return Math::Vec2u(data.width, data.height); }
            VT_NODISCARD virtual uint32 GetWidth() const noexcept { return data.width; }
            VT_NODISCARD virtual uint32 GetHeight() const noexcept { return data.height; }

            virtual void ShowCursor() const noexcept = 0;
            virtual void HideCursor() const noexcept = 0;
            virtual void RequestFocus() const = 0;
            virtual void SetCursor(CursorShape shape) = 0;
            virtual void SetCursor(const Ref<Pixel> pixels, int32 width, int32 height) = 0;
            virtual void SetFullscreen(bool fullscreen) = 0;
            virtual void Maximize() = 0;
            virtual void Minimize() = 0;
            virtual void SetIcon(std::string_view path, int32 width, int32 height) = 0;
            virtual void SetIcon(const Pixel* pixels, int32 width, int32 height) = 0;
            virtual void SetTitle(std::string_view title) const noexcept = 0;
            virtual void SetPosition(Math::Vec2u position) const { SetPosition(position.x, position.y); }
            virtual void SetPosition(uint32 x, uint32 y) const = 0;
            virtual void SetResizable(bool resizable) = 0;
            virtual void SetSize(Math::Vec2u size) { SetSize(size.x, size.y); }
            virtual void SetSize(uint32 width, uint32 height) = 0;
            virtual void SetStyle(WindowStyle style) = 0;
            virtual void SetWidth(uint32 width) { SetSize(width, data.height); }
            virtual void SetHeight(uint32 height) { SetSize(data.width, height); }
            virtual void SetVisible(bool visible) const = 0;
            virtual void SetTopMost(bool topmost) = 0;

            VT_NODISCARD static Ref<IWindow> CreateWindow(uint32 width, uint32 height, std::string title);
            VT_NODISCARD static Ref<IWindow> CreateWindow(uint32 width, uint32 height, uint32 bpp, std::string title);

        protected:
            struct WindowData
            {
                IWindow* window;
                uint32 width = 800;
                uint32 height = 600;
                uint16 bitsPerPixel = 32;
                std::string title = "Vortex";
                Math::Vec2i position = { 0, 0 };
                Math::Vec2d mousePosition = { 0.0, 0.0 };
                Math::Vec2d mouseScroll = { 0.0, 0.0 };
                Math::Vec2i windowModeSize = { 800, 600 };
                Math::Vec2i windowModePosition = { 100, 100 };
                bool isOpen = true;
                bool fullscreen = false;
                bool resizable = true;
                bool visible = true;
                bool focused = false;
                bool mouseHovered = false;
                bool minimized = false;
                Graphics::IGraphicsContext* graphicsContext = nullptr;
                bool keys[static_cast<uint32>(Input::KeyCode::KeysCount)];
                bool mouseButtons[static_cast<uint32>(Input::MouseCode::ButtonsCount)];
            };

            mutable WindowData data;
    };

    enum class MonitorState
    {
        eOn,
        eOff,
        eLowPower,
        eScreenSaver
    };

    template class VT_API NonCopyable<IWindow>;
}


#endif //VORTEX_IWINDOW_HPP

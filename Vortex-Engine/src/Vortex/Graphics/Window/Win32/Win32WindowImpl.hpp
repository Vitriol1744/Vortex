//
// Created by Vitriol1744 on 07.09.2022.
//
#ifndef VORTEX_WIN32WINDOWIMPL_HPP
#define VORTEX_WIN32WINDOWIMPL_HPP

#include "Vortex/Graphics/Window/IWindow.hpp"

#include <Windows.h>
#include <GLFW/glfw3.h>

namespace Vortex
{
    class VT_API WindowImpl : public IWindow
    {
        public:
            WindowImpl(uint32 width, uint32 height, uint32 bitsPerPixel, std::string_view title);
            ~WindowImpl() override;

            static void PollEvents();
            virtual void Present() override;

            virtual inline void ActivateContext() const override { data.graphicsContext->Activate(); }

            VT_NODISCARD virtual inline bool IsKeyPressed(Input::KeyCode keycode) const noexcept override { return data.keys[static_cast<uint32>(keycode)]; }
            VT_NODISCARD virtual inline bool IsMouseButtonPressed(Input::MouseCode mouse_code) const noexcept override { return data.mouseButtons[static_cast<uint32>(mouse_code)]; }
            VT_NODISCARD virtual inline Math::Vec2 GetMousePosition() const noexcept override { return data.mousePosition; }

            //VT_NODISCARD virtual inline NativeWindowHandle GetNativeHandle() const noexcept override { return reinterpret_cast<uintptr_t>(glfwGetWin32Window(window)); }
            VT_NODISCARD virtual inline NativeWindowHandle GetNativeHandle() const noexcept override { return reinterpret_cast<uintptr_t>(window); }
            VT_NODISCARD virtual inline bool IsFocused() const noexcept override { return data.focused; }
            VT_NODISCARD virtual inline bool IsOpen() const noexcept override { return data.isOpen; }
            VT_NODISCARD virtual inline Math::Vec2u GetPosition() const noexcept override { return data.position; }

            virtual void ShowCursor() const noexcept override;
            virtual void HideCursor() const noexcept override;
            virtual void RequestFocus() const override;
            virtual void SetCursor(CursorShape shape) override;
            virtual void SetCursor(const Ref<Pixel> pixels, int32 width, int32 height) override;
            virtual void SetFullscreen(bool fullscreen) override;
            virtual void Maximize() override;
            virtual void Minimize() override;
            virtual void SetIcon(std::string_view path, int32 width, int32 height) override;
            virtual void SetIcon(const Pixel* pixels, int32 width, int32 height) override;
            virtual void SetTitle(std::string_view title) const noexcept override;
            virtual void SetPosition(uint32 x, uint32 y) const override;
            virtual void SetResizable(bool resizable) override;
            virtual void SetSize(uint32 width, uint32 height) override;
            virtual void SetStyle(WindowStyle style) override;
            virtual void SetVisible(bool visible) const override;
            virtual void SetTopMost(bool topmost) override;


        private:
            static uint32 windowsCount;

            GLFWwindow* window;
            GLFWmonitor* monitor;

            static void Initialize();
            static void Shutdown();
    };
}


#endif //VORTEX_WIN32WINDOWIMPL_HPP

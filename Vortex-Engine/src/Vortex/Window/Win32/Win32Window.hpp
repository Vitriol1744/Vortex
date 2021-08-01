//
// Created by Vitriol1744 on 22.06.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"
#include "Vortex/Window/IWindow.hpp"

#include "Vortex/Graphics/API/IGraphicsContext.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <map>

namespace Vortex
{
    class VT_API WindowImpl : public IWindow
    {
        public:
            WindowImpl(uint32 width, uint32 height, uint32 bitsPerPixel, std::wstring_view title, Ref<IWindow> share = nullptr);
            ~WindowImpl() override;

            void Update() override;
            void Present() override;

            virtual inline Graphics::IGraphicsContext* GetGraphicsContext() override { return data.graphicsContext; }

            VT_NODISCARD virtual inline bool IsKeyPressed(Input::KeyCode keycode) const noexcept override { return keys[static_cast<uint32>(keycode)]; }
            VT_NODISCARD virtual inline bool IsMouseButtonPressed(Input::MouseCode mousecode) const noexcept override { return buttons[static_cast<uint32>(mousecode)]; }
            VT_NODISCARD virtual inline Math::Vec2 MousePosition() const noexcept override { return data.mousePosition; }

            VT_NODISCARD virtual inline bool Focus() const noexcept override { return hWnd == GetForegroundWindow(); }
            VT_NODISCARD virtual inline bool IsOpen() const noexcept override { return data.isOpen; }
            VT_NODISCARD virtual inline Math::Vec2u Position() const noexcept override { return data.position; }

            virtual void ShowCursor() const noexcept override;
            virtual void HideCursor() const noexcept override;
            virtual void RequestFocus() const override;
            virtual void SetFullscreen(bool fullscreen = true) override;
            virtual void SetIcon(std::string_view path, int32 width, int32 height) const override;
            virtual void SetTitle(std::string_view title) const noexcept override;
            virtual void SetTitle(std::wstring_view title) const noexcept override;
            virtual void SetPosition(uint32 x, uint32 y) const override;
            virtual void SetResizable(bool resizable) override;
            virtual void SetSize(uint32 width, uint32 height) override;
            virtual void SetVisible(bool visible = true) const override;

            virtual void ActivateContext() const override;

        private:
            static uint32 windowsCount;

            HWND hWnd;

            bool keys[static_cast<uint32>(Input::KeyCode::KeysCount)];
            bool buttons[static_cast<uint32>(Input::MouseCode::ButtonsCount)];

            static void Initialize();
            static void Shutdown();

            inline static std::unordered_map<HWND, WindowImpl*>* GetWindowsMap()
            {
                static auto windowsMap = new std::unordered_map<HWND, WindowImpl*>();

                return windowsMap;
            }

            static LRESULT WINAPI HandleGlobalEvents(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
            LRESULT WINAPI HandleEvents(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    };
}
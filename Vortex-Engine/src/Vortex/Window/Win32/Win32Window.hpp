//
// Created by Vitriol1744 on 22.06.2021.
//
#pragma once

#include "Window/IWindow.hpp"

#include "Graphics/API/IGraphicsContext.hpp"

#define UNICODE
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <Windows.h>

#include <map>

namespace Vortex
{
    class VT_API WindowImpl : public IWindow
    {
        public:
            WindowImpl(int32 width, int32 height, std::wstring_view title, Ref<IWindow> share = nullptr);
            ~WindowImpl() override;

            void Update() override;
            void Present() override;

            virtual inline Graphics::IGraphicsContext* GetGraphicsContext() override { return context; }

            VT_NODISCARD virtual inline bool IsKeyPressed(Input::KeyCode keycode) const noexcept override { return keys[static_cast<uint32>(keycode)]; }
            VT_NODISCARD virtual inline bool IsMouseButtonPressed(Input::MouseCode mousecode) const noexcept override { return buttons[static_cast<uint32>(mousecode)]; }

            VT_NODISCARD virtual inline bool IsOpen() const noexcept override { return isOpen; }
            VT_NODISCARD virtual inline Math::Vec2u Position() const noexcept { return Math::Vec2u(0, 0); }
            //TODO: Position^^^
            virtual void ShowCursor() const noexcept override;
            virtual void HideCursor() const noexcept override;
            virtual void SetFullscreen(bool fullscreen) const;
            virtual void SetIcon(std::string_view path) const;
            virtual void SetTitle(std::string_view title) const noexcept override;
            virtual void SetTitle(std::wstring_view title) const noexcept override;
            virtual void SetPosition(uint32 x, uint32 y) const;
            virtual void SetVisible(bool visible = true) const;
            
            virtual void ActivateContext() const override;

        private:
            static uint32 windowsCount;

            bool isOpen = true;
            HWND hWnd;
            Graphics::IGraphicsContext* context;

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
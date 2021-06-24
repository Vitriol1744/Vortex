//
// Created by Vitriol1744 on 22.06.2021.
//
#pragma once

#include "Window/IWindow.hpp"

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
            WindowImpl(int32 width, int32 height, std::wstring_view title);
            ~WindowImpl() override;

            void Update() override;

            [[maybe_unused]] [[nodiscard]] inline bool IsOpen() const override { return isOpen; }

            void SetTitle(std::wstring_view title) override;
            void ActivateContext() override;

        public:
            static uint32 windowsCount;
            static std::map<HWND, WindowImpl*> windows;

            bool isOpen = true;
            HWND hWnd;

            static void Initialize();
            static void Shutdown();

            static LRESULT WINAPI HandleGlobalEvents(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
            LRESULT WINAPI HandleEvents(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    };
}


//
// Created by Vitriol1744 on 22.06.2021.
//
#include "Core/Macros.hpp"

#ifdef VT_PLATFORM_WINDOWS
#include "Win32Window.hpp"

#include "Graphics/OpenGL46/GL46Context.hpp"

namespace
{
    const wchar_t* windowClassName = L"Window Class";
    HINSTANCE hInstance = nullptr;
}

namespace Vortex
{
    unsigned int WindowImpl::windowsCount = 0;
    std::map<HWND, WindowImpl*> WindowImpl::windows{};

    WindowImpl::WindowImpl(int32 width, int32 height, std::wstring_view title)
    {
        if (!windowsCount) Initialize();
        hWnd = CreateWindowExW
        (
            0, windowClassName, title.data(), WS_OVERLAPPEDWINDOW,
            0, 0, width, height, nullptr, nullptr, hInstance, nullptr
        );
        windows[hWnd] = this;

        context = CreateScope<GL46Context>();
        context->Initialize(hWnd);

        ShowWindow(hWnd, SW_SHOW);
        UpdateWindow(hWnd);
    }

    void WindowImpl::SwapBuffers()
    {
        context->SwapBuffers();
    }

    WindowImpl::~WindowImpl()
    {
        windowsCount--;
        if (!windowsCount) Shutdown();
    }

    void WindowImpl::Update()
    {
        MSG msg;
        while (PeekMessageW(&msg, hWnd, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void WindowImpl::SetTitle(std::wstring_view title)
    {
        SetWindowTextW(hWnd, title.data());
    }

    void WindowImpl::ActivateContext()
    {

    }

    void WindowImpl::Initialize()
    {
        hInstance = GetModuleHandle(nullptr);

        WNDCLASSEXW wcex{};

        wcex.cbSize = sizeof(wcex);
        wcex.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
        wcex.lpfnWndProc = HandleGlobalEvents;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hCursor = LoadCursor(hInstance, IDC_ARROW);
        wcex.hIcon = nullptr;
        wcex.hbrBackground = nullptr;
        wcex.hInstance = hInstance;
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = windowClassName;

        RegisterClassExW(&wcex);
    }

    void WindowImpl::Shutdown()
    {
        if (!UnregisterClassW(windowClassName, hInstance));
    }

    LRESULT WINAPI WindowImpl::HandleGlobalEvents(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        return windows[hWnd]->HandleEvents(hWnd, msg, wParam, lParam);
    }

    LRESULT WINAPI WindowImpl::HandleEvents(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
            case WM_CLOSE:
                isOpen = false;
                PostQuitMessage(0);
                break;
        }
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
}
#endif
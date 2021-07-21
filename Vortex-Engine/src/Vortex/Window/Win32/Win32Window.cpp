//
// Created by Vitriol1744 on 22.06.2021.
//

#include "vtpch.hpp"
#include "Core/Macros.hpp"

#ifdef VT_PLATFORM_WINDOWS
#define _UNICODE
#include "Win32Window.hpp"

#include "Graphics/API/OpenGL46/GL46Context.hpp"
#include "Graphics/API/IRendererAPI.hpp"

namespace
{
    const wchar_t* windowClassName = L"Window Class";
    HINSTANCE hInstance = nullptr;
}

namespace Vortex
{
    unsigned int WindowImpl::windowsCount = 0;

    WindowImpl::WindowImpl(int32 width, int32 height, std::wstring_view title, Ref<IWindow> share)
    {
        if (!windowsCount) Initialize();
        hWnd = CreateWindowExW
        (
            0, windowClassName, title.data(), WS_OVERLAPPEDWINDOW,
            0, 0, width, height, nullptr, nullptr, hInstance, nullptr
        );
        (*GetWindowsMap())[hWnd] = this;

        VT_CORE_LOG_TRACE("Window Created!");

        Graphics::IRendererAPI::Initialize();
        switch (Graphics::IRendererAPI::Get()->GetGraphicsAPI())
        {
            case Graphics::GraphicsAPI::OpenGL46:
            {
                context = CreateScope<Graphics::GL46Context>(reinterpret_cast<void*>(&window));
                break;
            }
            case Graphics::GraphicsAPI::None:

            default:
                VT_CORE_LOG_FATAL("Graphics API Not Supported!");
                break;
        }

        ShowWindow(hWnd, SW_SHOW);
        UpdateWindow(hWnd);
    }

    WindowImpl::~WindowImpl()
    {
        GetWindowsMap()->erase(hWnd);
        DestroyWindow(hWnd);
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

    void WindowImpl::Present()
    {
        context->Present();
    }

    void WindowImpl::HideCursor() const noexcept
    {

    }

    void WindowImpl::SetTitle(std::string_view title)
    {
        SetWindowTextA(hWnd, title.data());
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
        if (!windowsCount) return;
        UnregisterClassW(windowClassName, hInstance);
    }

    LRESULT WINAPI WindowImpl::HandleGlobalEvents(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        return (*GetWindowsMap())[hWnd]->HandleEvents(hWnd, msg, wParam, lParam);
    }

    LRESULT WINAPI WindowImpl::HandleEvents(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
            case WM_CLOSE:
            case WM_DESTROY:
                isOpen = false;
                PostQuitMessage(0);
                break;

            default:
                break;
        }
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}
#endif
//
// Created by Vitriol1744 on 22.06.2021.
//

#include "vtpch.hpp"
#include "Core/Macros.hpp"

#ifdef VT_PLATFORM_WINDOWS
//#define _UNICODE
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
    using namespace Vortex::Math;
    using namespace Vortex::Graphics;
    unsigned int WindowImpl::windowsCount = 0;

    WindowImpl::WindowImpl(int32 width, int32 height, std::wstring_view title, Ref<IWindow> share)
    {
        data.width = width;
        data.height = height;
        data.title = title;
        data.position = Vec2u(0, 0);
        if (!windowsCount) Initialize();
        hWnd = CreateWindowExW
        (
            0, windowClassName, data.title.data(), WS_OVERLAPPEDWINDOW,
            data.position.x, data.position.y, data.width, data.height, nullptr, nullptr, hInstance, nullptr
        );
        (*GetWindowsMap())[hWnd] = this;

        VT_CORE_LOG_TRACE("Window Created!");

        IRendererAPI::Initialize();
        switch (IRendererAPI::GetGraphicsAPI())
        {
            case GraphicsAPI::OpenGL46:
            {
                data.graphicsContext = new GL46Context(reinterpret_cast<void*>(hWnd), share ? share->GetGraphicsContext() : nullptr);
                break;
            }
            case GraphicsAPI::None:

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
        data.graphicsContext->Present();
    }

    void WindowImpl::ShowCursor() const noexcept
    {
        ::ShowCursor(true);
    }

    void WindowImpl::HideCursor() const noexcept
    {
        ::ShowCursor(false);
    }

    void WindowImpl::SetFullscreen(bool fullscreen) const
    {

    }

    void WindowImpl::SetIcon(std::string_view path, int32 width, int32 height) const
    {
        const std::wstring& name = std::wstring(path.begin(), path.end());
        auto icon = (LONG_PTR)LoadImageW(nullptr, name.c_str(), IMAGE_ICON, width, height, LR_LOADFROMFILE);

        SetClassLongPtrW(hWnd, GCLP_HICON, icon);
        SetClassLongPtrW(hWnd, GCLP_HICONSM, icon);
    }

    void WindowImpl::SetTitle(std::string_view title) const noexcept
    {
        SetWindowTextA(hWnd, title.data());
    }

    void WindowImpl::SetTitle(std::wstring_view title) const noexcept
    {
        SetWindowTextW(hWnd, title.data());
    }

    void WindowImpl::SetPosition(uint32 x, uint32 y) const
    {

    }

    void WindowImpl::SetVisible(bool visible) const
    {
        visible ? ShowWindow(hWnd, SW_SHOW) : ShowWindow(hWnd, SW_HIDE);
    }

    void WindowImpl::ActivateContext() const
    {
        data.graphicsContext->Activate();
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
                data.isOpen = false;
                PostQuitMessage(0);
                break;
            case WM_KEYUP:
            case WM_KEYDOWN:
                break;

            default:
                break;
        }
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}
#endif
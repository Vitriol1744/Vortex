//
// Created by Vitriol1744 on 29.06.2021.
//
#include "vtpch.hpp"
#include "Core/Macros.hpp"

#include "Graphics/OpenGL46/GL46Context.hpp"
#include "Graphics/IRendererAPI.hpp"

#ifdef VT_PLATFORM_LINUX
#include "PosixWindow.hpp"
namespace Vortex
{
    uint32   WindowImpl::windowsCount = 0;
    Display* WindowImpl::display = nullptr;

    WindowImpl::WindowImpl(int32 width, int32 height, std::wstring_view title)
    {
        if (windowsCount == 0) Initialize();

        window = XCreateSimpleWindow
        (
            display, DefaultRootWindow(display),
            10, 10, 800, 600, 0, 0, 0
        );
        windowsCount++;

        IRendererAPI::Initialize();
        switch (Vortex::IRendererAPI::Get()->GetGraphicsAPI())
        {
            case GraphicsAPI::OpenGL46:
                context = CreateScope<GL46Context>();
                break;
            case GraphicsAPI::None:

            default:
                VT_CORE_LOG_FATAL("Graphics API Not Supported!");
                break;
        }

        context->Initialize(reinterpret_cast<void*>(&window));
        VT_CORE_LOG_TRACE("Graphics Context Created!");
    }

    WindowImpl::~WindowImpl()
    {
        windowsCount--;
        if (windowsCount == 0) Shutdown();
    }

    void WindowImpl::Update()
    {

    }

    void WindowImpl::Present()
    {

    }

    void WindowImpl::SetTitle(std::wstring_view title)
    {

    }

    void WindowImpl::ActivateContext()
    {

    }

    void WindowImpl::Initialize()
    {
        display = XOpenDisplay(nullptr);
    }

    void WindowImpl::Shutdown()
    {

    }
}
#endif

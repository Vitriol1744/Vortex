//
// Created by Vitriol1744 on 29.06.2021.
//
#include "vtpch.hpp"
#include "Core/Macros.hpp"

#ifdef VT_PLATFORM_WINDOWS
#include "Win32GL46Context.hpp"
namespace Vortex
{
    bool GL46Context::initialized = false;
    
    GL46Context::GL46Context(void* windowHandle)
        : windowHandle(static_cast<HWND>(windowHandle))
    {
        Initialize();
        
        deviceContext = GetDC(reinterpret_cast<HWND>(windowHandle));

        const int32 pixelFormatAttributes[] =
        {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
            WGL_COLOR_BITS_ARB, 32,
            WGL_ALPHA_BITS_ARB, 8,
            WGL_DEPTH_BITS_ARB, 24,
            WGL_STENCIL_BITS_ARB, 8,
            WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
            WGL_SAMPLES_ARB, 4,
            0
        };

        int32 pixelFormat;
        UINT formatsCount;
        const bool status = wglChoosePixelFormatARB(deviceContext, pixelFormatAttributes, nullptr, 1, &pixelFormat, &formatsCount);
        VT_CORE_ASSERT(status != 0 && formatsCount != 0);

        PIXELFORMATDESCRIPTOR pfd;
        DescribePixelFormat(deviceContext, pixelFormat, sizeof(pfd), &pfd);
        SetPixelFormat(deviceContext, pixelFormat, &pfd);

        const int32 contextAttributes[] =
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB,
            4,
            WGL_CONTEXT_MINOR_VERSION_ARB,
            5,
            WGL_CONTEXT_PROFILE_MASK_ARB,
            WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0
        };

        renderingContext = wglCreateContextAttribsARB(deviceContext, nullptr, contextAttributes);
        VT_CORE_ASSERT(renderingContext != nullptr);
        
        VT_CORE_ASSERT(wglMakeCurrent(deviceContext, renderingContext) != 0);}
    }
    
    GL46Context::~GL46Context()
    {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(renderingContext);
        ReleaseDC(window, deviceContext);
    }

    void GL46Context::Initialize()
    {
        if (initialized) return;
        
        HINSTANCE hInstance = GetModuleHandleW(nullptr);
        const wchar_t* fakeWindowClass = L"Fake Window Class";

        WNDCLASSEXW wcex{};
        wcex.cbSize = sizeof(wcex);
        wcex.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
        wcex.lpfnWndProc = DefWindowProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hCursor = LoadCursor(hInstance, IDC_ARROW);
        wcex.hIcon = nullptr;
        wcex.hbrBackground = nullptr;
        wcex.hInstance = hInstance;
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = fakeWindowClass;

        RegisterClassExW(&wcex);

        HWND fakeWindow = CreateWindowExW
        (
            0, fakeWindowClass, L"Fake Window", WS_OVERLAPPEDWINDOW,
            0, 0, 100, 100, nullptr, nullptr, hInstance, nullptr
        );

        HDC fakeDC = GetDC(fakeWindow);

        PIXELFORMATDESCRIPTOR fakePFD;
        memset(&fakePFD, 0, sizeof(fakePFD));
        fakePFD.nSize = sizeof(fakePFD);
        fakePFD.nVersion = 1;
        fakePFD.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        fakePFD.iPixelType = PFD_TYPE_RGBA;
        fakePFD.cColorBits = 32;
        fakePFD.cAlphaBits = 8;
        fakePFD.cDepthBits = 24;

        const int32 fakePFDID = ChoosePixelFormat(fakeDC, &fakePFD);
        VT_CORE_ASSERT(fakePFDID != 0);
        VT_CORE_ASSERT(SetPixelFormat(fakeDC, fakePFDID, &fakePFD) != 0);

        HGLRC fakeRC = wglCreateContext(fakeDC);
        VT_CORE_ASSERT(fakeRC != nullptr);
        VT_CORE_ASSERT(wglMakeCurrent(fakeDC, fakeRC) != 0);

        PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
        wglChoosePixelFormatARB = reinterpret_cast<PFNWGLCHOOSEPIXELFORMATARBPROC>(wglGetProcAddress("wglChoosePixelFormatARB"));
        VT_CORE_ASSERT(wglChoosePixelFormatARB != nullptr);

        PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
        wglCreateContextAttribsARB = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));
        VT_CORE_ASSERT(wglCreateContextAttribsARB != nullptr);
    
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(fakeRC);
        ReleaseDC(fakeWindow, fakeDC);
        DestroyWindow(fakeWindow);
        UnregisterClassW(fakeWindowClass, hInstance);
        
        VT_CORE_ASSERT(Vortex::LoadGLFunctions() == true);
        
        initialized = true;
    }

    void GL46Context::Present()
    {
        ::SwapBuffers(deviceContext);
    }
}
#endif

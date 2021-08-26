//
// Created by Vitriol1744 on 23.08.2021.
//
#include "Vortex/Platform/Platform.hpp"

#include "Vortex/Platform/OpenGL.hpp"

#ifdef VT_PLATFORM_WINDOWS
#include "Vortex/Platform/WGL/WGL.hpp"

#include <mutex>

using PFNWGLCHOOSEPIXELFORMATARBPROC = BOOL(WINAPI*)(HDC hdc, const int* piAttribIList, const FLOAT* pfAttribFList, UINT nMaxFormats, int* piFormats, UINT* nNumFormats);
using PFNWGLCREATECONTEXTATTRIBSARBPROC = HGLRC(WINAPI*)(HDC hDC, HGLRC hShareContext, const int* attribList);

extern "C"
{
	// Force Machine to use Dedicated Graphics
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001; // NVidia
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;   // AMD
}

namespace Vortex::Platform
{
	HINSTANCE hInstance;
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormat;
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribs;

	struct WGLContext
	{
		HWND hWnd = nullptr;
		HDC deviceContext = nullptr;
		HGLRC renderingContext = nullptr;

		~WGLContext()
		{
			wglMakeCurrent(nullptr, nullptr);
			wglDeleteContext(renderingContext);
			ReleaseDC(hWnd, deviceContext);
		}
	};

	static void InitializeWGL()
	{
		hInstance = GetModuleHandleW(nullptr);
		constexpr LPCWSTR fake_lpClassName = L"Fake Window Class";

		WNDCLASSEXW wcex{};
		wcex.cbSize = sizeof(wcex);
		wcex.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
		wcex.lpfnWndProc = DefWindowProcW;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hCursor = LoadCursorW(hInstance, MAKEINTRESOURCEW(32512));
		wcex.hIcon = nullptr;
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
		wcex.hInstance = hInstance;
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = fake_lpClassName;
	
		RegisterClassExW(&wcex);

		HWND fake_hWnd;
		fake_hWnd = CreateWindowExW(0, fake_lpClassName, L"", WS_OVERLAPPEDWINDOW, 0, 0, 800, 600, nullptr, nullptr, hInstance, nullptr);

		HDC fakeDC = GetDC(fake_hWnd);

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

		wglChoosePixelFormat = reinterpret_cast<PFNWGLCHOOSEPIXELFORMATARBPROC>(wglGetProcAddress("wglChoosePixelFormatARB"));
		VT_CORE_ASSERT(wglChoosePixelFormat != nullptr);

		wglCreateContextAttribs = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));
		VT_CORE_ASSERT(wglCreateContextAttribs != nullptr);

		wglMakeCurrent(nullptr, nullptr);
		wglDeleteContext(fakeRC);
		ReleaseDC(fake_hWnd, fakeDC);
		DestroyWindow(fake_hWnd);
		UnregisterClassW(fake_lpClassName, hInstance);
	}

	EGLContext EGLCreateContext(EGLContextCreateInfo& createInfo)
	{
		static std::once_flag flag;
		std::call_once(flag, InitializeWGL);

		Ref<WGLContext> context = CreateRef<WGLContext>();
		context->hWnd = reinterpret_cast<HWND>(createInfo.windowHandle);
		context->deviceContext = GetDC(context->hWnd);

		const int32 pixelFormatAttributes[] =
		{
			WGL_DRAW_TO_WINDOW_ARB,     GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB,     GL_TRUE,
			WGL_DOUBLE_BUFFER_ARB,      GL_TRUE,
			WGL_PIXEL_TYPE_ARB,         WGL_TYPE_RGBA_ARB,
			WGL_ACCELERATION_ARB,       WGL_FULL_ACCELERATION_ARB,
			WGL_COLOR_BITS_ARB,         32,
			WGL_ALPHA_BITS_ARB,         8,
			WGL_DEPTH_BITS_ARB,         24,
			WGL_STENCIL_BITS_ARB,       8,
			WGL_SAMPLE_BUFFERS_ARB,     GL_TRUE,
			WGL_SAMPLES_ARB,            4,
			0
		};

		int32 pixelFormat = 0;
		UINT formatsCount = 0;

		const bool status = wglChoosePixelFormat ? wglChoosePixelFormat(context->deviceContext, pixelFormatAttributes, nullptr, 1, &pixelFormat, &formatsCount) : false;
		VT_CORE_ASSERT(status != 0 && formatsCount != 0);

		PIXELFORMATDESCRIPTOR pfd;
		DescribePixelFormat(context->deviceContext, pixelFormat, sizeof(pfd), &pfd);
		SetPixelFormat(context->deviceContext, pixelFormat, &pfd);

		const int32 contextAttributes[] =
		{
			WGL_CONTEXT_MAJOR_VERSION_ARB,	createInfo.openGLContextVersionMajor,
			WGL_CONTEXT_MINOR_VERSION_ARB,	createInfo.openGLContextVersionMinor,
			WGL_CONTEXT_PROFILE_MASK_ARB,	WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			GL_NONE
		};

		if (wglCreateContextAttribs) context->renderingContext = wglCreateContextAttribs(context->deviceContext, 0, contextAttributes);
		VT_CORE_ASSERT(context->renderingContext != 0);
		
		Ref<WGLContext> sharedContext = std::reinterpret_pointer_cast<WGLContext>(createInfo.sharedContext);
		wglShareLists(context->context, sharedContext ? sharedContext->renderingContext : nullptr);
		
		wglMakeCurrent(context->deviceContext, context->renderingContext);

		return { context };
	}
	void EGLSwapBuffers(EGLContext& _context)
	{
		Ref<WGLContext> context = std::reinterpret_pointer_cast<WGLContext>(_context.nativeContext);
		SwapBuffers(context->deviceContext);
	}
	void EGLMakeContextCurrent(EGLContext& _context)
	{
		Ref<WGLContext> context = std::reinterpret_pointer_cast<WGLContext>(_context.nativeContext);

		wglMakeCurrent(nullptr, nullptr);
		wglMakeCurrent(context->deviceContext, context->renderingContext);
	}
}
#endif
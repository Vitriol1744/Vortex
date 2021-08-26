//
// Created by Vitriol1744 on 25.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Platform/EGL.hpp"

#include <Windows.h>

namespace Vortex::Platform
{
	using PFNWGLCHOOSEPIXELFORMATARBPROC = BOOL(WINAPI*)(HDC hdc, const int* piAttribIList, const FLOAT* pfAttribFList, UINT nMaxFormats, int* piFormats, UINT* nNumFormats);
	using PFNWGLCREATECONTEXTATTRIBSARBPROC = HGLRC(WINAPI*)(HDC hDC, HGLRC hShareContext, const int* attribList);

	extern PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormat;
	extern PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribs;
}
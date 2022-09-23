//
// Created by Vitriol1744 on 07.09.2022.
//
#include "Vortex/vtpch.hpp"
#include "Win32WindowImpl.hpp"

#include "Vortex/Core/Utf.hpp"
#include "Vortex/Core/Events/EventSystem.hpp"
#include "Vortex/Core/Events/EventType.hpp"
#include "Vortex/Core/Events/VortexEvents.hpp"

#include "Vortex/Graphics/API/IRendererAPI.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanContext.hpp"

using namespace Vortex::Graphics;
using namespace Vortex::Math;
using namespace Vortex::Input;

#define VTCreateWindow(width, height, title, style, x, y) \
    CreateWindowExW(0, windowClassName, title.data(), style, x, y, width, height, nullptr, nullptr, hInstance, nullptr)

namespace Vortex
{
    namespace
    {
        KeyCode     VTKeyCode(uint32 keycode, bool extended = false)
        {
            switch (keycode)
            {
                case '0':           return KeyCode::Num0;
                case '1':           return KeyCode::Num1;
                case '2':           return KeyCode::Num2;
                case '3':           return KeyCode::Num3;
                case '4':           return KeyCode::Num4;
                case '5':           return KeyCode::Num5;
                case '6':           return KeyCode::Num6;
                case '7':           return KeyCode::Num7;
                case '8':           return KeyCode::Num8;
                case '9':           return KeyCode::Num9;
                case 'A':           return KeyCode::A;
                case 'B':           return KeyCode::B;
                case 'C':           return KeyCode::C;
                case 'D':           return KeyCode::D;
                case 'E':           return KeyCode::E;
                case 'F':           return KeyCode::F;
                case 'G':           return KeyCode::G;
                case 'H':           return KeyCode::H;
                case 'I':           return KeyCode::I;
                case 'J':           return KeyCode::J;
                case 'K':           return KeyCode::K;
                case 'L':           return KeyCode::L;
                case 'M':           return KeyCode::M;
                case 'N':           return KeyCode::N;
                case 'O':           return KeyCode::O;
                case 'P':           return KeyCode::P;
                case 'Q':           return KeyCode::Q;
                case 'R':           return KeyCode::R;
                case 'S':           return KeyCode::S;
                case 'T':           return KeyCode::T;
                case 'U':           return KeyCode::U;
                case 'V':           return KeyCode::V;
                case 'W':           return KeyCode::W;
                case 'X':           return KeyCode::X;
                case 'Y':           return KeyCode::Y;
                case 'Z':           return KeyCode::Z;
                case VK_OEM_3:      return KeyCode::Tilde;
                case VK_F1:         return KeyCode::F1;
                case VK_F2:         return KeyCode::F2;
                case VK_F3:         return KeyCode::F3;
                case VK_F4:         return KeyCode::F4;
                case VK_F5:         return KeyCode::F5;
                case VK_F6:         return KeyCode::F6;
                case VK_F7:         return KeyCode::F7;
                case VK_F8:         return KeyCode::F8;
                case VK_F9:         return KeyCode::F9;
                case VK_F10:        return KeyCode::F10;
                case VK_F11:        return KeyCode::F11;
                case VK_F12:        return KeyCode::F12;
                case VK_F13:        return KeyCode::F13;
                case VK_F14:        return KeyCode::F14;
                case VK_F15:        return KeyCode::F15;
                case VK_ESCAPE:     return KeyCode::Escape;
                case VK_BACK:       return KeyCode::BackSpace;
                case VK_TAB:        return KeyCode::Tab;
                case VK_CAPITAL:    return KeyCode::CapsLock;
                case VK_RETURN:     return extended ? KeyCode::Enter : KeyCode::Return;
                case VK_LSHIFT:     return KeyCode::LShift;
                case VK_RSHIFT:     return KeyCode::RShift;
                case VK_LCONTROL:   return KeyCode::LCtrl;
                case VK_RCONTROL:   return KeyCode::RCtrl;
                case VK_MENU:       return extended ? KeyCode::RAlt : KeyCode::LAlt;
                case VK_LWIN:       return KeyCode::LSystem;
                case VK_RWIN:       return KeyCode::RSystem;
                case VK_SPACE:      return KeyCode::Space;
                case VK_OEM_MINUS:  return KeyCode::Hyphen;
                case VK_OEM_PLUS:   return KeyCode::Equal;
                case VK_DECIMAL:    return KeyCode::Decimal;
                case VK_OEM_4:      return KeyCode::LBracket;
                case VK_OEM_6:      return KeyCode::RBracket;
                case VK_OEM_1:      return KeyCode::Semicolon;
                case VK_OEM_7:      return KeyCode::Apostrophe;
                case VK_OEM_COMMA:  return KeyCode::Comma;
                case VK_OEM_PERIOD: return KeyCode::Period;
                case VK_OEM_2:      return KeyCode::Slash;
                case VK_OEM_5:      return KeyCode::BackSlash;
                case VK_UP:         return KeyCode::Up;
                case VK_DOWN:       return KeyCode::Down;
                case VK_LEFT:       return KeyCode::Left;
                case VK_RIGHT:      return KeyCode::Right;
                case VK_NUMPAD0:    return KeyCode::Numpad0;
                case VK_NUMPAD1:    return KeyCode::Numpad1;
                case VK_NUMPAD2:    return KeyCode::Numpad2;
                case VK_NUMPAD3:    return KeyCode::Numpad3;
                case VK_NUMPAD4:    return KeyCode::Numpad4;
                case VK_NUMPAD5:    return KeyCode::Numpad5;
                case VK_NUMPAD6:    return KeyCode::Numpad6;
                case VK_NUMPAD7:    return KeyCode::Numpad7;
                case VK_NUMPAD8:    return KeyCode::Numpad8;
                case VK_NUMPAD9:    return KeyCode::Numpad9;
                case VK_SEPARATOR:  return KeyCode::Separator;
                case VK_ADD:        return KeyCode::Add;
                case VK_SUBTRACT:   return KeyCode::Subtract;
                case VK_MULTIPLY:   return KeyCode::Multiply;
                case VK_DIVIDE:     return KeyCode::Divide;
                case VK_INSERT:     return KeyCode::Insert;
                case VK_DELETE:     return KeyCode::Delete;
                case VK_PRIOR:      return KeyCode::PageUp;
                case VK_NEXT:       return KeyCode::PageDown;
                case VK_HOME:       return KeyCode::Home;
                case VK_END:        return KeyCode::End;
            }

            return KeyCode::Unknown;
        }
        MouseCode   VTMouseCode(uint32 mouse_code)
        {
            switch (mouse_code)
            {
                case MK_LBUTTON:    return MouseCode::Left;
                case MK_MBUTTON:    return MouseCode::Middle;
                case MK_RBUTTON:    return MouseCode::Right;
                case MK_XBUTTON1:   return MouseCode::X1;
                case MK_XBUTTON2:   return MouseCode::X2;
            }

            return MouseCode::Unknown;
        }
        /*LONG        Win32Style(WindowStyle style, bool resizable)
        {
            LONG result = 0;
            if (style & WindowStyle::Titlebar) result |= WS_CAPTION;
            if (style & WindowStyle::MaximizeButton) result |= WS_MAXIMIZEBOX;
            if (style & WindowStyle::MinimizeButton) result |= WS_MAXIMIZEBOX;
            if (resizable) result |= WS_THICKFRAME;

            return result;
        }*/
    }
    static const char* ToStringGLFW_ErrorCode(int32 errorCode)
    {
        switch (errorCode)
        {
            case GLFW_NO_ERROR: return "GLFW_NO_ERROR";
            case GLFW_NOT_INITIALIZED: return "GLFW_NOT_INITIALIZED";
            case GLFW_NO_CURRENT_CONTEXT: return "GLFW_NO_CURRENT_CONTEXT";
            case GLFW_INVALID_ENUM: return "GLFW_INVALID_ENUM";
            case GLFW_INVALID_VALUE: return "GLFW_INVALID_VALUE";
            case GLFW_OUT_OF_MEMORY: return "GLFW_OUT_OF_MEMORY";
            case GLFW_API_UNAVAILABLE: return "GLFW_API_UNAVAILABLE";
            case GLFW_VERSION_UNAVAILABLE: return "GLFW_VERSION_UNAVAILABLE";
            case GLFW_PLATFORM_ERROR: return "GLFW_PLATFORM_ERROR";
            case GLFW_FORMAT_UNAVAILABLE: return "GLFW_FORMAT_UNAVAILABLE";
            case GLFW_NO_WINDOW_CONTEXT: return "GLFW_NO_WINDOW_CONTEXT";
        }

        return "Unknown GLFW_ErrorCode!";
    }

    uint32 WindowImpl::windowsCount = 0;

    WindowImpl::WindowImpl(uint32 width, uint32 height, uint32 bitsPerPixel, std::string_view title)
    {
        if (windowsCount == 0) Initialize();

        VT_CORE_ASSERT_MSG(width != 0 || height != 0, "Invalid window size! width: {}, height: {}", width, height);

        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
        glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
        glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
        glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);
        glfwWindowHint(GLFW_FLOATING, GLFW_FALSE);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);
        glfwWindowHint(GLFW_CENTER_CURSOR, GLFW_FALSE);
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
        glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_TRUE);
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);
        glfwWindowHint(GLFW_REFRESH_RATE, GLFW_DONT_CARE);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
        VT_CORE_ASSERT_MSG(window != NULL, "Failed to create window!");
        windowsCount++;

        uintptr_t windowHandle = reinterpret_cast<uintptr_t>(window);
        int32 framebufferWidth, framebufferHeight;
        glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
        data.graphicsContext = new VulkanContext(windowHandle, framebufferWidth, framebufferHeight);
        monitor = glfwGetPrimaryMonitor();

        glfwSetWindowUserPointer(window, &data);
        #pragma region window_callbacks
        #define VTGetWindowData *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window))
        #define SendEvent(e) EventSystem::GetInstance()->SendEvent(e)
        auto glfwWindowPosCallback = [](GLFWwindow* window, int xpos, int ypos)
        {
            WindowData& data = VTGetWindowData;
            data.position.x = xpos;
            data.position.y = ypos;

            WindowMovedEvent e;
            e.window = data.window;
            e.position.x = xpos;
            e.position.y = ypos;
            SendEvent(e);
        };
        auto glfwWindowSizeCallback = [](GLFWwindow* window, int width, int height)
        {
            WindowData& data = VTGetWindowData;
            data.width = width;
            data.height = height;

            WindowResizedEvent e;
            e.window = data.window;
            e.size.x = width;
            e.size.y = height;
            SendEvent(e);
        };
        auto glfwWindowCloseCallback = [](GLFWwindow* window)
        {
            WindowData& data = VTGetWindowData;
            data.isOpen = false;

            WindowClosedEvent e;
            e.window = data.window;
            SendEvent(e);
        };
        auto glfwWindowFocusCallback = [](GLFWwindow* window, int focused)
        {
            WindowData& data = VTGetWindowData;
            data.focused = focused;

            WindowFocusChangedEvent e;
            e.window = data.window;
            e.focused = focused;
            SendEvent(e);
        };
        auto glfwIconifyCallback = [](GLFWwindow* window, int iconified)
        {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
            if (!iconified) return;
            data.minimized = true;

            WindowMinimizedEvent e;
            e.window = data.window;
            SendEvent(e);
        };
        auto glfwMaximizeCallback = [](GLFWwindow* window, int maximized)
        {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
            if (!maximized) return;
            data.minimized = false;

            WindowMaximizedEvent e;
            e.window = data.window;
            SendEvent(e);
        };
        auto glfwFramebufferSizeCallback = [](GLFWwindow* window, int width, int height)
        {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

            FramebufferResizedEvent e;
            e.window = data.window;
            e.size = { width, height };
            SendEvent(e);
        };
        auto glfwMouseButtonCallback = [](GLFWwindow* window, int button, int action, int mods)
        {
            WindowData& data = VTGetWindowData;
            data.mouseButtons[button] = action == GLFW_PRESS;

            MouseCode mouseCode = static_cast<MouseCode>(button);
            if (action == GLFW_PRESS || GLFW_REPEAT)
            {
                MouseButtonPressedEvent e;
                e.window = data.window;
                e.mouseCode = mouseCode;
                //TODO: Win32: Double click handling
                e.doubleClick = action == GLFW_REPEAT;
                SendEvent(e);
                return;
            }
            MouseButtonReleasedEvent e;
            e.window = data.window;
            e.mouseCode = mouseCode;
            SendEvent(e);
        };
        auto glfwCursorPosCallback = [](GLFWwindow* window, double xpos, double ypos)
        {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
            data.mousePosition = { xpos, ypos };

        };
        auto glfwCursorEnterCallback = [](GLFWwindow* window, int entered)
        {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
            data.mouseHovered = entered;

            MouseCursorEnterEvent e;
            e.window = data.window;
            e.entered = entered;
            SendEvent(e);
        };
        auto glfwScrollCallback = [](GLFWwindow* window, double xoffset, double yoffset)
        {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
            data.mouseScroll.x = xoffset;
            data.mouseScroll.y = yoffset;

            MouseScrolledEvent e;
            e.window = data.window;
            e.scroll = { xoffset, yoffset };
            SendEvent(e);
        };
        auto glfwKeyCallback = [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            WindowData& data = VTGetWindowData;

            KeyCode keycode = static_cast<KeyCode>(key);
            if (action == GLFW_PRESS || action == GLFW_REPEAT)
            {
                KeyPressedEvent e;
                e.window = data.window;
                e.keycode = keycode;
                e.repeated = action == GLFW_REPEAT;
                SendEvent(e);
                return;
            }
            KeyReleasedEvent e;
            e.window = data.window;
            e.keycode = keycode;
            SendEvent(e);
        };
        auto glfwCharCallback = [](GLFWwindow* window, unsigned int codepoint)
        {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

            KeyTypedEvent e;
            e.window = data.window;
            e.character = codepoint;
            SendEvent(e);
        };
        auto glfwCharModsCallback = [](GLFWwindow* window, unsigned int codepoint, int mods)
        {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

            KeyTypedEvent e;
            e.window = data.window;
            e.character = codepoint;
            SendEvent(e);
        };
        #pragma endregion

        glfwSetWindowPosCallback(window, glfwWindowPosCallback);
        glfwSetWindowSizeCallback(window, glfwWindowSizeCallback);
        glfwSetWindowCloseCallback(window, glfwWindowCloseCallback);
        glfwSetWindowFocusCallback(window, glfwWindowFocusCallback);
        glfwSetWindowIconifyCallback(window, glfwIconifyCallback);
        glfwSetWindowMaximizeCallback(window, glfwMaximizeCallback);
        glfwSetFramebufferSizeCallback(window, glfwFramebufferSizeCallback);
        glfwSetMouseButtonCallback(window, glfwMouseButtonCallback);
        glfwSetCursorPosCallback(window, glfwCursorPosCallback);
        glfwSetCursorEnterCallback(window, glfwCursorEnterCallback);
        glfwSetScrollCallback(window, glfwScrollCallback);
        glfwSetKeyCallback(window, glfwKeyCallback);
        glfwSetCharCallback(window, glfwCharCallback);
        glfwSetCharModsCallback(window, glfwCharModsCallback);
    }
    WindowImpl::~WindowImpl()
    {
        delete data.graphicsContext;
        glfwDestroyWindow(window);
        windowsCount--;
        if (windowsCount == 0) Shutdown();
    }

    void WindowImpl::PollEvents()
    {
        glfwPollEvents();
    }
    void WindowImpl::Present()
    {
        data.graphicsContext->Present();
    }

    void WindowImpl::ShowCursor() const noexcept
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    void WindowImpl::HideCursor() const noexcept
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    void WindowImpl::RequestFocus() const
    {
        glfwFocusWindow(window);
    }
    void WindowImpl::SetCursor(CursorShape shape)
    {
        GLFWcursor* cursor = nullptr;
        switch (shape)
        {
            case CursorShape::eNone: break;
            case CursorShape::eArrow: cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR); break;
            case CursorShape::eIBeam: cursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR); break;
            case CursorShape::eCrosshair: cursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR); break;
            case CursorShape::eGrabbed: cursor = glfwCreateStandardCursor(GLFW_POINTING_HAND_CURSOR); break;
            case CursorShape::eHResize: cursor = glfwCreateStandardCursor(GLFW_RESIZE_NS_CURSOR); break;
            case CursorShape::eVResize: cursor = glfwCreateStandardCursor(GLFW_RESIZE_EW_CURSOR); break;
        }
    }
    void WindowImpl::SetCursor(const Ref<Pixel> pixels, int32 width, int32 height)
    {
        //TODO: Win32: SetCursor
        uint32 size = static_cast<uint64>(width) * static_cast<uint64>(height) * 4L;
        unsigned char* cursorPixels = new unsigned char[size];

        for (uint64 i = 0; i < size / 4; ++i)
        {
            cursorPixels[i * 4 + 0] = pixels.get()[i * 4 + 2];
            cursorPixels[i * 4 + 1] = pixels.get()[i * 4 + 1];
            cursorPixels[i * 4 + 2] = pixels.get()[i * 4 + 0];
            cursorPixels[i * 4 + 3] = pixels.get()[i * 4 + 3];
        }
        GLFWimage cursorImage;
        cursorImage.pixels = cursorPixels;
        cursorImage.width = width;
        cursorImage.height = height;
        GLFWcursor* cursor = glfwCreateCursor(&cursorImage, 0, 0);
        if (!cursor) VTCoreLogError("Failed to create GLFWcursor!");
        glfwSetCursor(window, cursor);
    }
    void WindowImpl::SetFullscreen(bool fullscreen)
    {
        if (data.fullscreen == fullscreen) return;

        if (fullscreen)
        {
            glfwGetWindowPos(window, &data.windowModeSize.x, &data.windowModeSize.y);
            glfwGetWindowSize(window, &data.windowModePosition.x, &data.windowModePosition.y);

            const GLFWvidmode* mode = glfwGetVideoMode(monitor);

            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, 0 );
        }
        else
        {
            int32 width = data.windowModeSize.x;
            int32 height = data.windowModeSize.y;
            int32 x = data.windowModePosition.x;
            int32 y = data.windowModePosition.y;

            glfwSetWindowMonitor(window, nullptr, width, height, x, y, 0);
        }
        data.fullscreen = fullscreen;
    }
    void WindowImpl::Maximize()
    {
        glfwRestoreWindow(window);
    }
    void WindowImpl::Minimize()
    {
        glfwIconifyWindow(window);
    }
    void WindowImpl::SetIcon(std::string_view path, int32 width, int32 height)
    {
        //TODO: Win32: SetIcon(file_path)
    }
    void WindowImpl::SetIcon(const Pixel* pixels, int32 width, int32 height)
    {
        uint32 size = static_cast<uint64>(width) * static_cast<uint64>(height) * 4L;
        unsigned char* iconPixels = new unsigned char[size];

        for (uint64 i = 0; i < size / 4; ++i)
        {
            iconPixels[i * 4 + 0] = pixels[i * 4 + 2];
            iconPixels[i * 4 + 1] = pixels[i * 4 + 1];
            iconPixels[i * 4 + 2] = pixels[i * 4 + 0];
            iconPixels[i * 4 + 3] = pixels[i * 4 + 3];
        }
        GLFWimage icon;
        icon.pixels = iconPixels;
        icon.width = width;
        icon.height = height;
        glfwSetWindowIcon(window, 1, &icon);
        delete[] iconPixels;
    }
    void WindowImpl::SetTitle(std::string_view title) const noexcept
    {
        glfwSetWindowTitle(window, title.data());
        data.title = title;
    }
    void WindowImpl::SetPosition(uint32 x, uint32 y) const
    {
        glfwSetWindowPos(window, x, y);
    }
    void WindowImpl::SetResizable(bool resizable)
    {
        glfwSetWindowAttrib(window, GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
    }
    void WindowImpl::SetSize(uint32 width, uint32 height)
    {
        glfwSetWindowSize(window, width, height);
    }
    void WindowImpl::SetStyle(WindowStyle style)
    {
        if (style & WindowStyle::eTitlebar && style & WindowStyle::eMaximizeButton && style & WindowStyle::eMinimizeButton)
            glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
        else glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
    }
    void WindowImpl::SetVisible(bool visible) const
    {
        if (visible == data.visible) return;
        if (visible) glfwShowWindow(window);
        else glfwHideWindow(window);
    }
    void WindowImpl::SetTopMost(bool topmost)
    {
        glfwSetWindowAttrib(window, GLFW_FLOATING, topmost ? GLFW_TRUE : GLFW_FALSE);
    }

    void WindowImpl::Initialize()
    {
        VT_CORE_ASSERT_MSG(glfwInit() == GLFW_TRUE, "Failed to Initialize GLFW!");

        auto glfwErrorCallback = [](int error_code, const char* description)
        {
            VTCoreLogError("GLFW: error_code: {}, description: {}", ToStringGLFW_ErrorCode(error_code), description);
        };

        int32 glfw_Major, glfw_Minor, glfw_Patch;
        glfwGetVersion(&glfw_Major, &glfw_Minor, &glfw_Patch);
        VTCoreLogInfo("GLFW: Version: {}.{}.{}", glfw_Major, glfw_Minor, glfw_Patch);

        glfwSetErrorCallback(glfwErrorCallback);
    }
    void WindowImpl::Shutdown()
    {
        glfwTerminate();
    }
}
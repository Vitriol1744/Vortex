/*
 * Created by v1tr10l7 on 25.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Window/Win32/Win32Monitor.hpp>

namespace Vortex
{
    Win32Monitor::Win32Monitor(GLFWmonitor* handle)
        : m_Handle(handle)
    {

        m_Name                            = glfwGetMonitorName(m_Handle);

        const GLFWvidmode* currentMode    = glfwGetVideoMode(m_Handle);
        m_CurrentVideoMode.Width          = currentMode->width;
        m_CurrentVideoMode.Height         = currentMode->height;
        m_CurrentVideoMode.RedBits        = currentMode->redBits;
        m_CurrentVideoMode.GreenBits      = currentMode->greenBits;
        m_CurrentVideoMode.BlueBits       = currentMode->blueBits;
        m_CurrentVideoMode.RefreshRate    = currentMode->refreshRate;

        i32                videoModeCount = 0;
        const GLFWvidmode* modes = glfwGetVideoModes(m_Handle, &videoModeCount);
        m_VideoModes.Reserve(videoModeCount);

        for (i32 i = 0; i < videoModeCount; i++)
        {
            VideoMode   current;
            GLFWvidmode glfwVideoMode = modes[i];
            current.Width             = glfwVideoMode.width;
            current.Height            = glfwVideoMode.height;
            current.RedBits           = glfwVideoMode.redBits;
            current.GreenBits         = glfwVideoMode.greenBits;
            current.BlueBits          = glfwVideoMode.blueBits;
            current.RefreshRate       = glfwVideoMode.refreshRate;
            m_VideoModes.PushBack(current);
        }

        glfwGetMonitorPhysicalSize(m_Handle, &m_PhysicalWidth,
                                   &m_PhysicalHeight);

        const GLFWgammaramp* ramp = glfwGetGammaRamp(m_Handle);
        struct GammaRamp     currentRamp;
        currentRamp.Red.Resize(256);
        currentRamp.Green.Resize(256);
        currentRamp.Blue.Resize(256);

        if (ramp)
        {
            i32 size = ramp->size > 256 ? 256 : ramp->size;
            Memory::Copy(currentRamp.Red.Raw(), ramp->red, size);
            Memory::Copy(currentRamp.Green.Raw(), ramp->green,
                         size * sizeof(u16));
            Memory::Copy(currentRamp.Blue.Raw(), ramp->blue,
                         size * sizeof(u16));
            m_CurrentRamp  = currentRamp;
            m_OriginalRamp = currentRamp;
        }
        glfwSetMonitorUserPointer(m_Handle, this);
    }

    Math::Vec2i Win32Monitor::Position() const
    {
        Math::Vec2i ret;
        glfwGetMonitorPos(m_Handle, &ret.x, &ret.y);

        return ret;
    }
    Rectangle<> Win32Monitor::WorkArea() const
    {
        Rectangle<> ret;
        glfwGetMonitorWorkarea(m_Handle, &ret.X, &ret.Y, &ret.Width,
                               &ret.Height);

        return ret;
    }
    Math::Vec2f Win32Monitor::ContentScale() const
    {
        Math::Vec2f ret;
        glfwGetMonitorContentScale(m_Handle, &ret.x, &ret.y);

        return ret;
    }
    void Win32Monitor::SetGamma(f32 gamma) const
    {
        glfwSetGamma(m_Handle, gamma);
    }
    void Win32Monitor::SetGammaRamp(struct GammaRamp& gammaRamp)
    {
        GLFWgammaramp ramp;
        ramp.red      = gammaRamp.Red.Raw();
        ramp.green    = gammaRamp.Green.Raw();
        ramp.blue     = gammaRamp.Blue.Raw();
        ramp.size     = 256;

        m_CurrentRamp = gammaRamp;
        glfwSetGammaRamp(m_Handle, &ramp);
    }

    bool Win32Monitor::Initialize(Vector<::Ref<Monitor>>& monitors)
    {
        VtCoreAssert(glfwInit());
        i32           monitorCount;
        GLFWmonitor** glfwMonitors = glfwGetMonitors(&monitorCount);
        monitors.Reserve(monitorCount);

        for (i32 i = 0; i < monitorCount; i++)
            monitors.PushBack(CreateRef<Win32Monitor>(glfwMonitors[i]));

        glfwSetMonitorCallback(
            [](GLFWmonitor* handle, i32 event)
            {
                Win32Monitor* monitor = reinterpret_cast<Win32Monitor*>(
                    glfwGetMonitorUserPointer(handle));
                MonitorState state = MonitorState::eDisconnected;
                if (event == GLFW_CONNECTED) state = MonitorState::eConnected;

                MonitorEvents::MonitorStateChangedEvent(monitor, state);
            });

        return true;
    };
}; // namespace Vortex

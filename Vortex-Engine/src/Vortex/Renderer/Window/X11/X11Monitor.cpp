/*
 * Created by v1tr10l7 on 25.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/Window/X11/X11Monitor.hpp"

namespace Vortex
{
    X11Monitor::X11Monitor(GLFWmonitor* handle)
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
        m_VideoModes.reserve(videoModeCount);

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
            m_VideoModes.push_back(current);
        }

        glfwGetMonitorPhysicalSize(m_Handle, &m_PhysicalWidth,
                                   &m_PhysicalHeight);

        const GLFWgammaramp* ramp = glfwGetGammaRamp(m_Handle);
        GammaRamp            currentRamp;
        currentRamp.Red.resize(256);
        currentRamp.Green.resize(256);
        currentRamp.Blue.resize(256);

        if (ramp)
        {
            i32 size = ramp->size > 256 ? 256 : ramp->size;
            std::memcpy(currentRamp.Red.data(), ramp->red, size);
            std::memcpy(currentRamp.Green.data(), ramp->green,
                        size * sizeof(u16));
            std::memcpy(currentRamp.Blue.data(), ramp->blue,
                        size * sizeof(u16));
            m_CurrentRamp  = currentRamp;
            m_OriginalRamp = currentRamp;
        }
        glfwSetMonitorUserPointer(m_Handle, this);
    }

    Vec2i X11Monitor::GetPosition() const
    {
        Vec2i ret;
        glfwGetMonitorPos(m_Handle, &ret.x, &ret.y);

        return ret;
    }
    Vec4i X11Monitor::GetWorkArea() const
    {
        Vec4i ret;
        glfwGetMonitorWorkarea(m_Handle, &ret.x, &ret.y, &ret.z, &ret.w);

        return ret;
    }
    Vec2f X11Monitor::GetContentScale() const
    {
        Vec2f ret;
        glfwGetMonitorContentScale(m_Handle, &ret.x, &ret.y);

        return ret;
    }
    void X11Monitor::SetGamma(f32 gamma) const
    {
        glfwSetGamma(m_Handle, gamma);
    }
    void X11Monitor::SetGammaRamp(GammaRamp& gammaRamp)
    {
        GLFWgammaramp ramp;
        ramp.red      = gammaRamp.Red.data();
        ramp.red      = gammaRamp.Red.data();
        ramp.red      = gammaRamp.Red.data();
        ramp.size     = 256;

        m_CurrentRamp = gammaRamp;
        glfwSetGammaRamp(m_Handle, &ramp);
    }

    bool X11Monitor::Initialize(std::vector<Ref<Monitor>>& monitors)
    {
        VtCoreAssert(glfwInit());
        i32           monitorCount;
        GLFWmonitor** glfwMonitors = glfwGetMonitors(&monitorCount);
        monitors.reserve(monitorCount);

        for (i32 i = 0; i < monitorCount; i++)
            monitors.push_back(CreateRef<X11Monitor>(glfwMonitors[i]));

        glfwSetMonitorCallback(
            [](GLFWmonitor* handle, i32 event)
            {
                X11Monitor* monitor = reinterpret_cast<X11Monitor*>(
                    glfwGetMonitorUserPointer(handle));
                MonitorState state = MonitorState::eDisconnected;
                if (event == GLFW_CONNECTED) state = MonitorState::eConnected;

                MonitorEvents::monitorStateChangedEvent(monitor, state);
            });

        return true;
    };
}; // namespace Vortex

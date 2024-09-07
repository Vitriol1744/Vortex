/*
 * Created by v1tr10l7 on 25.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Window/Wayland/WaylandMonitor.hpp"

namespace Vortex
{
    WaylandMonitor::WaylandMonitor(u32 outputName, wl_output* output)
        : m_OutputName(outputName)
        , m_Output(output)
    {
        static const wl_output_listener outputListener = {
            .geometry    = OnOutputGeometry,
            .mode        = OnOutputMode,
            .done        = OnOutputDone,
            .scale       = OnOutputScale,
            .name        = OnOutputName,
            .description = OnOutputDescription,
        };

        auto gammaControlManager = Wayland::GetGammaControlManager();
        if (gammaControlManager)
            m_GammaControl = zwlr_gamma_control_manager_v1_get_gamma_control(
                gammaControlManager, m_Output);
        wl_output_add_listener(output, &outputListener, this);
    }
    WaylandMonitor::~WaylandMonitor()
    {
        VtCoreTrace("Wayland: Destroying wl_output...");
        if (m_GammaControl) zwlr_gamma_control_v1_destroy(m_GammaControl);

        wl_output_destroy(m_Output);
    }

    Vec2i       WaylandMonitor::GetPosition() const { return m_Position; }
    Rectangle<> WaylandMonitor::GetWorkArea() const
    {
        Rectangle<> workArea{};
        workArea.X      = m_Position.x;
        workArea.Y      = m_Position.y;
        workArea.Width  = m_CurrentVideoMode.Width;
        workArea.Height = m_CurrentVideoMode.Height;

        return workArea;
    }
    Vec2f WaylandMonitor::GetContentScale() const
    {
        Vec2f contentScale;
        contentScale.x = static_cast<f32>(m_ContentScale);
        contentScale.y = static_cast<f32>(m_ContentScale);

        return contentScale;
    }
    void WaylandMonitor::SetGamma(f32 gamma) const
    {
        VtUnused(gamma);
        VtTodo();
    }
    void WaylandMonitor::SetGammaRamp(GammaRamp& gammaRamp)
    {
        VtUnused(gammaRamp);
        VtTodo();
    }

    bool WaylandMonitor::Initialize()
    {
        Wayland::Initialize();

        return true;
    };

    void WaylandMonitor::Connect(u32 name, u32 version)
    {
        wl_registry* registry = Wayland::GetRegistry();
        wl_output*   output   = reinterpret_cast<wl_output*>(
            wl_registry_bind(registry, name, &wl_output_interface, version));

        s_Monitors.push_back(CreateRef<WaylandMonitor>(name, output));
    }
    void WaylandMonitor::Disconnect(u32 name)
    {
        auto& monitors
            = const_cast<std::vector<Ref<Monitor>>&>(Monitor::GetMonitors());

        monitors.erase(std::remove_if(
            monitors.begin(), monitors.end(),
            [name](Ref<Monitor> monitor) -> bool
            {
                auto wlMonitor
                    = std::dynamic_pointer_cast<WaylandMonitor>(monitor);

                if (wlMonitor && wlMonitor->m_OutputName == name)
                {
                    MonitorEvents::MonitorStateChangedEvent(
                        monitor.get(), MonitorState::eDisconnected);
                    return true;
                }

                return false;
            }));
    }

    void WaylandMonitor::OnOutputGeometry(void* userData, wl_output* output,
                                          i32 x, i32 y, i32 physicalWidth,
                                          i32 physicalHeight, i32 subpixel,
                                          const char* make, const char* model,
                                          i32 transform)
    {
        VtUnused(output);
        VtUnused(subpixel);
        VtUnused(transform);

        auto monitor              = reinterpret_cast<WaylandMonitor*>(userData);
        monitor->m_Position       = Vec2i(x, y);
        monitor->m_PhysicalWidth  = physicalWidth;
        monitor->m_PhysicalHeight = physicalHeight;
        monitor->m_Name           = fmt::format("{} {}", make, model);
    }
    void WaylandMonitor::OnOutputMode(void* userData, wl_output* output,
                                      u32 flags, i32 width, i32 height,
                                      i32 refreshRate)
    {
        VtUnused(output);

        auto      monitor = reinterpret_cast<WaylandMonitor*>(userData);
        VideoMode videoMode{};

        videoMode.Width       = width;
        videoMode.Height      = height;
        videoMode.RedBits     = 8;
        videoMode.GreenBits   = 8;
        videoMode.BlueBits    = 8;
        videoMode.RefreshRate = std::round(refreshRate / 1000.0);

        monitor->m_VideoModes.push_back(videoMode);
        if (flags & WL_OUTPUT_MODE_CURRENT)
            monitor->m_CurrentVideoMode = videoMode;
    }
    void WaylandMonitor::OnOutputDone(void* userData, wl_output* output)
    {
        VtUnused(output);

        auto monitor = reinterpret_cast<WaylandMonitor*>(userData);
        if (monitor->m_PhysicalWidth <= 0 || monitor->m_PhysicalHeight <= 0)
        {
            VideoMode& currentMode = monitor->m_CurrentVideoMode;
            monitor->m_PhysicalWidth
                = static_cast<i32>(currentMode.Width * 25.4f / 96.0f);
            monitor->m_PhysicalHeight
                = static_cast<i32>(currentMode.Height * 25.4f / 96.0f);
        }

        MonitorEvents::MonitorStateChangedEvent(monitor,
                                                MonitorState::eConnected);
    }
    void WaylandMonitor::OnOutputScale(void* userData, wl_output* output,
                                       i32 factor)
    {
        VtUnused(output);

        auto monitor            = reinterpret_cast<WaylandMonitor*>(userData);
        monitor->m_ContentScale = factor;
    }

    void WaylandMonitor::OnOutputName(void* userData, wl_output* output,
                                      const char* name)
    {
        VtUnused(output);

        auto monitor    = reinterpret_cast<WaylandMonitor*>(userData);
        monitor->m_Name = name;
    }
    void WaylandMonitor::OnOutputDescription(void* userData, wl_output* output,
                                             const char* description)
    {
        VtUnused(userData);
        VtUnused(output);
        VtUnused(description);
    }
}; // namespace Vortex

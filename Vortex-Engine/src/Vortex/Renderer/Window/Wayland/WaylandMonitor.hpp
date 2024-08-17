/*
 * Created by v1tr10l7 on 25.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Renderer/Window/Monitor.hpp"

#define GLFW_EXPOSE_NATIVE_WAYLAND
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace Vortex
{
    class VT_API WaylandMonitor : public Monitor
    {
      public:
        WaylandMonitor(GLFWmonitor* handle);

        virtual std::any GetNativeHandle() override { return m_Handle; }

        virtual Vec2i    GetPosition() const override;
        virtual Vec4i    GetWorkArea() const override;
        virtual Vec2f    GetContentScale() const override;
        virtual void     SetGamma(f32 gamma) const override;
        virtual void     SetGammaRamp(GammaRamp& gammaRamp) override;

        static bool      Initialize(std::vector<Ref<Monitor>>& monitors);

      private:
        wl_output*   m_MonitorHandle = nullptr;
        GLFWmonitor* m_Handle;
    };
}; // namespace Vortex

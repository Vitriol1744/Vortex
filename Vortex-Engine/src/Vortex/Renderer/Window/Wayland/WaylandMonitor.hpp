/*
 * Created by v1tr10l7 on 25.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Renderer/Window/Monitor.hpp"
#include "Vortex/Renderer/Window/Wayland/Wayland.hpp"

#include <GLFW/glfw3.h>

namespace Vortex
{
    class VT_API WaylandMonitor : public Monitor
    {
      public:
        WaylandMonitor(wl_output* output);
        ~WaylandMonitor();

        virtual std::any    GetNativeHandle() override { return m_Output; }

        virtual Vec2i       GetPosition() const override;
        virtual Rectangle<> GetWorkArea() const override;
        virtual Vec2f       GetContentScale() const override;
        virtual void        SetGamma(f32 gamma) const override;
        virtual void        SetGammaRamp(GammaRamp& gammaRamp) override;

        static bool         Initialize(std::vector<Ref<Monitor>>& monitors);

      private:
        wl_output*             m_Output       = nullptr;
        zwlr_gamma_control_v1* m_GammaControl = nullptr;
        Vec2i                  m_Position     = {0, 0};
        i32                    m_ContentScale = 0;

        static void OnOutputGeometry(void* userData, wl_output* output, i32 x,
                                     i32 y, i32 physicalWidth,
                                     i32 physicalHeight, i32 subpixel,
                                     const char* make, const char* model,
                                     i32 transform);
        static void OnOutputMode(void* userData, wl_output* output, u32 flags,
                                 i32 width, i32 height, i32 refreshRate);
        static void OnOutputDone(void* userData, wl_output* output);
        static void OnOutputScale(void* userData, wl_output* output,
                                  i32 factor);
        static void OnOutputName(void* userData, wl_output* output,
                                 const char* name);
        static void OnOutputDescription(void* userData, wl_output* output,
                                        const char* description);
    };
}; // namespace Vortex

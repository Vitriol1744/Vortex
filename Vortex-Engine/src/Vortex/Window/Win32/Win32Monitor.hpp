/*
 * Created by v1tr10l7 on 25.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Window/Monitor.hpp"

#include <GLFW/glfw3.h>

namespace Vortex
{
    class VT_API Win32Monitor : public Monitor
    {
      public:
        Win32Monitor(GLFWmonitor* handle);

        virtual std::any    GetNativeHandle() override { return m_Handle; }

        virtual Vec2i       GetPosition() const override;
        virtual Rectangle<> GetWorkArea() const override;
        virtual Vec2f       GetContentScale() const override;
        virtual void        SetGamma(f32 gamma) const override;
        virtual void        SetGammaRamp(GammaRamp& gammaRamp) override;

        static bool         Initialize(std::vector<Ref<Monitor>>& monitors);

      private:
        GLFWmonitor* m_Handle;
    };
}; // namespace Vortex

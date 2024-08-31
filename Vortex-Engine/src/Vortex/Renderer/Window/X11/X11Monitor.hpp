/*
 * Created by v1tr10l7 on 25.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Core.hpp"
#include "Vortex/Renderer/Window/Monitor.hpp"

namespace Vortex
{
    class VT_API X11Monitor : public Monitor
    {
      public:
        X11Monitor();

        virtual std::any GetNativeHandle() override
        {
            VtTodo();
            return nullptr;
        }

        virtual Vec2i       GetPosition() const override;
        virtual Rectangle<> GetWorkArea() const override;
        virtual Vec2f       GetContentScale() const override;
        virtual void        SetGamma(f32 gamma) const override;
        virtual void        SetGammaRamp(GammaRamp& gammaRamp) override;

        static bool         Initialize(std::vector<Ref<Monitor>>& monitors);

      private:
    };
}; // namespace Vortex

/*
 * Created by v1tr10l7 on 25.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Core.hpp>
#include <Vortex/Window/Monitor.hpp>

namespace Vortex
{
    class VT_API X11Monitor : public Monitor
    {
      public:
        X11Monitor();

        virtual std::any NativeHandle() override
        {
            VtTodo();
            return nullptr;
        }

        virtual Math::Vec2i Position() const override;
        virtual Rectangle<> WorkArea() const override;
        virtual Math::Vec2f ContentScale() const override;
        virtual void        SetGamma(f32 gamma) const override;
        virtual void        SetGammaRamp(struct GammaRamp& gammaRamp) override;

        static bool         Initialize(Vector<::Ref<Monitor>>& monitors);

      private:
    };
}; // namespace Vortex

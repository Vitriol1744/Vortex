/*
 * Created by v1tr10l7 on 27.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Layer.hpp"

#include "Vortex/Window/Window.hpp"

class SandboxLayer2D : public Vortex::Layer
{
  public:
    SandboxLayer2D(StringView name = "Layer")
        : Vortex::Layer(name)
    {
    }

    virtual void OnAttach() override;
    virtual void OnDetach() override;

    virtual void OnUpdate(Vortex::Timestep ts) override;
    virtual void OnRender() override;
    virtual void OnImGuiRender() override;
};

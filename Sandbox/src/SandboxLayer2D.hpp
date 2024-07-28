/*
 * Created by v1tr10l7 on 27.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Layer.hpp"

class SandboxLayer2D : public Vortex::Layer
{
  public:
    SandboxLayer2D(std::string_view name = "Layer")
        : Vortex::Layer(name)
    {
    }

    virtual void OnAttach();
    virtual void OnDetach();

    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnImGuiRender();
};

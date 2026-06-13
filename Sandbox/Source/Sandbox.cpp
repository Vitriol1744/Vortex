/*
 * Created by v1tr10l7 on 17.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "Vortex/Core/Log/Log.hpp"
#include "Vortex/Engine/Application.hpp"

#include "SandboxLayer2D.hpp"

using namespace Vortex;
class Game : public Application
{
  public:
    Game(const ApplicationSpecification& spec)
        : Application(spec)
    {
        PushLayer(CreateRef<SandboxLayer2D>("SandboxLayer2D"));
    }
};

Application* Vortex::CreateApplication(const CommandLineArguments&)
{
    ApplicationSpecification spec{};
    return new Game(spec);
}

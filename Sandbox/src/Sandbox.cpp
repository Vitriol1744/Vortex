/*
 * Created by v1tr10l7 on 17.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <iostream>

#include "Vortex/Core/Application.hpp"

using namespace Vortex;
class Game : public Application
{
  public:
    Game(const ApplicationSpecification& spec)
        : Application(spec)
    {
    }
};

Application* Vortex::CreateApplication(const CommandLineArguments&)
{
    ApplicationSpecification spec{};
    return new Game(spec);
}

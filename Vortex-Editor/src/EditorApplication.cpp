/*
 * Created by v1tr10l7 on 07.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Core/Log/Log.hpp>
#include <Vortex/Engine/Application.hpp>

#include "EditorLayer.hpp"

using namespace Vortex;
class Editor : public Application
{
  public:
    Editor(const ApplicationSpecification& spec)
        : Application(spec)
    {
        PushLayer(CreateRef<EditorLayer>());
    }
};

Application* Vortex::CreateApplication(const CommandLineArguments&)
{
    ApplicationSpecification spec{};
    return new Editor(spec);
}

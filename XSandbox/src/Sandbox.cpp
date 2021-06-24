//
// Created by Vitriol1744 on 22.06.2021.
//
#include "Sandbox.hpp"

#include "Vortex/Core/EntryPoint.hpp"

#include <iostream>

Sandbox::Sandbox()
{
    std::cout << "Hello, World!\n";
}

Vortex::Application* Vortex::CreateApplication()
{
    return new Sandbox;
}


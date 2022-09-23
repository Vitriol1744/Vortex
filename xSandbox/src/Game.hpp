//
// Created by Vitriol1744 on 05.09.2022.
//
#ifndef VORTEX_GAME_HPP
#define VORTEX_GAME_HPP

#include "Vortex/Engine/Application.hpp"

using namespace Vortex;

class Game : public Application
{

};

Application* Vortex::CreateApplication(std::vector<char*> args)
{
    return new Game();
}

#endif //VORTEX_GAME_HPP

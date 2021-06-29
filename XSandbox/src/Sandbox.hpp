//
// Created by Vitriol1744 on 22.06.2021.
//
#pragma once

#include "Vortex.hpp"

class Sandbox : public Vortex::Application
{
    public:
        Sandbox();

        void Update() override;
        void Render() override;

    private:
        uint32_t vao{};
        uint32_t vbo{};
        uint32_t ibo{};

        Vortex::Ref<Vortex::IWindow> window;
};



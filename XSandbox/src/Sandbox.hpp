//
// Created by Vitriol1744 on 22.06.2021.
//
#pragma once

#include "Vortex.hpp"

using namespace Vortex;

class Sandbox : public Vortex::Application
{
    public:
        Sandbox();

        void Update() override;
        void Render() override;

        void OnMouseMove(Math::Vec2);

    private:
        Ref<VTGraphics::IMesh> mesh;
        Ref<VTGraphics::IVertexBuffer> vertexBuffer;
        Ref<VTGraphics::IIndexBuffer> indexBuffer;

        Ref<VTGraphics::IShader> shader;

        Ref<IWindow> window;

        Math::Mat4 model;
        Math::Mat4 projection;

        float y{}, y2 = 1;
};



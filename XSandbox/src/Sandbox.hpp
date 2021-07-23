//
// Created by Vitriol1744 on 22.06.2021.
//
#pragma once

#include "Vortex.hpp"

#include "Vortex/Graphics/Sprite.hpp"
#include "Vortex/Graphics/Renderer2D.hpp"

using namespace Vortex;
using namespace VTGraphics;

class Sandbox : public Vortex::Application
{
    public:
        Sandbox();

        void Update() override;
        void Render() override;

        void OnMouseMove(Math::Vec2);

    private:
        const int32 width  = 800;
        const int32 height = 600;
        int spritesCount = 1000;
        Ref<VTGraphics::IVertexArray> mesh1;
        Ref<VTGraphics::IVertexArray> mesh2;
        Ref<VTGraphics::IVertexBuffer> vertexBuffer;
        Ref<VTGraphics::IIndexBuffer> indexBuffer;

        Ref<VTGraphics::IShader> shader1;
        Ref<VTGraphics::IShader> shader2;

        VTGraphics::Sprite sprite;

        Ref<IWindow> window;

        Math::Mat4 model;
        Math::Mat4 view;
        Math::Mat4 projection;

        float y{}, y2 = 1;
        float angle = 0.0f;
};



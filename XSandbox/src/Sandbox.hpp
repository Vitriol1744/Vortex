//
// Created by Vitriol1744 on 22.06.2021.
//
#pragma once

#include <Vortex/Graphics/FPSCameraController.hpp>
#include "Vortex.hpp"

#include "Vortex/Graphics/Sprite.hpp"
#include "Vortex/Graphics/Renderer2D.hpp"
#include "Vortex/Audio/AudioSource.hpp"

#include "Vortex/Graphics/API/ITexture2D.hpp"

using namespace Vortex;
using namespace Vortex::Audio;
using namespace Vortex::Math;
using namespace Vortex::Input;
using namespace Vortex::Graphics;
using namespace VTGraphics;

class Sandbox : public Vortex::Application
{
    public:
        Sandbox(std::vector<char*>& arguments);

        void Initialize() override;
        void Shutdown() override;

        void Update() override;
        void Render() override;

        bool OnMouseMove(IWindow*, Vec2);
        bool OnMouseScroll(IWindow*, Vec2);

    private:
        int32 width  = 800;
        int32 height = 600;
        int spritesCount = 100;

        SoundBuffer buffer{false};
        AudioSource source{false};

        Sprite sprite;

        Ref<IWindow>        window;
        Ref<ITexture>       texture;
        Ref<IVertexArray>   vertexArray;
        Ref<IVertexBuffer>  vertexBuffer;
        Ref<IIndexBuffer>   indexBuffer;
        Ref<IShader>        shader1;
        Ref<IShader>        shader2;

        FPSCameraController cameraController;
};



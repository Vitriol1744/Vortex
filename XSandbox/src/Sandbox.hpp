//
// Created by Vitriol1744 on 22.06.2021.
//
#pragma once

#include <Vortex/Graphics/Camera.hpp>
#include "Vortex.hpp"

#include "Vortex/Graphics/Sprite.hpp"
#include "Vortex/Graphics/Renderer2D.hpp"
#include "Vortex/Audio/AudioSource.hpp"

#include "Vortex/Graphics/API/ITexture2D.hpp"

using namespace Vortex;
using namespace Vortex::Audio;
using namespace VTGraphics;

class Sandbox : public Vortex::Application
{
    public:
        Sandbox(std::vector<char*>& arguments);

        void Initialize() override;
        void Shutdown() override;

        void Update() override;
        void Render() override;

        void OnMouseMove(Math::Vec2);
        void OnMouseScroll(Math::Vec2);

    private:
        int32 width  = 800;
        int32 height = 600;
        int spritesCount = 100;

        SoundBuffer buffer{false};
        AudioSource source{false};

        VTGraphics::Sprite sprite;

        Ref<IWindow> window;
        Ref<ITexture> texture;
        Camera camera;
        glm::vec3 cameraPosition;
        glm::vec3 cameraScale;

        float y{}, y2 = 1;
        float angle = 0.0f;
};



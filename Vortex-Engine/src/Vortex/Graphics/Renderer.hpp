//
// Created by Vitriol1744 on 09.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Graphics/API/IShader.hpp"
#include "Vortex/Graphics/API/IVertexArray.hpp"
#include "Vortex/Graphics/Camera.hpp"

namespace Vortex::Graphics
{
    class VT_API Renderer
    {
        public:
            static void BeginScene(const Camera& camera);
            static void EndScene();

            static void DrawMesh(const Ref<IShader>& shader, const Ref<IVertexArray>& vao, glm::mat4 transform);
    };
}
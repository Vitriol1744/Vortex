//
// Created by Vitriol1744 on 09.08.2021.
//
#include "Renderer.hpp"

#include "Vortex/Graphics/API/IRendererAPI.hpp"

namespace Vortex::Graphics
{
    using namespace Math;
    namespace
    {
        Mat4 viewProjectionMatrix;
    }

    void Renderer::BeginScene(const Camera& camera)
    {
        viewProjectionMatrix = camera.GetViewProjectionMatrix();
    }
    void Renderer::EndScene()
    {
        
    }

    void Renderer::DrawMesh(const Ref<IShader>& shader, const Ref<IVertexArray>& mesh, Math::Mat4 transform)
    {
        shader->Bind();
        shader->SetUniformMat4f("u_ViewProjection", viewProjectionMatrix);
        shader->SetUniformMat4f("u_Model", transform);

        IRendererAPI::DrawIndexed(mesh);
    }
}

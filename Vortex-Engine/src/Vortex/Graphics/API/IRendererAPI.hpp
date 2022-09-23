//
// Created by Vitriol1744 on 08.09.2022.
//
#ifndef VORTEX_IRENDERERAPI_HPP
#define VORTEX_IRENDERERAPI_HPP

#include "Vortex/Core/Core.hpp"
#include "Vortex/Core/Math/Math.hpp"

#include "Vortex/Graphics/API/IVertexBuffer.hpp"
#include "Vortex/Graphics/API/IIndexBuffer.hpp"
#include "Vortex/Graphics/Color.hpp"

namespace Vortex::Graphics
{
    enum class GraphicsAPI
    {
        eNone,
        eVulkan
    };

    inline std::string ToString(GraphicsAPI api)
    {
        switch (api)
        {
            case GraphicsAPI::eNone:     return "None";
            case GraphicsAPI::eVulkan:   return "Vulkan";
        }

        return "Unknown API!";
    }

    class VT_API IRendererAPI
    {
        public:
            IRendererAPI() = default;
            virtual ~IRendererAPI() = default;

            static void Initialize(GraphicsAPI api, std::string_view applicationName, Math::Vec3 applicationVersion);
            static void Shutdown();

            inline static GraphicsAPI GetGraphicsAPI() { return api; }

            inline static void BeginRenderPass() { instance->BeginRenderPassImpl(); }
            inline static void EndRenderPass() { instance->EndRenderPassImpl(); }

            inline static void Draw(Ref<IVertexBuffer> vertexBuffer) { instance->DrawImpl(vertexBuffer); }
            inline static void DrawIndexed(Ref<IVertexBuffer> vertexBuffer, Ref<IIndexBuffer> indexBuffer, uint16 indexCount)
            { instance->DrawIndexedImpl(vertexBuffer, indexBuffer, indexCount); }

            inline static void SetClearColor(Color color) { instance->SetClearColorImpl(color); }
            inline static void SetViewport(const Math::Rectangle& rect) { instance->SetViewportImpl(rect); }

            inline static void Clear() { instance->ClearImpl(); }

        protected:
            static GraphicsAPI api;
            static IRendererAPI* instance;

            Math::Vec3 applicationVersion;

            virtual void InitializeImpl() = 0;

            virtual void BeginRenderPassImpl() = 0;
            virtual void EndRenderPassImpl() = 0;

            virtual void DrawImpl(Ref<IVertexBuffer> vertexBuffer) = 0;
            virtual void DrawIndexedImpl(Ref<IVertexBuffer> vertexBuffer, Ref<IIndexBuffer> indexBuffer, uint16 indexCount) = 0;

            virtual void SetClearColorImpl(Color color) = 0;
            virtual void SetViewportImpl(const Math::Rectangle& rect) = 0;

            virtual void ClearImpl() = 0;
            //virtual void DrawIndexedImpl(const Ref<IVertexArray>& mesh, uint32 indicesCount) = 0;

            std::string applicationName;

    };
}


#endif //VORTEX_IRENDERERAPI_HPP

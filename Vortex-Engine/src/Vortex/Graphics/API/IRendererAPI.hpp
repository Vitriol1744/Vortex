//
// Created by Vitriol1744 on 26.06.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Graphics/API/IVertexArray.hpp"

#undef None

namespace Vortex::Graphics
{
    enum class GraphicsAPI
    {
        eNone,
        eOpenGL46,
        eVulkan
    };

    inline std::string ToString(GraphicsAPI api)
    {
        switch (api)
        {
            case GraphicsAPI::eNone:     return "None";
            case GraphicsAPI::eOpenGL46: return "OpenGL46";
            case GraphicsAPI::eVulkan:   return "Vulkan";
        }

        return "Unknown API!";
    }

    class VT_API IRendererAPI
    {
        public:
            IRendererAPI() = default;
            virtual ~IRendererAPI() { api = GraphicsAPI::eNone; }

            static void Initialize(GraphicsAPI api, std::string_view applicationName, Math::Vec3 applicationVersion);
            static void Shutdown();

            VT_NODISCARD inline static GraphicsAPI GetGraphicsAPI() { return api; }

            inline static void SetClearColor(Math::Vec4 color) { instance->SetClearColorImpl(color); }
            inline static void SetViewport(const Math::Rectangle& rect) { instance->SetViewportImpl(rect); }

            inline static void Clear() { instance->ClearImpl(); }
            inline static void DrawIndexed(const Ref<IVertexArray>& mesh, uint32 indicesCount = 0)
            {
                if (!indicesCount) indicesCount = mesh->GetIndexBuffer()->GetIndicesCount();
                instance->DrawIndexedImpl(mesh, indicesCount);
            }

        protected:
            static GraphicsAPI api;
            static IRendererAPI* instance;

            Math::Vec3 applicationVersion;

            virtual void InitializeImpl() = 0;

            virtual void SetClearColorImpl(Math::Vec4 color) = 0;
            virtual void SetViewportImpl(const Math::Rectangle& rect) = 0;

            virtual void ClearImpl() = 0;
            virtual void DrawIndexedImpl(const Ref<IVertexArray>& mesh, uint32 indicesCount) = 0;

            std::string applicationName;
    };
}
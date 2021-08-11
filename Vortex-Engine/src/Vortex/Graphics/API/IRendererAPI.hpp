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
        None,
        OpenGL46
    };

    class VT_API IRendererAPI
    {
        public:
            IRendererAPI() = default;
            virtual ~IRendererAPI() = default;

            static void Initialize();
            VT_NODISCARD inline static GraphicsAPI GetGraphicsAPI() { return api; }

            inline static void SetClearColor(Math::Vec4 color) { Get()->SetClearColorImpl(color); }
            inline static void SetViewport(const Math::Rectangle& rect) { Get()->SetViewportImpl(rect); }

            inline static void Clear() { Get()->ClearImpl(); }
            inline static void DrawIndexed(const Ref<IVertexArray>& mesh, uint32 indicesCount = 0)
            {
                if (!indicesCount) indicesCount = mesh->GetIndexBuffer()->GetIndicesCount();
                Get()->DrawIndexedImpl(mesh, indicesCount);
            }

        private:
            static Scope<IRendererAPI>& Get();

            virtual void SetClearColorImpl(Math::Vec4 color) = 0;
            virtual void SetViewportImpl(const Math::Rectangle& rect) = 0;

            virtual void ClearImpl() = 0;
            virtual void DrawIndexedImpl(const Ref<IVertexArray>& mesh, uint32 indicesCount) = 0;

            static GraphicsAPI api;

            static Scope<IRendererAPI> CreateRendererAPI();
    };
}
//
// Created by Vitriol1744 on 26.06.2021.
//
#pragma once

#include "Core/Core.hpp"

#include "Graphics/IMesh.hpp"

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
            virtual ~IRendererAPI() = default;

            static Scope<IRendererAPI>& Get();

            IRendererAPI() = default;
            VT_NODISCARD inline static GraphicsAPI GetGraphicsAPI() { return api; }

            virtual void SetClearColor(Math::Vec4 color) = 0;
            virtual void Clear() = 0;

            virtual void DrawIndexed(const Ref<IMesh> mesh, uint32_t indicesCount) = 0;

            static GraphicsAPI api;

            static void Initialize();
            static Scope<IRendererAPI> CreateRendererAPI();
    };
}
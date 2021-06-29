//
// Created by Vitriol1744 on 26.06.2021.
//
#pragma once

#include "Core/Core.hpp"

namespace Vortex
{
    enum class GraphicsAPI
    {
        None,
        OpenGL46
    };

    class VT_API IRendererAPI
    {
        public:
            friend class WindowImpl;

            static Scope<IRendererAPI>& Get();

            IRendererAPI() = default;
            VT_NODISCARD inline static GraphicsAPI GetGraphicsAPI() { return api; }

            virtual void SetClearColor(Vec4 color) = 0;
            virtual void Clear() = 0;

        private:
            static GraphicsAPI api;

            static void Initialize();
            static Scope<IRendererAPI> CreateRendererAPI();
    };
}
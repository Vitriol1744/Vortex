//
// Created by Vitriol1744 on 10.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Graphics/API/IShader.hpp"

#include <unordered_map>

namespace Vortex::Graphics
{
    class VT_API ShaderLibrary
    {
        public:
            Ref<IShader> CreateShader(strview name, strview vertexPath, strview pixelPath, bool precompiled = true);
            Ref<IShader> CreateShader(strview vertexPath, strview pixelPath, bool precompiled = true);

            void AddShader(std::string_view name, Ref<IShader> shader);
            void AddShader(Ref<IShader> shader);

            Ref<IShader> GetShader(strview name);

        private:
            std::unordered_map<HashedID, Ref<IShader>> shaders;
    };
}
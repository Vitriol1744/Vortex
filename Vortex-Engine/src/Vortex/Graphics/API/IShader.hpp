//
// Created by Vitriol1744 on 10.09.2022.
//
#ifndef VORTEX_ISHADER_HPP
#define VORTEX_ISHADER_HPP

#include "Vortex/Core/Core.hpp"

namespace Vortex::Graphics
{
    class VT_API IShader
    {
        public:
            virtual ~IShader() = default;
            virtual const std::string& GetName() const noexcept = 0;

            static Ref<IShader> Create(std::string_view vertexShaderPath, std::string_view fragmentShaderPath, bool binaries = true);
    };
}

#endif //VORTEX_ISHADER_HPP

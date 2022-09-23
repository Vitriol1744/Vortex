//
// Created by Vitriol1744 on 08.09.2022.
//
#ifndef VORTEX_IGRAPHICSCONTEXT_HPP
#define VORTEX_IGRAPHICSCONTEXT_HPP

#include "Vortex/Core/Core.hpp"

namespace Vortex::Graphics
{
    struct VT_API IGraphicsContext
    {
        IGraphicsContext() = default;
        virtual ~IGraphicsContext() = default;

        virtual void Present() = 0;

        virtual void Activate() = 0;
    };
}

#endif //VORTEX_IGRAPHICSCONTEXT_HPP

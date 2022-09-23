//
// Created by Vitriol1744 on 08.09.2022.
//
#ifndef VORTEX_WINDOWMANAGER_HPP
#define VORTEX_WINDOWMANAGER_HPP

#include "Vortex/Core/Core.hpp"

#include "Vortex/Graphics/Window/IWindow.hpp"

#include <utility>
#include <vector>

namespace Vortex
{
    class VT_API WindowManager : public Singleton<WindowManager>
    {
        public:
            WindowManager() = default;
            ~WindowManager() = default;

            inline void Bind(Ref<IWindow> window) { bound_window = window; bound_window->ActivateContext(); }
            VT_NODISCARD inline Ref<IWindow> GetBoundWindow() const { return bound_window; }

            Ref<IWindow> NewWindow(int32 width, int32 height, std::string_view title);

        private:
            Ref<IWindow> bound_window = nullptr;
            std::vector<Ref<IWindow>> windows;
    };
    template class VT_API Singleton<WindowManager>;
}

#endif //VORTEX_WINDOWMANAGER_HPP

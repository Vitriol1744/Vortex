//
// Created by Vitriol1744 on 18.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Core/Input/Input.hpp"

#include "Vortex/Graphics/Window/IWindow.hpp"

namespace Vortex::Graphics
{
    class VT_API ImGuiRenderer
    {
        public:
            static void Initialize(Ref<IWindow> window);
            static void Shutdown();

            static void Begin();
            static void End();

        public:
            static Ref<IWindow> window;

            static bool OnKeyPressedEvent(IWindow*, Input::KeyCode keycode, uint32 repeatCount);
            static bool OnKeyReleasedEvent(IWindow*, Input::KeyCode keycode);
            static bool OnKeyTypedEvent(IWindow*, uint32 character);
            static bool OnMouseButtonPressedEvent(IWindow*, Input::MouseCode mousecode, bool doubleClick);
            static bool OnMouseButtonReleasedEvent(IWindow*, Input::MouseCode mousecode);
            static bool OnMouseScrolledEvent(IWindow*, Math::Vec2 wheel);
            static bool OnMouseMovedEvent(IWindow*, Math::Vec2 position);
            static bool OnWindowClosedEvent(IWindow*);
            static bool OnWindowResizedEvent(IWindow*, Math::Vec2u size);
            static bool OnWindowMovedEvent(IWindow*, Math::Vec2 position);
    };
}
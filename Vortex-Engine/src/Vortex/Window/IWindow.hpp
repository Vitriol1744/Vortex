//
// Created by Vitriol1744 on 22.06.2021.
//
#pragma once

#include "Core/Core.hpp"
#include "Core/IEvent.hpp"

#include "Input/KeyCode.hpp"
#include "Input/MouseCode.hpp"

#include "Graphics/IGraphicsContext.hpp"

#include <string>

#undef CreateWindow

namespace Vortex
{
    struct KeyPressedEventData
    {
        Input::KeyCode keyCode;
        uint32 repeatCount;
        Math::Vec2 position;
    };

    class VT_API IWindow : public NonCopyable<IWindow>
    {
        public:
            friend class WindowManager;

            IWindow() = default;
            virtual ~IWindow() = default;

            virtual void Update() = 0;
            virtual void Present() = 0;

            virtual Scope<Graphics::IGraphicsContext>& GetGraphicsContext() = 0;

            VT_NODISCARD virtual bool IsOpen() const noexcept = 0;
            VT_NODISCARD virtual bool IsKeyPressed(Input::KeyCode keycode) const noexcept = 0;
            VT_NODISCARD virtual bool IsMouseButtonPressed(Input::MouseCode mousecode) const noexcept = 0;

            virtual void HideCursor() const noexcept = 0;
            virtual void SetTitle(std::string_view title) const noexcept = 0;
            virtual void SetTitle(std::wstring_view title) const noexcept = 0;

            VT_NODISCARD static Ref<IWindow> CreateWindow(int width, int height, std::wstring_view title, Ref<IWindow> share = nullptr);

            // Window Specific Events
            // Vortex::KeyPressedEventData data
            Event<KeyPressedEventData> keyPressedEvent;
            // Vortex::Input::KeyCode keycode, Vortex::Math::Vec2 mousePosition
            Event<Input::KeyCode, Math::Vec2> keyReleasedEvent;
            // uint32 character
            Event<uint32> keyTypedEvent;

            // Vortex::Input::MouseCode mouseCode, Vortex::Math::Vec2 mousePosition
            Event<Input::MouseCode, Math::Vec2> mouseButtonPressedEvent;
            // Vortex::Input::MouseCode mouseCode, Vortex::Math::Vec2 mousePosition
            Event<Input::MouseCode, Math::Vec2> mouseButtonReleasedEvent;
            // Vortex::Math::Vec2 wheel, Vortex::Math::Vec2 mousePosition
            Event<Math::Vec2, Math::Vec2> mouseScrolledEvent;
            Event<Math::Vec2> mouseMovedEvent;

        private:
            virtual void ActivateContext() const = 0;
    };
}



/*
 * Created by v1tr10l7 on 25.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/NonCopyable.hpp>
#include <Prism/String/String.hpp>

#include <Vortex/Core/Events/Event.hpp>
#include <Vortex/Core/Math/Rectangle.hpp>
#include <Vortex/Core/Math/Vector.hpp>

namespace Vortex
{
    struct VideoMode
    {
        VideoMode() = default;
        VideoMode(i32 width, i32 height)
            : Width(width)
            , Height(height)
        {
        }

        auto operator<=>(const VideoMode& other) const = default;

        i32  Width                                     = 800;
        i32  Height                                    = 600;
        i32  RedBits                                   = 8;
        i32  GreenBits                                 = 8;
        i32  BlueBits                                  = 8;
        i32  RefreshRate                               = 60;
    };

    struct GammaRamp
    {
        GammaRamp()
        {
            Red.Resize(256);
            Green.Resize(256);
            Blue.Resize(256);
        }

        auto        operator<=>(const GammaRamp& other) const = default;

        Vector<u16> Red;
        Vector<u16> Green;
        Vector<u16> Blue;
    };

    enum class MonitorState
    {
        eConnected,
        eDisconnected,
        eLowPower,
        eScreenSaver,
    };

    class VT_API Monitor : public NonCopyable<Monitor>, public RefCounted
    {
      public:
        virtual ~Monitor()                                  = default;

        virtual std::any                     NativeHandle() = 0;

        static ::Ref<Monitor>                PrimaryMonitor();
        static const Vector<::Ref<Monitor>>& Monitors();

        inline StringView                    Name() const { return m_Name; }
        inline const VideoMode&              CurrentVideoMode() const
        {
            return m_CurrentVideoMode;
        }
        virtual Math::Vec2i Position() const = 0;
        virtual Rectangle<> WorkArea() const = 0;
        inline Math::Vec2i  PhysicalSize() const
        {
            return {m_PhysicalWidth, m_PhysicalHeight};
        }
        virtual Math::Vec2f ContentScale() const = 0;

        /**
         * @brief Acquires the monitor's physical width
         * @return Monitor's physical width in millimeters
         */
        inline i32          PhysicalWidth() const { return m_PhysicalWidth; }
        /**
         * @brief Acquires the monitor's physical height
         * @return Monitor's physical height in millimeters
         */
        inline i32          PhysicalHeight() const { return m_PhysicalHeight; }

        inline const Vector<VideoMode>& VideoModes() const
        {
            return m_VideoModes;
        }

        inline const GammaRamp& GammaRamp() const { return m_CurrentRamp; }

        virtual void            SetGamma(f32 gamma) const                 = 0;
        virtual void            SetGammaRamp(struct GammaRamp& gammaRamp) = 0;

      protected:
        Monitor()                            = default;

        String                        m_Name = "Unknown";
        VideoMode                     m_CurrentVideoMode;

        // Monitor's physical dimensions in millimeters
        i32                           m_PhysicalWidth  = 0;
        i32                           m_PhysicalHeight = 0;

        Vector<VideoMode>             m_VideoModes;
        struct GammaRamp              m_OriginalRamp;
        struct GammaRamp              m_CurrentRamp;

        static Vector<::Ref<Monitor>> s_Monitors;

      private:
        static void Initialize();
    };

    namespace MonitorEvents
    {
        // monitor, monitorState
        extern Event<Monitor*, MonitorState> MonitorStateChangedEvent;
    }; // namespace MonitorEvents
}; // namespace Vortex

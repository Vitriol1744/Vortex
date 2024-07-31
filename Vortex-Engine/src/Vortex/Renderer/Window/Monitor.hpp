/*
 * Created by v1tr10l7 on 25.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Events/Event.hpp"
#include "Vortex/Core/Math/Vector.hpp"
#include "Vortex/Core/NonCopyable.hpp"

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
            Red.resize(256);
            Green.resize(256);
            Blue.resize(256);
        }

        auto             operator<=>(const GammaRamp& other) const = default;

        std::vector<u16> Red;
        std::vector<u16> Green;
        std::vector<u16> Blue;
    };

    enum class MonitorState
    {
        eConnected,
        eDisconnected,
        eLowPower,
        eScreenSaver,
    };

    class VT_API Monitor : public NonCopyable<Monitor>
    {
      public:
        virtual ~Monitor()                                        = default;

        virtual std::any                        GetNativeHandle() = 0;

        static Ref<Monitor>                     GetPrimaryMonitor();
        static const std::vector<Ref<Monitor>>& GetMonitors();

        inline std::string_view GetName() const { return m_Name; }
        inline const VideoMode& GetCurrentVideoMode() const
        {
            return m_CurrentVideoMode;
        }
        virtual Vec2i GetPosition() const = 0;
        virtual Vec4i GetWorkArea() const = 0;
        inline Vec2i  GetPhysicalSize() const
        {
            return {m_PhysicalWidth, m_PhysicalHeight};
        }
        virtual Vec2f GetContentScale() const = 0;

        /**
         * @brief Acquires the monitor's physical width
         * @return Monitor's physical width in millimeters
         */
        inline i32    GetPhysicalWidth() const { return m_PhysicalWidth; }
        /**
         * @brief Acquires the monitor's physical height
         * @return Monitor's physical height in millimeters
         */
        inline i32    GetPhysicalHeight() const { return m_PhysicalHeight; }

        inline const std::vector<VideoMode>& GetVideoModes() const
        {
            return m_VideoModes;
        }

        inline const GammaRamp& GetGammaRamp() const { return m_CurrentRamp; }

        virtual void            SetGamma(f32 gamma) const          = 0;
        virtual void            SetGammaRamp(GammaRamp& gammaRamp) = 0;

      protected:
        Monitor()                     = default;

        std::string            m_Name = "Unknown";
        VideoMode              m_CurrentVideoMode;

        // Monitor's physical dimensions in millimeters
        i32                    m_PhysicalWidth  = 0;
        i32                    m_PhysicalHeight = 0;

        std::vector<VideoMode> m_VideoModes;
        GammaRamp              m_OriginalRamp;
        GammaRamp              m_CurrentRamp;
    };

    namespace MonitorEvents
    {
        // monitor, monitorState
        extern Event<Monitor*, MonitorState> MonitorStateChangedEvent;

    }; // namespace MonitorEvents
}; // namespace Vortex

/*
 * Created by v1tr10l7 on 24.10.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <vector>
#include <memory>
#include <filesystem>

#include <Coral/Array.hpp>
#include <Coral/Attribute.hpp>
#include <Coral/GC.hpp>
#include <Coral/HostInstance.hpp>
#include <Coral/String.hpp>
#include <Coral/TypeCache.hpp>
#include <Vortex/Core/Core.hpp>
#include <Vortex/Script/Glue/ScriptGlue.hpp>
#include <Vortex/Window/Input/Keyboard.hpp>

namespace Vortex
{
    namespace ScriptEngine::ScriptGlue
    {
        using Log::LogLevel;
        void Log_LogMessage(LogLevel level, Coral::String message)
        {
            std::string_view msgView = message.Data();

            switch (level)
            {
                case LogLevel::eDebug:
                    VtDebugTag("Managed", "{}", msgView);
                    break;
                case LogLevel::eTrace:
                    VtTraceTag("Managed", "{}", msgView);
                    break;
                case LogLevel::eInfo:
                    VtInfoTag("Managed", "{}", msgView);
                    break;
                case LogLevel::eWarn:
                    VtWarnTag("Managed", "{}", msgView);
                    break;
                case LogLevel::eError:
                    VtErrorTag("Managed", "{}", msgView);
                    break;
                case LogLevel::eFatal:
                    VtFatalTag("Managed", "{}", msgView);
                    break;
                default: VtCoreInfo("[Managed] {}", msgView); break;
            }
        }

        void Input_IsKeyPressed(i16 keycode, bool* outIsPressed)
        {
            *outIsPressed
                = Keyboard::IsKeyPressed(static_cast<Input::KeyCode>(keycode));
        }

        void Input_IsKeyReleased(i16 keycode, bool* outIsPressed)
        {
            *outIsPressed
                = !Keyboard::IsKeyPressed(static_cast<Input::KeyCode>(keycode));
        }

        void RegisterAll(Coral::ManagedAssembly& coreAssembly)
        {
            VtCoreTrace("Registering internal calls for assembly: {}",
                        coreAssembly.GetName().data());

            coreAssembly.AddInternalCall(
                "Vortex.InternalCalls", "Log_LogMessage",
                reinterpret_cast<void*>(&Log_LogMessage));
            coreAssembly.AddInternalCall(
                "Vortex.InternalCalls", "Input_IsKeyPressed",
                reinterpret_cast<void*>(&Input_IsKeyPressed));
            coreAssembly.AddInternalCall(
                "Vortex.InternalCalls", "Input_IsKeyReleased",
                reinterpret_cast<void*>(&Input_IsKeyReleased));

            VtCoreTrace("Registered internal calls for assembly: {}",
                        coreAssembly.GetName().data());
        }
    }; // namespace ScriptEngine::ScriptGlue
}; // namespace Vortex

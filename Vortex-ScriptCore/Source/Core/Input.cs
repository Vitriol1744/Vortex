/*
 * Created by v1tr10l7 on 13.10.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
namespace Vortex
{
    public enum CursorMode
    {
        Normal = 0,
        Hidden = 1,
        Locked = 2
    }

    public static class Input
    {
        public static bool IsKeyPressed(KeyCode keycode)
        {
            bool result = false;
            unsafe { InternalCalls.Input_IsKeyPressed(keycode, &result); }
            return result;
        }

        public static bool IsKeyHeld(KeyCode keycode)
        {
            unsafe { return InternalCalls.Input_IsKeyHeld(keycode); }
        }

        public static bool IsKeyDown(KeyCode keycode)
        {
            unsafe { return InternalCalls.Input_IsKeyDown(keycode); }
        }

        public static bool IsKeyReleased(KeyCode keycode)
        {
            bool result = false;
            unsafe { InternalCalls.Input_IsKeyReleased(keycode, &result); }
            return result;
        }

        public static bool IsMouseButtonPressed(MouseCode button)
        {
            unsafe { return InternalCalls.Input_IsMouseButtonPressed(button); }
        }

        public static bool IsMouseButtonHeld(MouseCode button)
        {
            unsafe { return InternalCalls.Input_IsMouseButtonHeld(button); }
        }

        public static bool IsMouseButtonDown(MouseCode button)
        {
            unsafe { return InternalCalls.Input_IsMouseButtonDown(button); }
        }

        public static bool IsMouseButtonReleased(MouseCode button)
        {
            unsafe { return InternalCalls.Input_IsMouseButtonReleased(button); }
        }

        public static Vector2 GetMousePosition()
        {
            Vector2 position;
            unsafe { InternalCalls.Input_GetMousePosition(&position); }
            return position;
        }

        public static void SetCursorMode(CursorMode mode)
        {
            unsafe { InternalCalls.Input_SetCursorMode(mode); }
        }

        public static CursorMode GetCursorMode()
        {
            unsafe { return InternalCalls.Input_GetCursorMode(); }
        }
    }
}

//
// Created by Vitriol1744 on 03.08.2021.
//
#include "Vortex/Core/Platform.hpp"

#ifdef VT_PLATFORM_WINDOWS
#include "Win32Gamepad.hpp"

#pragma comment(lib, "XInput.lib")
#define VTP_BUTTON(button) static_cast<bool>(buttons & button)

namespace Vortex::Input
{
    static DWORD packetNumber = 0;

    float32 GamepadImpl::IsButtonPressedImpl(uint8 index, GamepadButton button) const noexcept
    {
        WORD buttons = controllers[index].state.Gamepad.wButtons;

        switch (button)
        {
            case GamepadButton::Cross:      return VTP_BUTTON(XINPUT_GAMEPAD_A              );
            case GamepadButton::Square:     return VTP_BUTTON(XINPUT_GAMEPAD_X              );
            case GamepadButton::Triangle:   return VTP_BUTTON(XINPUT_GAMEPAD_Y              );
            case GamepadButton::Circle:     return VTP_BUTTON(XINPUT_GAMEPAD_B              );
            case GamepadButton::L1:         return VTP_BUTTON(XINPUT_GAMEPAD_LEFT_SHOULDER  );
            case GamepadButton::R1:         return VTP_BUTTON(XINPUT_GAMEPAD_RIGHT_SHOULDER );
            case GamepadButton::L2:         return           (controllers[index].L2         );
            case GamepadButton::R2:         return           (controllers[index].R2         );
            case GamepadButton::L3:         return VTP_BUTTON(XINPUT_GAMEPAD_LEFT_THUMB     );
            case GamepadButton::R3:         return VTP_BUTTON(XINPUT_GAMEPAD_RIGHT_THUMB    );
            case GamepadButton::Select:     return VTP_BUTTON(XINPUT_GAMEPAD_START          );
            case GamepadButton::Start:      return VTP_BUTTON(XINPUT_GAMEPAD_BACK           );
            case GamepadButton::Up:         return VTP_BUTTON(XINPUT_GAMEPAD_DPAD_UP        );
            case GamepadButton::Down:       return VTP_BUTTON(XINPUT_GAMEPAD_DPAD_DOWN      );
            case GamepadButton::Left:       return VTP_BUTTON(XINPUT_GAMEPAD_DPAD_LEFT      );
            case GamepadButton::Right:      return VTP_BUTTON(XINPUT_GAMEPAD_DPAD_RIGHT     );
        }
    }
    float32 GamepadImpl::AxisPositionImpl(uint8 index, Axis axis)
    {
        const Controller& controller = controllers[index];
        switch (axis)
        {
            case Axis::LX: return controller.LX;
            case Axis::LY: return controller.LY;
            case Axis::RX: return controller.RX;
            case Axis::RY: return controller.RY;
        }
    }

    void GamepadImpl::RumbleImpl(uint8 index, uint16 leftMotorSpeed, uint16 rightMotorSpeed)
    {
        XINPUT_VIBRATION vibration = {};
        vibration.wLeftMotorSpeed   = leftMotorSpeed;
        vibration.wRightMotorSpeed  = rightMotorSpeed;

        XInputSetState(index, &vibration);
    }

    void GamepadImpl::PollInputImpl()
	{
        DWORD errorCode;
        for (DWORD index = 0; index < XUSER_MAX_COUNT; index++)
        {
            XINPUT_STATE state = {};
            XINPUT_GAMEPAD xgamepad = state.Gamepad;

            errorCode = XInputGetState(index, &state);

            if (errorCode == ERROR_SUCCESS)
            {
                // Controller is connected
                controllers[index].isConnected = true;
                VT_CORE_LOG_INFO("Controller is Connected!");

                // State Changed?
                if (state.dwPacketNumber != packetNumber)
                {
                    packetNumber = state.dwPacketNumber;
                    controllers[index].state = state;

                    controllers[index].L2 = xgamepad.bLeftTrigger  > XINPUT_GAMEPAD_TRIGGER_THRESHOLD ? xgamepad.bLeftTrigger  / 255.0f : 0.0f;
                    controllers[index].R2 = xgamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD ? xgamepad.bRightTrigger / 255.0f : 0.0f;

                    UpdateThumbsticks(index, xgamepad);
                }
            }
            else if (errorCode == ERROR_DEVICE_NOT_CONNECTED)
            {
                // Controller is not connected
                controllers[index].isConnected = false;
            }
            else VT_CORE_LOG_ERROR("XInput: Failed to Retrieve State of Gamepad at index: {}", index);
        }
	}

    void GamepadImpl::UpdateThumbsticks(uint8 index, XINPUT_GAMEPAD gamepadState)
    {
        Controller& controller = controllers[index];

        float normLX = fmaxf(-1, (float)gamepadState.sThumbLX / 32767);
        float normLY = fmaxf(-1, (float)gamepadState.sThumbLY / 32767);

        controller.LX = (abs(normLX) < controller.deadzoneX ? 0 : (abs(normLX) - controller.deadzoneX) * (normLX / abs(normLX)));
        controller.LY = (abs(normLY) < controller.deadzoneY ? 0 : (abs(normLY) - controller.deadzoneY) * (normLY / abs(normLY)));

        if (controller.deadzoneX > 0) controller.LX *= 1 / (1 - controller.deadzoneX);
        if (controller.deadzoneY > 0) controller.LY *= 1 / (1 - controller.deadzoneY);

        float normRX = fmaxf(-1, (float)gamepadState.sThumbRX / 32767);
        float normRY = fmaxf(-1, (float)gamepadState.sThumbRY / 32767);

        controller.RX = (abs(normRX) < controller.deadzoneX ? 0 : (abs(normRX) - controller.deadzoneX) * (normRX / abs(normRX)));
        controller.RY = (abs(normRY) < controller.deadzoneY ? 0 : (abs(normRY) - controller.deadzoneY) * (normRY / abs(normRY)));

        if (controller.deadzoneX > 0) controller.RX *= 1 / (1 - controller.deadzoneX);
        if (controller.deadzoneY > 0) controller.RY *= 1 / (1 - controller.deadzoneY);
    }
}
#endif
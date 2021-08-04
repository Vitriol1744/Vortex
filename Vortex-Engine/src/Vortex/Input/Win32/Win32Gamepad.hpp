//
// Created by Vitriol1744 on 03.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Input/Gamepad.hpp"

#include <Windows.h>
#include <Xinput.h>

namespace Vortex::Input
{
	struct Controller
	{
		XINPUT_STATE state = {};
		float32 L2 = 0.0f;
		float32 R2 = 0.0f;
		float32 LX = 0.0f;
		float32 LY = 0.0f;
		float32 RX = 0.0f;
		float32 RY = 0.0f;

		float32 deadzoneX = 0.5f;
		float32 deadzoneY = 0.2f;

		bool isConnected = false;
	};

	class VT_API GamepadImpl final : public Gamepad
	{
		public:
			inline GamepadImpl() {  }
			
			virtual inline bool IsConnectedImpl(uint8 index) const noexcept override { return controllers[index].isConnected; }
			virtual float32 IsButtonPressedImpl(uint8 index, GamepadButton button) const noexcept override;
			virtual float32 AxisPositionImpl(uint8 index, Axis axis) override;

			virtual void SetDeadzoneImpl(uint8 index, float32 x, float32 y) noexcept override { controllers[index].deadzoneX = x; controllers[index].deadzoneY = y; }
			virtual void RumbleImpl(uint8 index, uint16 leftMotorSpeed, uint16 rightMotorSpeed) override;

			virtual void PollInputImpl() override;

		private:
			Controller controllers[XUSER_MAX_COUNT];

			void UpdateThumbsticks(uint8 index, XINPUT_GAMEPAD gamepadState);
	};
}
//
// Created by Vitriol1744 on 03.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"
#include "Vortex/Core/Event.hpp"

namespace Vortex::Input
{
	enum class GamepadButton : uint8
	{
		Cross,
		Square,
		Triangle,
		Circle,
		L1,
		R1,
		L2,
		R2,
		L3,
		R3,
		Select,
		Start,
		Up,
		Down,
		Left,
		Right,
		ButtonsCount,

		A = Cross,
		X = Square,
		Y = Triangle,
		B = Circle,
		LB = L1,
		RB = R1,
		LT = L2,
		RT = R2,
		LS = L3,
		RS = R3,
		View = Start,
		Menu = Select
	};

	enum class Axis
	{
		LX,
		RX,
		LY,
		RY
	};

	class VT_API Gamepad
	{
		public:
			virtual ~Gamepad() = default;

			inline static bool IsConnected(uint8 index) noexcept { return Instance()->IsConnectedImpl(index); }
			inline static float32 IsButtonPressed(uint8 index, GamepadButton button) noexcept { return Instance()->IsButtonPressedImpl(index, button); }
			inline static float32 AxisPosition(uint8 index, Axis axis) { return Instance()->AxisPositionImpl(index, axis); }

			inline static void SetDeadzone(uint8 index, float32 x, float32 y) noexcept { return Instance()->SetDeadzoneImpl(index, x, y); }
			inline static void Rumble(uint8 index, uint16 leftMotorSpeed = 32000, uint16 rightMotorSpeed = 16000)
			{ 
				Instance()->RumbleImpl(index, leftMotorSpeed, rightMotorSpeed); 
			}

			inline static void PollInput() { Instance()->PollInputImpl(); }

			static Event<GamepadButton> buttonPressedEvent;
			static Event<GamepadButton> buttonReleasedEvent;

		protected:
			virtual bool IsConnectedImpl(uint8 index) const noexcept = 0;
			virtual float32 IsButtonPressedImpl(uint8 index, GamepadButton button) const noexcept = 0;
			virtual float32 AxisPositionImpl(uint8 index, Axis axis) = 0;
			
			virtual void SetDeadzoneImpl(uint8 index, float32 x, float32 y) noexcept = 0;
			virtual void RumbleImpl(uint8 index, uint16 leftMotorSpeed, uint16 rightMotorSpeed) = 0;

			virtual void PollInputImpl() = 0;

		private:
			static Gamepad* Instance();
	};
}
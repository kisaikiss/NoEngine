#include "stdafx.h"
#include "InputConfig.h"
#include "Input.h"


namespace NoEngine {
namespace Input {
void InputBindAction(const std::string& actionName, Input::DeviceType device, int code) {
	InputConfig::Get().BindAction(actionName, device, code);
}

void InputBindAxis(const std::string& axisName, DeviceType device, int code, float scale, float deadZone) {
	InputConfig::Get().BindAxis(axisName, device, code, scale, deadZone);
}

bool InputIsTrigger(const std::string& actionName) {
	return InputConfig::Get().IsActionTriggered(actionName);
}

bool InputIsRelease(const std::string& actionName) {
	return InputConfig::Get().IsActionReleased(actionName);
}

bool InputIsPress(const std::string& actionName) {
	return InputConfig::Get().IsActionPressed(actionName);
}

float GetInputAxisValue(const std::string& axisName) {
	return InputConfig::Get().GetAxisValue(axisName);
}

float GetInputPreAxisValue(const std::string& axisName) {
	return InputConfig::Get().GetPreAxisValue(axisName);
}

}

using namespace Input;
void InputConfig::BindAction(const std::string& actionName, DeviceType device, int code) {
	actionMap_[actionName].inputs.push_back({ device,code });
}

void InputConfig::BindAxis(const std::string& axisName, Input::DeviceType device, int code, float scale, float deadZone) {
	axisMap_[axisName].inputs.push_back({ device,code,scale,deadZone });
}

bool InputConfig::IsActionTriggered(const std::string& actionName) {
	for (auto input : actionMap_[actionName].inputs) {
		if (CheckPhysicalTrigger(input)) return true;
	}
	return false;
}

bool InputConfig::IsActionReleased(const std::string& actionName) {
	for (auto input : actionMap_[actionName].inputs) {
		if (CheckPhysicalRelease(input)) return true;
	}
	return false;
}

bool InputConfig::IsActionPressed(const std::string& actionName) {
	for (auto input : actionMap_[actionName].inputs) {
		if (CheckPhysicalPress(input)) return true;
	}
	return false;
}

float InputConfig::GetAxisValue(const std::string& axisName) {
	auto it = axisMap_.find(axisName);
	if (it == axisMap_.end()) return 0.0f;

	float totalValue = 0.0f;

	// 登録されているすべての入力を評価して合計する
	for (const auto& input : it->second.inputs) {
		totalValue += GetPhysicalAxis(input);
	}

	// 必要に応じて -1.0f 〜 1.0f の範囲にクランプ（制限）する
	return std::max(-1.0f, std::min(1.0f, totalValue));
}

float InputConfig::GetPreAxisValue(const std::string& axisName) {
	auto it = axisMap_.find(axisName);
	if (it == axisMap_.end()) return 0.0f;

	float totalValue = 0.0f;

	// 登録されているすべての入力を評価して合計する
	for (const auto& input : it->second.inputs) {
		totalValue += GetPrePhysicalAxis(input);
	}

	// 必要に応じて -1.0f 〜 1.0f の範囲にクランプ（制限）する
	return std::max(-1.0f, std::min(1.0f, totalValue));
}

bool InputConfig::CheckPhysicalTrigger(const PhysicalInput& bind) {
	switch (bind.device) {
	case DeviceType::Keyboard: return Keyboard::IsTrigger(static_cast<uint8_t>(bind.code));
	case DeviceType::GamepadButton:  return Pad::IsTrigger(static_cast<GamepadButton>(bind.code));
	case DeviceType::Mouse:    return Mouse::IsTrigger(static_cast<MouseButton>(bind.code));
	}
	return false;
}

bool InputConfig::CheckPhysicalRelease(const PhysicalInput& bind) {
	switch (bind.device) {
	case DeviceType::Keyboard: return Keyboard::IsRelease(static_cast<uint8_t>(bind.code));
	case DeviceType::GamepadButton:  return Pad::IsRelease(static_cast<GamepadButton>(bind.code));
	case DeviceType::Mouse:    return Mouse::IsRelease(static_cast<MouseButton>(bind.code));
	}
	return false;
}

bool InputConfig::CheckPhysicalPress(const PhysicalInput& bind) {
	switch (bind.device) {
	case DeviceType::Keyboard: return Keyboard::IsPress(static_cast<uint8_t>(bind.code));
	case DeviceType::GamepadButton:  return Pad::IsPress(static_cast<GamepadButton>(bind.code));
	case DeviceType::Mouse:    return Mouse::IsPress(static_cast<MouseButton>(bind.code));
	}
	return false;
}

float InputConfig::GetPhysicalAxis(const PhysicalInput& bind) {
	if (!Pad::IsGamepadConnected()) {
		// パッドが繋がっていないなら、パッド関連の入力はすべて0
		if (bind.device == DeviceType::GamepadButton || bind.device == DeviceType::GamepadAxis) {
			return 0.0f;
		}
	}

	switch (bind.device) {
	case DeviceType::Keyboard: 
		return static_cast<float>(Keyboard::IsPress(static_cast<uint8_t>(bind.code))) * bind.scale;
	case DeviceType::GamepadButton: 
		return static_cast<float>(Pad::IsPress(static_cast<GamepadButton>(bind.code))) * bind.scale;
	case DeviceType::GamepadAxis: {
		GamepadAxis axisType = static_cast<GamepadAxis>(bind.code);
		float rawValue = 0.0f;
		switch (axisType) {
		case NoEngine::Input::GamepadAxis::LeftStickX:		 rawValue = Pad::GetStick().leftStickX * bind.scale;			 break;
		case NoEngine::Input::GamepadAxis::LeftStickY:		 rawValue = Pad::GetStick().leftStickY * bind.scale;			 break;
		case NoEngine::Input::GamepadAxis::RightStickX:		 rawValue = Pad::GetStick().rightStickX * bind.scale;			 break;
		case NoEngine::Input::GamepadAxis::RightStickY:		 rawValue = Pad::GetStick().rightStickY * bind.scale;			 break;
		case NoEngine::Input::GamepadAxis::LeftTrigger:		 rawValue = Pad::GetTriggerButton().leftTrigger * bind.scale;	 break;
		case NoEngine::Input::GamepadAxis::RightTrigger:	 rawValue = Pad::GetTriggerButton().rightTrigger * bind.scale;	 break;
		}
		float processedValue = ApplyDeadZone(rawValue, bind.deadZone);
		return processedValue * bind.scale;

		break;
	}
	case DeviceType::Mouse:    return static_cast<float>(Mouse::IsPress(static_cast<MouseButton>(bind.code))) * bind.scale;
	}

	return 0.0f;
}
float InputConfig::GetPrePhysicalAxis(const PhysicalInput& bind) {
	if (!Pad::IsGamepadConnected()) {
		// パッドが繋がっていないなら、パッド関連の入力はすべて0
		if (bind.device == DeviceType::GamepadButton || bind.device == DeviceType::GamepadAxis) {
			return 0.0f;
		}
	}

	switch (bind.device) {
	case DeviceType::Keyboard:
		return static_cast<float>(Keyboard::PreIsPress(static_cast<uint8_t>(bind.code))) * bind.scale;
	case DeviceType::GamepadButton:
		return static_cast<float>(Pad::PreIsPress(static_cast<GamepadButton>(bind.code))) * bind.scale;
	case DeviceType::GamepadAxis: {
		GamepadAxis axisType = static_cast<GamepadAxis>(bind.code);
		float rawValue = 0.0f;
		switch (axisType) {
		case NoEngine::Input::GamepadAxis::LeftStickX:		 rawValue = Pad::GetPreStick().leftStickX * bind.scale;			  break;
		case NoEngine::Input::GamepadAxis::LeftStickY:		 rawValue = Pad::GetPreStick().leftStickY * bind.scale;			  break;
		case NoEngine::Input::GamepadAxis::RightStickX:		 rawValue = Pad::GetPreStick().rightStickX * bind.scale;		  break;
		case NoEngine::Input::GamepadAxis::RightStickY:		 rawValue = Pad::GetPreStick().rightStickY * bind.scale;		  break;
		case NoEngine::Input::GamepadAxis::LeftTrigger:		 rawValue = Pad::GetPreTriggerButton().leftTrigger * bind.scale;  break;
		case NoEngine::Input::GamepadAxis::RightTrigger:	 rawValue = Pad::GetPreTriggerButton().rightTrigger * bind.scale; break;
		}
		float processedValue = ApplyDeadZone(rawValue, bind.deadZone);
		return processedValue * bind.scale;

		break;
	}
	case DeviceType::Mouse:    return static_cast<float>(Mouse::PreIsPress(static_cast<MouseButton>(bind.code))) * bind.scale;
	}
	return 0.0f;
}

float InputConfig::ApplyDeadZone(float value, float deadZone) {
	float absValue = std::abs(value);

	// 閾値未満なら完全に0にする
	if (absValue < deadZone) return 0.0f;

	// デッドゾーンの終わり(0.0)から最大値(1.0)までを綺麗に補間する
	float sign = (value > 0.0f) ? 1.0f : -1.0f;
	return sign * (absValue - deadZone) / (1.0f - deadZone);
}
}
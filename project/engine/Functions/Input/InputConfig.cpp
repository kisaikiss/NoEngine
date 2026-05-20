#include "stdafx.h"
#include "InputConfig.h"
#include "Input.h"


namespace NoEngine {
namespace Input {
void InputBindAction(const std::string& actionName, Input::DeviceType device, int code) {
	InputConfig::Get().BindAction(actionName, device, code);
}

bool InputIsTrigger(const std::string& actionName) {
	return InputConfig::Get().IsActionTriggered(actionName);
}

bool InputIsPress(const std::string& actionName) {
	return InputConfig::Get().IsActionPressed(actionName);
}

}

using namespace Input;
void InputConfig::BindAction(const std::string& actionName, DeviceType device, int code) {
	actionMap[actionName].inputs.push_back({ device,code });
}

bool InputConfig::IsActionTriggered(const std::string& actionName) {
	for (auto input : actionMap[actionName].inputs) {
		if (CheckPhysicalTrigger(input)) return true;
	}
	return false;
}

bool InputConfig::IsActionPressed(const std::string& actionName) {
	for (auto input : actionMap[actionName].inputs) {
		if (CheckPhysicalPress(input)) return true;
	}
	return false;
}

bool InputConfig::CheckPhysicalTrigger(const PhysicalInput& bind) {
	switch (bind.device) {
	case DeviceType::Keyboard: return Keyboard::IsTrigger(static_cast<uint8_t>(bind.code));
	case DeviceType::Gamepad:  return Pad::IsTrigger(static_cast<GamepadButton>(bind.code));
	case DeviceType::Mouse:    return Mouse::IsTrigger(static_cast<MouseButton>(bind.code));
	}
	return false;
}

bool InputConfig::CheckPhysicalPress(const PhysicalInput& bind) {
	switch (bind.device) {
	case DeviceType::Keyboard: return Keyboard::IsPress(static_cast<uint8_t>(bind.code));
	case DeviceType::Gamepad:  return Pad::IsPress(static_cast<GamepadButton>(bind.code));
	case DeviceType::Mouse:    return Mouse::IsPress(static_cast<MouseButton>(bind.code));
	}
	return false;
}
}
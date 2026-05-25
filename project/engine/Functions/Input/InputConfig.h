#pragma once
namespace NoEngine {
namespace Input {
enum class DeviceType {
    Keyboard,
    Gamepad,
    Mouse
};

void InputBindAction(const std::string& actionName, Input::DeviceType device, int code);

bool InputIsTrigger(const std::string& actionName);
bool InputIsRelease(const std::string& actionName);
bool InputIsPress(const std::string& actionName);

}

struct PhysicalInput {
    Input::DeviceType device;
    int code; // KeyboardならkeyCode, PadならGamepadButton, MouseならMouseButtonをキャストして保持
};

// 1つのアクション（例："MoveForward"）に対する割り当てリスト
struct ActionBinding {
    std::vector<PhysicalInput> inputs;
};

class InputConfig {
public:
    InputConfig(const InputConfig& obj) = delete;
    InputConfig& operator=(const InputConfig& obj) = delete;

    static InputConfig& Get() { 
        static InputConfig inputConfig;
        return inputConfig;
    }

    void BindAction(const std::string& actionName, Input::DeviceType device, int code);

    bool IsActionTriggered(const std::string& actionName);
    bool IsActionReleased(const std::string& actionName);
    bool IsActionPressed(const std::string& actionName);
private:
    std::unordered_map<std::string, ActionBinding> actionMap;

    bool CheckPhysicalTrigger(const PhysicalInput& bind);
    bool CheckPhysicalRelease(const PhysicalInput& bind);
    bool CheckPhysicalPress(const PhysicalInput& bind);

    InputConfig() = default;
    ~InputConfig() = default;
};
}

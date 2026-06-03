#pragma once
namespace NoEngine {
namespace Input {
enum class DeviceType {
    Keyboard,
    GamepadButton,
    GamepadAxis,
    Mouse
};

enum class GamepadAxis {
    LeftStickX,
    LeftStickY,
    RightStickX,
    RightStickY,
    LeftTrigger,
    RightTrigger
};

void InputBindAction(const std::string& actionName, Input::DeviceType device, int code);
void InputBindAxis(const std::string& axisName, DeviceType device, int code, float scale = 1.0f, float deadZone = 0.2f);

bool InputIsTrigger(const std::string& actionName);
bool InputIsRelease(const std::string& actionName);
bool InputIsPress(const std::string& actionName);
float GetInputAxisValue(const std::string& axisName);
float GetInputPreAxisValue(const std::string& axisName);

}

struct PhysicalInput {
    Input::DeviceType device;
    int code; // KeyboardならkeyCode, PadならGamepadButton, MouseならMouseButtonをキャストして保持
    float scale = 1.0f;
    float deadZone;
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
    void BindAxis(const std::string& axisName, Input::DeviceType device, int code, float scale = 1.0f, float deadZone = 0.2f);

    bool IsActionTriggered(const std::string& actionName);
    bool IsActionReleased(const std::string& actionName);
    bool IsActionPressed(const std::string& actionName);
    float GetAxisValue(const std::string& axisName);
    float GetPreAxisValue(const std::string& axisName);
private:
    std::unordered_map<std::string, ActionBinding> actionMap_;
    std::unordered_map<std::string, ActionBinding> axisMap_;

    bool CheckPhysicalTrigger(const PhysicalInput& bind);
    bool CheckPhysicalRelease(const PhysicalInput& bind);
    bool CheckPhysicalPress(const PhysicalInput& bind);
    float GetPhysicalAxis(const PhysicalInput& bind);
    float GetPrePhysicalAxis(const PhysicalInput& bind);
    float ApplyDeadZone(float value, float deadZone);
    InputConfig() = default;
    ~InputConfig() = default;
};
}

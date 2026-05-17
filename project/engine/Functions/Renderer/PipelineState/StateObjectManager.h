#pragma once
namespace NoEngine {
struct StateObject {
    Microsoft::WRL::ComPtr<ID3D12StateObject> stateObject;
    Microsoft::WRL::ComPtr<ID3D12Resource> shaderTable;
    D3D12_DISPATCH_RAYS_DESC dispatchDesc;
};


class StateObjectManager {
public:

    void Register(const std::string& name, const StateObject& entry);
    StateObject& Get(const std::string& name);

private:
    std::unordered_map<std::string, StateObject> entries_;
};
}


#pragma once
namespace NoEngine {
/// <summary>
/// コマンドアロケータを所有するクラス
/// </summary>
class CommandAllocator {
public:
	CommandAllocator(ID3D12Device* device);
	~CommandAllocator();

	ID3D12CommandAllocator* GetCommandAllocator() { return commandAllocator_.Get(); }
private:
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
};


}
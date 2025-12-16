#pragma once
#include "CommandAllocatorPool.h"

namespace NoEngine {
/// <summary>
/// コマンドキューを所有するクラス
/// </summary>
class CommandQueue {
	friend class CommandListManager;
	friend class CommandContext;
public:
	CommandQueue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);
	~CommandQueue();

	void Create();
	void Shutdown();

	inline bool IsReady() {
		return commandQueue_ != nullptr;
	}

	/// <summary>
	/// フェンスの値を更新します。
	/// </summary>
	/// <returns>更新後のフェンスの値</returns>
	uint64_t IncrementFence(void);

	/// <summary>
	/// 指定した値のフェンスが完了しているかどうかを取得します。
	/// </summary>
	/// <param name="fenceValue">フェンスの値</param>
	/// <returns>完了しているか</returns>
	bool IsFenceComplete(uint64_t fenceValue);
	void StallForFence(uint64_t fenceValue);
	void StallForProducer(CommandQueue& producer);
	void WaitForFence(uint64_t fenceValue);
	void WaitForIdle(void) { WaitForFence(IncrementFence()); }

	ID3D12CommandQueue* GetCommandQueue() { return commandQueue_.Get(); }
private:
	uint64_t ExecuteCommandList(ID3D12CommandList* list);
	ID3D12CommandAllocator* RequestAllocator(void);
	void DiscardAllocator(uint64_t fenceValueForReset, ID3D12CommandAllocator* allocator);

	// コマンドキュー(コマンドリストをGPUに投げて実行させる物)
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;

	const D3D12_COMMAND_LIST_TYPE type_;

	CommandAllocatorPool allocatorPool_;
	std::mutex fenceMutex_;
	std::mutex eventMutex_;

	// これらのオブジェクトの存続期間は記述子キャッシュによって管理されます
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
	uint64_t nextFenceValue_ = 0;
	uint64_t lastCompletedFenceValue_ = 0;
	HANDLE fenceEventHandle_ = nullptr;
};
}

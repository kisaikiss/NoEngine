#pragma once
#include "CommandQueue.h"


namespace NoEngine {

class CommandListManager {
	friend class CommandContext;
public:
	CommandListManager();
	~CommandListManager();

	void Create();
	void Shutdown();

	CommandQueue& GetGraphicsQueue(void) { return graphicsQueue_; }
	CommandQueue& GetComputeQueue(void) { return computeQueue_; }
	CommandQueue& GetCopyQueue(void) { return copyQueue_; }

	/// <summary>
	/// コマンドリストのタイプからコマンドキューを取得します。
	/// </summary>
	/// <param name="Type">コマンドリストのタイプ</param>
	/// <returns>コマンドキュー</returns>
	CommandQueue& GetQueue(D3D12_COMMAND_LIST_TYPE Type = D3D12_COMMAND_LIST_TYPE_DIRECT) {
		switch (Type) {
		case D3D12_COMMAND_LIST_TYPE_COMPUTE: return computeQueue_;
		case D3D12_COMMAND_LIST_TYPE_COPY: return copyQueue_;
		default: return graphicsQueue_;
		}
	}

	/// <summary>
	/// Direct3D 12のコマンドキューを取得します。
	/// </summary>
	/// <returns>GraphicsQueue</returns>
	ID3D12CommandQueue* GetCommandQueue() {
		return graphicsQueue_.GetCommandQueue();
	}

	/// <summary>
	/// 新しいコマンドリストを作成します。
	/// </summary>
	/// <param name="Type">コマンドリストのタイプ</param>
	/// <param name="List">作成するコマンドリストのポインタ</param>
	/// <param name="Allocator">コマンドアロケータのダブルポインタ</param>
	void CreateNewCommandList(
		D3D12_COMMAND_LIST_TYPE type,
		ID3D12GraphicsCommandList** list,
		ID3D12CommandAllocator** allocator);
private:
	CommandQueue graphicsQueue_;
	CommandQueue computeQueue_;
	CommandQueue copyQueue_;
};
}

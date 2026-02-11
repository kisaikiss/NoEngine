#pragma once
#include "engine/Utilities/NonCopyable.h"
#include "../GpuResource/GpuBuffer.h"
#include "../ContextManager.h"
#include "../GpuResource/LinearAllocator/LinearAllocator.h"
#include "CommandListManager.h"
#include "../DescriptorHeap/DynamicDescriptorHeap.h"
#include "engine/Runtime/PipelineStateObject/PSO.h"

namespace NoEngine {
class ColorBuffer;
class DepthBuffer;
class Texture;
class GraphicsContext;
class ComputeContext;
class UploadBuffer;
class ReadbackBuffer;

/// <summary>
/// 描画・計算・リソース操作を抽象化するクラス
/// </summary>
class CommandContext : NonCopyable {
	friend ContextManager;
private:

	CommandContext(D3D12_COMMAND_LIST_TYPE type);


public:
	void Reset(void);

	~CommandContext(void);

	static void DestroyAllContexts(void);

	static CommandContext& Begin(const std::wstring id = L"");

	/// <summary>
	/// 既存のコマンドをフラッシュし、現在のコンテキストを解放します。結果を uint64_t で返します。
	/// </summary>
	/// <param name="WaitForCompletion">完了まで待機するかどうかを示すフラグ。true の場合、関数は処理が完了するまでブロッキングします。デフォルトは false。</param>
	/// <returns>処理の結果を表す uint64_t 値。</returns>
	uint64_t Finish(bool WaitForCompletion = false);

	// Prepare to render by reserving a command list and command allocator
	void Initialize(void);

	GraphicsContext& GetGraphicsContext();

	ComputeContext& GetComputeContext();

	void CopyBuffer(GpuResource& Dest, GpuResource& Src);
	void CopyBufferRegion(GpuResource& Dest, size_t DestOffset, GpuResource& Src, size_t SrcOffset, size_t NumBytes);
	

	DynAlloc ReserveUploadMemory(size_t SizeInBytes) {
	    return cpuLinearAllocator_.Allocate(SizeInBytes);
	}

	/// <summary>
	/// テクスチャを初期化します。
	/// </summary>
	/// <param name="Dest">初期化先のGPUバッファへの参照</param>
	/// <param name="NumSubresources"></param>
	/// <param name="SubData"></param>
	static void InitializeTexture(GpuResource& dest, UINT numSubresources, D3D12_SUBRESOURCE_DATA subData[]);

	/// <summary>
	/// GPUバッファを初期化します。指定したバイト数をソースデータから宛先バッファの指定オフセットにコピーします。
	/// </summary>
	/// <param name="dest">初期化先のGPUバッファへの参照。</param>
	/// <param name="data">コピー元のデータへのポインタ。numBytesバイト分を読み取ります。</param>
	/// <param name="numBytes">コピーするバイト数。</param>
	/// <param name="destOffset">宛先バッファ内の書き込み開始オフセット（バイト単位）。既定値は0。</param>
	static void InitializeBuffer(GpuBuffer& dest, const void* data, size_t numBytes, size_t destOffset = 0);

	/// <summary>
	/// UploadBufferからGpuBufferへデータをコピーして初期化します。
	/// </summary>
	/// <param name="dest">コピー先のGpuBuffer。書き込み先バッファを参照で渡します。</param>
	/// <param name="src">コピー元のUploadBuffer。読み取り元のバッファ（const参照）。</param>
	/// <param name="srcOffset">src内で読み取りを開始するオフセット（バイト単位）。</param>
	/// <param name="numBytes">コピーするバイト数。省略またはデフォルト値（-1 → size_tの最大値）の場合はsrcの残り全てをコピーします。</param>
	/// <param name="destOffset">dest内で書き込みを開始するオフセット（バイト単位）。デフォルトは0。</param>
	static void InitializeBuffer(GpuBuffer& dest, const UploadBuffer& src, size_t srcOffset, size_t numBytes = -1, size_t destOffset = 0);
	
	/// <summary>
	/// リソースの状態遷移を行います。
	/// </summary>
	/// <param name="resource">状態遷移したいリソース</param>
	/// <param name="newState">リソースの新しい状態</param>
	/// <param name="flushImmediate">即座に状態遷移するかどうか(true : 遷移する, false : 遷移しない)。遷移しない場合はFlushResourceBarriers()を呼び出す必要があります。</param>
	void TransitionResource(GpuResource& resource, D3D12_RESOURCE_STATES newState, bool flushImmediate = false);

	/// <summary>
	/// 溜めていた状態遷移要求をまとめて発行します。
	/// </summary>
	void FlushResourceBarriers(void);

	void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, ID3D12DescriptorHeap* heapPtr);
	void SetDescriptorHeaps(UINT heapCount, D3D12_DESCRIPTOR_HEAP_TYPE type[], ID3D12DescriptorHeap* heapPtrs[]);
	void SetPipelineState(const PSO& PSO);

	ID3D12GraphicsCommandList4* GetCommandList() const { return commandList_; }
protected:

	void BindDescriptorHeaps(void);

	std::unique_ptr<CommandListManager> owningManager_;
	ID3D12GraphicsCommandList4* commandList_;
	ID3D12CommandAllocator* currentAllocator_;

	ID3D12RootSignature* curGraphicsRootSignature_;
	ID3D12RootSignature* curComputeRootSignature_;
	ID3D12PipelineState* curPipelineState_;

	DynamicDescriptorHeap dynamicViewDescriptorHeap_;		// HEAP_TYPE_CBV_SRV_UAV
	DynamicDescriptorHeap dynamicSamplerDescriptorHeap_;	// HEAP_TYPE_SAMPLER

	D3D12_RESOURCE_BARRIER resourceBarrierBuffer_[16];
	UINT numBarriersToFlush_;

	ID3D12DescriptorHeap* currentDescriptorHeaps_[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

	LinearAllocator cpuLinearAllocator_;
	LinearAllocator gpuLinearAllocator_;

	std::wstring id_;
	void SetID(const std::wstring& id) { id_ = id; }
	//
	D3D12_COMMAND_LIST_TYPE type_;
};


inline void CommandContext::SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, ID3D12DescriptorHeap* heapPtr) {
	if (currentDescriptorHeaps_[type] != heapPtr) {
		currentDescriptorHeaps_[type] = heapPtr;
		BindDescriptorHeaps();
	}
}


inline void CommandContext::SetDescriptorHeaps(UINT heapCount, D3D12_DESCRIPTOR_HEAP_TYPE type[], ID3D12DescriptorHeap* heapPtrs[]) {
	bool anyChanged = false;

	for (UINT i = 0; i < heapCount; ++i) {
		if (currentDescriptorHeaps_[type[i]] != heapPtrs[i]) {
			currentDescriptorHeaps_[type[i]] = heapPtrs[i];
			anyChanged = true;
		}
	}

	if (anyChanged)
		BindDescriptorHeaps();
}

inline void CommandContext::SetPipelineState(const PSO& PSO) {
	ID3D12PipelineState* PipelineState = PSO.GetPipelineStateObject();
	if (PipelineState == curPipelineState_)
		return;

	commandList_->SetPipelineState(PipelineState);
	curPipelineState_ = PipelineState;
}

}


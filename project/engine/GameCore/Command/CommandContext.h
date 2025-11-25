#pragma once
#include "engine/Utilities/NonCopyable.h"
#include "../GpuResource/GpuBuffer.h"
#include "../ContextManager.h"
#include "../GpuResource/LinearAllocator/LinearAllocator.h"
#include "CommandListManager.h"




class ColorBuffer;
class DepthBuffer;
class Texture;
class GraphicsContext;
class ComputeContext;
class UploadBuffer;
class ReadbackBuffer;

namespace NoEngine {
/// <summary>
/// 描画・計算・リソース操作を抽象化するクラス
/// </summary>
class CommandContext : NonCopyable {
	friend ContextManager;
private:

	CommandContext(D3D12_COMMAND_LIST_TYPE type);

	void Reset(void);

public:

	~CommandContext(void);

	//static void DestroyAllContexts(void);

	static CommandContext& Begin(const std::wstring id = L"");

	// Flush existing commands to the GPU but keep the context alive

	/// <summary>
	/// 既存のコマンドをGPUにフラッシュしますが、コンテキストは維持します。結果を uint64_t で返します。
	/// </summary>
	/// <param name="WaitForCompletion">完了まで待機するかどうかを示すフラグ。true の場合、関数は処理が完了するまでブロッキングします。デフォルトは false。</param>
	/// <returns>処理の結果を表す uint64_t 値。</returns>
	//uint64_t Flush(bool WaitForCompletion = false);

	// Flush existing commands and release the current context

	/// <summary>
	/// 既存のコマンドをフラッシュし、現在のコンテキストを解放します。結果を uint64_t で返します。
	/// </summary>
	/// <param name="WaitForCompletion">完了まで待機するかどうかを示すフラグ。true の場合、関数は処理が完了するまでブロッキングします。デフォルトは false。</param>
	/// <returns>処理の結果を表す uint64_t 値。</returns>
	//uint64_t Finish(bool WaitForCompletion = false);

	// Prepare to render by reserving a command list and command allocator
	void Initialize(void);

	//GraphicsContext& GetGraphicsContext() {
	//    ASSERT(m_Type != D3D12_COMMAND_LIST_TYPE_COMPUTE, "Cannot convert async compute context to graphics");
	//    return reinterpret_cast<GraphicsContext&>(*this);
	//}

	//ComputeContext& GetComputeContext() {
	//    return reinterpret_cast<ComputeContext&>(*this);
	//}

	//ID3D12GraphicsCommandList* GetCommandList() {
	//    return m_CommandList;
	//}

	//void CopyBuffer(GpuResource& Dest, GpuResource& Src);
	//void CopyBufferRegion(GpuResource& Dest, size_t DestOffset, GpuResource& Src, size_t SrcOffset, size_t NumBytes);
	//void CopySubresource(GpuResource& Dest, UINT DestSubIndex, GpuResource& Src, UINT SrcSubIndex);
	//void CopyCounter(GpuResource& Dest, size_t DestOffset, StructuredBuffer& Src);
	//void CopyTextureRegion(GpuResource& Dest, UINT x, UINT y, UINT z, GpuResource& Source, RECT& rect);
	//void ResetCounter(StructuredBuffer& Buf, uint32_t Value = 0);

	//// Creates a readback buffer of sufficient size, copies the texture into it,
	//// and returns row pitch in bytes.
	//uint32_t ReadbackTexture(ReadbackBuffer& DstBuffer, PixelBuffer& SrcBuffer);

	//DynAlloc ReserveUploadMemory(size_t SizeInBytes) {
	//    return m_CpuLinearAllocator.Allocate(SizeInBytes);
	//}

	//static void InitializeTexture(GpuResource& Dest, UINT NumSubresources, D3D12_SUBRESOURCE_DATA SubData[]);

	/// <summary>
	/// GPUバッファを初期化します。指定したバイト数をソースデータから宛先バッファの指定オフセットにコピーします。
	/// </summary>
	/// <param name="dest">初期化先のGPUバッファへの参照。</param>
	/// <param name="data">コピー元のデータへのポインタ。numBytesバイト分を読み取ります。</param>
	/// <param name="numBytes">コピーするバイト数。</param>
	/// <param name="destOffset">宛先バッファ内の書き込み開始オフセット（バイト単位）。既定値は0。</param>
	//static void InitializeBuffer(GpuBuffer& dest, const void* data, size_t numBytes, size_t destOffset = 0);

	/// <summary>
	/// UploadBufferからGpuBufferへデータをコピーして初期化します。
	/// </summary>
	/// <param name="dest">コピー先のGpuBuffer。書き込み先バッファを参照で渡します。</param>
	/// <param name="src">コピー元のUploadBuffer。読み取り元のバッファ（const参照）。</param>
	/// <param name="srcOffset">src内で読み取りを開始するオフセット（バイト単位）。</param>
	/// <param name="numBytes">コピーするバイト数。省略またはデフォルト値（-1 → size_tの最大値）の場合はsrcの残り全てをコピーします。</param>
	/// <param name="destOffset">dest内で書き込みを開始するオフセット（バイト単位）。デフォルトは0。</param>
	//static void InitializeBuffer(GpuBuffer& dest, const UploadBuffer& src, size_t srcOffset, size_t numBytes = -1, size_t destOffset = 0);
	//static void InitializeTextureArraySlice(GpuResource& Dest, UINT SliceIndex, GpuResource& Src);
	//
	//    void WriteBuffer(GpuResource& Dest, size_t DestOffset, const void* Data, size_t NumBytes);
	//    void FillBuffer(GpuResource& Dest, size_t DestOffset, DWParam Value, size_t NumBytes);
	//
	//    void TransitionResource(GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate = false);
	//    void BeginResourceTransition(GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate = false);
	//    void InsertUAVBarrier(GpuResource& Resource, bool FlushImmediate = false);
	//    void InsertAliasBarrier(GpuResource& Before, GpuResource& After, bool FlushImmediate = false);
	//    inline void FlushResourceBarriers(void);
	//
	//    void InsertTimeStamp(ID3D12QueryHeap* pQueryHeap, uint32_t QueryIdx);
	//    void ResolveTimeStamps(ID3D12Resource* pReadbackHeap, ID3D12QueryHeap* pQueryHeap, uint32_t NumQueries);
	//    void PIXBeginEvent(const wchar_t* label);
	//    void PIXEndEvent(void);
	//    void PIXSetMarker(const wchar_t* label);
	//
	//    void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type, ID3D12DescriptorHeap* HeapPtr);
	//    void SetDescriptorHeaps(UINT HeapCount, D3D12_DESCRIPTOR_HEAP_TYPE Type[], ID3D12DescriptorHeap* HeapPtrs[]);
	//    void SetPipelineState(const PSO& PSO);
	//
	//    void SetPredication(ID3D12Resource* Buffer, UINT64 BufferOffset, D3D12_PREDICATION_OP Op);
	//
protected:

	//void BindDescriptorHeaps(void);

	std::unique_ptr<CommandListManager> owningManager_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
	ID3D12CommandAllocator* currentAllocator_;

	ID3D12RootSignature* m_CurGraphicsRootSignature;
	ID3D12RootSignature* m_CurComputeRootSignature;
	ID3D12PipelineState* m_CurPipelineState;

	//DynamicDescriptorHeap m_DynamicViewDescriptorHeap;		// HEAP_TYPE_CBV_SRV_UAV
	//DynamicDescriptorHeap m_DynamicSamplerDescriptorHeap;	// HEAP_TYPE_SAMPLER

	D3D12_RESOURCE_BARRIER m_ResourceBarrierBuffer[16];
	UINT m_NumBarriersToFlush;

	ID3D12DescriptorHeap* m_CurrentDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

	//LinearAllocator m_CpuLinearAllocator;
	//LinearAllocator m_GpuLinearAllocator;

	std::wstring id_;
	void SetID(const std::wstring& id) { id_ = id; }
	//
	D3D12_COMMAND_LIST_TYPE type_;
};
}


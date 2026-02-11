#include "CommandContext.h"
#include "GraphicsContext.h"

#include "../GraphicsCore.h"
#include "engine/Functions/Debug/Logger/Log.h"
#include "engine/Math/Common.h"

#include "../GpuResource/UploadBuffer.h"

namespace NoEngine {
CommandContext::CommandContext(D3D12_COMMAND_LIST_TYPE type) :
	type_(type),
	dynamicViewDescriptorHeap_(*this, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
	dynamicSamplerDescriptorHeap_(*this, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER),
	cpuLinearAllocator_(LinearAllocatorType::kCpuWritable),
	gpuLinearAllocator_(LinearAllocatorType::kGpuExclusive) {

	owningManager_ = nullptr;
	commandList_ = nullptr;
	currentAllocator_ = nullptr;
	ZeroMemory(currentDescriptorHeaps_, sizeof(currentDescriptorHeaps_));

	curGraphicsRootSignature_ = nullptr;
	curComputeRootSignature_ = nullptr;
	curPipelineState_ = nullptr;
	numBarriersToFlush_ = 0;
}

void CommandContext::Reset(void) {
	// Reset() は、以前に解放されたコンテキストに対してのみ呼び出します。コマンドリストは保持されますが、
	// 新しいアロケータを要求する必要があります。
	assert(commandList_ != nullptr && currentAllocator_ == nullptr);
	currentAllocator_ = GraphicsCore::gCommandListManager.GetQueue(type_).RequestAllocator();
	commandList_->Reset(currentAllocator_, nullptr);

	curGraphicsRootSignature_ = nullptr;
	curComputeRootSignature_ = nullptr;
	curPipelineState_ = nullptr;
	numBarriersToFlush_ = 0;

	BindDescriptorHeaps();
}

CommandContext::~CommandContext(void) {
	if (commandList_) {
		commandList_->Release();
	}
}

void CommandContext::DestroyAllContexts(void) {
	LinearAllocator::DestroyAll();
	DynamicDescriptorHeap::DestroyAll();
	GraphicsCore::gContextManager.DestroyAllContexts();
}

CommandContext& CommandContext::Begin(const std::wstring id) {
	CommandContext* newContext = GraphicsCore::gContextManager.AllocateContext(D3D12_COMMAND_LIST_TYPE_DIRECT);
	newContext->SetID(id);
	return *newContext;
}

//uint64_t CommandContext::Flush(bool WaitForCompletion) {
//	return 0;
//}

uint64_t CommandContext::Finish(bool WaitForCompletion) {
	assert(type_ == D3D12_COMMAND_LIST_TYPE_DIRECT || type_ == D3D12_COMMAND_LIST_TYPE_COMPUTE);

	FlushResourceBarriers();

	assert(currentAllocator_ != nullptr);

	CommandQueue& queue = GraphicsCore::gCommandListManager.GetQueue(type_);
	uint64_t fenceValue = queue.ExecuteCommandList(commandList_);

	queue.DiscardAllocator(fenceValue, currentAllocator_);
	currentAllocator_ = nullptr;
	cpuLinearAllocator_.CleanupUsedPages(fenceValue);
	gpuLinearAllocator_.CleanupUsedPages(fenceValue);
	dynamicViewDescriptorHeap_.CleanupUsedHeaps(fenceValue);
	dynamicSamplerDescriptorHeap_.CleanupUsedHeaps(fenceValue);

	if (WaitForCompletion) {
		GraphicsCore::gCommandListManager.WaitForFence(fenceValue);
	}

	GraphicsCore::gContextManager.FreeContext(this);

	return fenceValue;
}

void CommandContext::Initialize(void) {
	GraphicsCore::gCommandListManager.CreateNewCommandList(type_, &commandList_, &currentAllocator_);
}

GraphicsContext& CommandContext::GetGraphicsContext() {
	if (type_ == D3D12_COMMAND_LIST_TYPE_COMPUTE) {
		Log::DebugPrint("Cannot convert async compute context to graphics", VerbosityLevel::kCritical);
		assert(false);
	}
	Log::DebugPrint("Converting a command context to a graphics context");
	return reinterpret_cast<GraphicsContext&>(*this);
}

ComputeContext& CommandContext::GetComputeContext() {
	Log::DebugPrint("Converting a command context to a compute context");
	return reinterpret_cast<ComputeContext&>(*this);
	
}

void CommandContext::CopyBuffer(GpuResource& Dest, GpuResource& Src) {
	TransitionResource(Dest, D3D12_RESOURCE_STATE_COPY_DEST);
	TransitionResource(Src, D3D12_RESOURCE_STATE_COPY_SOURCE);
	FlushResourceBarriers();
	commandList_->CopyResource(Dest.GetResource(), Src.GetResource());
}

void CommandContext::CopyBufferRegion(GpuResource& Dest, size_t DestOffset, GpuResource& Src, size_t SrcOffset, size_t NumBytes) {
	TransitionResource(Dest, D3D12_RESOURCE_STATE_COPY_DEST);
	//TransitionResource(Src, D3D12_RESOURCE_STATE_COPY_SOURCE);
	FlushResourceBarriers();
	commandList_->CopyBufferRegion(Dest.GetResource(), DestOffset, Src.GetResource(), SrcOffset, NumBytes);
}

void CommandContext::InitializeTexture(GpuResource& dest, UINT numSubresources, D3D12_SUBRESOURCE_DATA subData[]) {
	UINT64 uploadBufferSize = GetRequiredIntermediateSize(dest.GetResource(), 0, numSubresources);

	CommandContext& InitContext = CommandContext::Begin();

	// データを中間アップロードヒープにコピーし、アップロードヒープからデフォルトテクスチャへのコピーをスケジュールします。
	DynAlloc mem = InitContext.ReserveUploadMemory(uploadBufferSize);
	UpdateSubresources(InitContext.commandList_, dest.GetResource(), mem.Buffer.GetResource(), 0, 0, numSubresources, subData);
	InitContext.TransitionResource(dest, D3D12_RESOURCE_STATE_GENERIC_READ);

	// コマンドリストを実行し、アップロードバッファを解放できるように完了するまで待ちます。
	InitContext.Finish(true);
}

void CommandContext::InitializeBuffer(GpuBuffer& dest, const void* bufferData, size_t numBytes, size_t destOffset) {
	CommandContext& InitContext = CommandContext::Begin();

	DynAlloc mem = InitContext.ReserveUploadMemory(numBytes);
	// ToDo : エンジン設計の元にしたMicrosoft MiniEngineではSIMDを利用したより高速なmemcpy()を使用しているので、std::memcpyでは速度に多少問題があると思われます。
	std::memcpy(mem.DataPtr, bufferData, Math::DivideByMultiple(numBytes, 16));

	// データを中間アップロードヒープにコピーし、アップロードヒープからデフォルトテクスチャへのコピーをスケジュールします。
	InitContext.TransitionResource(dest, D3D12_RESOURCE_STATE_COPY_DEST, true);
	InitContext.commandList_->CopyBufferRegion(dest.GetResource(), destOffset, mem.Buffer.GetResource(), 0, numBytes);
	InitContext.TransitionResource(dest, D3D12_RESOURCE_STATE_GENERIC_READ, true);

	// コマンドリストを実行し、アップロードバッファを解放できるように完了するまで待ちます。
	InitContext.Finish(true);
}

void CommandContext::InitializeBuffer(GpuBuffer& dest, const UploadBuffer& src, size_t srcOffset, size_t numBytes, size_t destOffset) {
	CommandContext& InitContext = CommandContext::Begin();

	size_t maxBytes = std::min<size_t>(dest.GetBufferSize() - destOffset, src.GetBufferSize() - srcOffset);
	numBytes = std::min<size_t>(maxBytes, numBytes);

	// データを中間アップロードヒープにコピーし、アップロードヒープからデフォルトテクスチャへのコピーをスケジュールします。
	InitContext.TransitionResource(dest, D3D12_RESOURCE_STATE_COPY_DEST, true);
	InitContext.commandList_->CopyBufferRegion(dest.GetResource(), destOffset, (ID3D12Resource*)src.GetResource(), srcOffset, numBytes);
	InitContext.TransitionResource(dest, D3D12_RESOURCE_STATE_GENERIC_READ, true);

	// コマンドリストを実行し、アップロードバッファを解放できるように完了するまで待ちます。
	InitContext.Finish(true);
}

void CommandContext::TransitionResource(GpuResource& resource, D3D12_RESOURCE_STATES newState, bool flushImmediate) {
	D3D12_RESOURCE_STATES oldState = resource.usageState_;

	if (oldState != newState) {
		if (numBarriersToFlush_ >= 16) {
			Log::DebugPrint("Exceeded arbitrary limit on buffered barriers", VerbosityLevel::kCritical);
			assert(false);
		}

		D3D12_RESOURCE_BARRIER& barrierDesc = resourceBarrierBuffer_[numBarriersToFlush_++];

		barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierDesc.Transition.pResource = resource.GetResource();
		barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrierDesc.Transition.StateBefore = oldState;
		barrierDesc.Transition.StateAfter = newState;

		// 移行がすでに開始されているかどうかを確認します。
		if (newState == resource.transitioningState_) {
			barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
			resource.transitioningState_ = (D3D12_RESOURCE_STATES)-1;
		} else {
			barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		}

		resource.usageState_ = newState;
	} else {
		if (newState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
			//InsertUAVBarrier(Resource, FlushImmediate);
		}
	}

	if (flushImmediate || numBarriersToFlush_ == 16) {
		FlushResourceBarriers();
	}
}

void CommandContext::FlushResourceBarriers(void) {
	if (numBarriersToFlush_ > 0) {
		commandList_->ResourceBarrier(numBarriersToFlush_, resourceBarrierBuffer_);
		numBarriersToFlush_ = 0;
	}
}

void CommandContext::BindDescriptorHeaps(void) {
	UINT NonNullHeaps = 0;
	ID3D12DescriptorHeap* HeapsToBind[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
	for (UINT i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i) {
		ID3D12DescriptorHeap* HeapIter = currentDescriptorHeaps_[i];
		if (HeapIter != nullptr)
			HeapsToBind[NonNullHeaps++] = HeapIter;
	}

	if (NonNullHeaps > 0)
		commandList_->SetDescriptorHeaps(NonNullHeaps, HeapsToBind);
}

}
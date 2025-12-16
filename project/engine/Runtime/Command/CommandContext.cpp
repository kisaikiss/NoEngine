#include "CommandContext.h"
#include "GraphicsContext.h"

#include "../GraphicsCore.h"
#include "engine/Debug/Logger/Log.h"

namespace NoEngine {
CommandContext::CommandContext(D3D12_COMMAND_LIST_TYPE type) :
type_(type)
{
}

void CommandContext::Reset(void){
	// Reset() は、以前に解放されたコンテキストに対してのみ呼び出します。コマンドリストは保持されますが、
	// 新しいアロケータを要求する必要があります。
	assert(commandList_ != nullptr && currentAllocator_ == nullptr);
	currentAllocator_ = GraphicsCore::gCommandListManager.GetQueue(type_).RequestAllocator();
	commandList_->Reset(currentAllocator_, nullptr);

	m_CurGraphicsRootSignature = nullptr;
	m_CurComputeRootSignature = nullptr;
	m_CurPipelineState = nullptr;
	numBarriersToFlush_ = 0;

	BindDescriptorHeaps();
}

CommandContext::~CommandContext(void) {
	if (commandList_) {
		commandList_->Release();
	}
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

//
//void CommandContext::InitializeBuffer(GpuBuffer& dest, const void* data, size_t numBytes, size_t destOffset) {
//	CommandContext& initContext = CommandContext::Begin();
//
//	DynAlloc mem = InitContext.ReserveUploadMemory(NumBytes);
//	SIMDMemCopy(mem.DataPtr, BufferData, Math::DivideByMultiple(NumBytes, 16));
//
//	// copy data to the intermediate upload heap and then schedule a copy from the upload heap to the default texture
//	InitContext.TransitionResource(dest, D3D12_RESOURCE_STATE_COPY_DEST, true);
//	InitContext.m_CommandList->CopyBufferRegion(dest.GetResource(), destOffset, mem.Buffer.GetResource(), 0, NumBytes);
//	InitContext.TransitionResource(dest, D3D12_RESOURCE_STATE_GENERIC_READ, true);
//
//	// Execute the command list and wait for it to finish so we can release the upload buffer
//	InitContext.Finish(true);
//}

}
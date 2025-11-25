#include "CommandContext.h"

#include "../GraphicsCore.h"

namespace NoEngine {
CommandContext::CommandContext(D3D12_COMMAND_LIST_TYPE type) :
type_(type)
{
}

void CommandContext::Reset(void){
	// Reset() は、以前に解放されたコンテキストに対してのみ呼び出します。コマンドリストは保持されますが、
	// 新しいアロケータを要求する必要があります。
	/*assert(commandList_ != nullptr && currentAllocator_ == nullptr);
	currentAllocator_ = g_CommandManager.GetQueue(m_Type).RequestAllocator();
	commandList_->Reset(m_CurrentAllocator, nullptr);

	m_CurGraphicsRootSignature = nullptr;
	m_CurComputeRootSignature = nullptr;
	m_CurPipelineState = nullptr;
	m_NumBarriersToFlush = 0;

	BindDescriptorHeaps();*/
}

CommandContext::~CommandContext(void) {}

CommandContext& CommandContext::Begin(const std::wstring id) {
	CommandContext* newContext = GraphicsCore::gContextManager.AllocateContext(D3D12_COMMAND_LIST_TYPE_DIRECT);
	newContext->SetID(id);
	return *newContext;
}
//
//uint64_t CommandContext::Flush(bool WaitForCompletion) {
//	return 0;
//}

void CommandContext::Initialize(void) {
	GraphicsCore::gCommandListManager.CreateNewCommandList(type_, &commandList_, &currentAllocator_);
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
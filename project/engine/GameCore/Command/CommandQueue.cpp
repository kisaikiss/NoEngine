#include "CommandQueue.h"
#include "CommandListManager.h"
namespace NoEngine {
CommandQueue::CommandQueue(D3D12_COMMAND_LIST_TYPE type) :
	type_(type) ,
	allocatorPool_(type){
}

CommandQueue::~CommandQueue() {
	Shutdown();
}

void CommandQueue::Create(ID3D12Device* device) {
	// コマンドキュー(コマンドリストをGPUに投げて実行させる物)を生成します。
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	commandQueueDesc.Type = type_;
	commandQueueDesc.NodeMask = 1;
	HRESULT hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue_));
	// コマンドキューの生成がうまくいかなかった場合は起動できないので、assertで止めます。
	assert(SUCCEEDED(hr));
	// PIXなどのデバッグ用に名前を付けます。
	commandQueue_.Get()->SetName(L"CommandListManager::commandQueue_");

	// フェンスを生成します。
	hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	assert(SUCCEEDED(hr));
	fence_.Get()->SetName(L"CommandListManager::fence_");

	// << 56によって64bit整数の上位8bitにキュー識別を埋め込みます。
	// これでフェンス値の上位8bitでキューの種類を識別し、下位56bitで通常のカウントに使います。
	fence_.Get()->Signal((uint64_t)type_ << 56);

	// fenceのSignalを待つためのイベントを作成します。
	fenceEventHandle_ = CreateEvent(nullptr, false, false, nullptr);
	assert(fenceEventHandle_ != nullptr);

	// m_AllocatorPool.Create(pDevice);
}

void CommandQueue::Shutdown() {

}

uint64_t CommandQueue::IncrementFence(void) {
	std::lock_guard<std::mutex> LockGuard(fenceMutex_);
	commandQueue_->Signal(fence_.Get(), nextFenceValue_);
	return nextFenceValue_++;
}

bool CommandQueue::IsFenceComplete(uint64_t fenceValue) {
	// 最後に確認したフェンス値と比較することで、フェンス値のクエリを回避します。
	// max() は、最後に完了したフェンス値が後退する可能性のある、起こりそうにない競合状態から保護するためのものです。
	if (fenceValue > lastCompletedFenceValue_)
		lastCompletedFenceValue_ = std::max(lastCompletedFenceValue_, fence_->GetCompletedValue());

	return fenceValue <= lastCompletedFenceValue_;
}



void CommandQueue::StallForProducer(CommandQueue& producer) {
	assert(producer.nextFenceValue_ > 0);
	commandQueue_->Wait(producer.fence_.Get(), producer.nextFenceValue_ - 1);
}


void CommandQueue::WaitForFence(uint64_t fenceValue) {
	if (IsFenceComplete(fenceValue))
		return;

	// TODO: これがマルチスレッド環境でどのような影響を与えるか考えてみましょう。スレッドAがフェンス100を待機しようとしていて、その後スレッドBがフェンス99を待機しようとしたとします。
	// もしフェンスが完了時に設定できるイベントが1つだけの場合、スレッドBはフェンス99の準備ができたと認識するまでに
	// フェンス100を待機しなければなりません。シーケンシャルイベントを挿入する方がよいでしょうか？
	{
		std::lock_guard<std::mutex> LockGuard(eventMutex_);

		fence_->SetEventOnCompletion(fenceValue, fenceEventHandle_);
		WaitForSingleObject(fenceEventHandle_, INFINITE);
		lastCompletedFenceValue_ = fenceValue;
	}
}

ID3D12CommandAllocator* CommandQueue::RequestAllocator(void) {
	uint64_t CompletedFence = fence_->GetCompletedValue();

	return allocatorPool_.RequestAllocator(CompletedFence);
}

}

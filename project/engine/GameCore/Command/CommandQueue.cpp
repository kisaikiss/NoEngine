#include "CommandQueue.h"
#include "CommandListManager.h"
#include "../GraphicsCore.h"

#include "engine/Debug/Logger/Log.h"

namespace NoEngine {
CommandQueue::CommandQueue(D3D12_COMMAND_LIST_TYPE type) :
	type_(type) ,
	allocatorPool_(type),
	nextFenceValue_(0),
	lastCompletedFenceValue_(0),
	fenceEventHandle_(nullptr) {
}

CommandQueue::~CommandQueue() {
	Shutdown();
}

void CommandQueue::Create() {
	// コマンドキュー(コマンドリストをGPUに投げて実行させる物)を生成します。
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	commandQueueDesc.Type = type_;
	commandQueueDesc.NodeMask = 1;
	HRESULT hr = GraphicsCore::gGraphicsDevice->GetDevice()->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue_));
	// コマンドキューの生成がうまくいかなかった場合は起動できないので、assertで止めます。
	assert(SUCCEEDED(hr));
	// PIXなどのデバッグ用に名前を付けます。
	commandQueue_.Get()->SetName(L"CommandListManager::commandQueue_");

	// フェンスを生成します。
	hr = GraphicsCore::gGraphicsDevice->GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	assert(SUCCEEDED(hr));
	fence_.Get()->SetName(L"CommandListManager::fence_");

	// フェンスの初期値を明示的に設定しておく（上位8bitにキュー種別を埋める意図を保持）
	uint64_t initialFence = ((uint64_t)type_ << 56);
	// CPU側でフェンス値を進めておき、メンバを正しく初期化する
	fence_.Get()->Signal(initialFence);
	nextFenceValue_ = initialFence + 1;
	lastCompletedFenceValue_ = fence_->GetCompletedValue();

	// fenceのSignalを待つためのイベントを作成します。
	fenceEventHandle_ = CreateEvent(nullptr, false, false, nullptr);
	assert(fenceEventHandle_ != nullptr);

	allocatorPool_.Create(GraphicsCore::gGraphicsDevice->GetDevice());
}

void CommandQueue::Shutdown() {
	if (commandQueue_ == nullptr)
		return;

	allocatorPool_.Shutdown();

	if (fenceEventHandle_) {
		CloseHandle(fenceEventHandle_);
		fenceEventHandle_ = nullptr;
	}

	fence_.Reset();

	commandQueue_.Reset();
}

uint64_t CommandQueue::IncrementFence(void) {
	std::lock_guard<std::mutex> LockGuard(fenceMutex_);
	commandQueue_->Signal(fence_.Get(), nextFenceValue_);
	return nextFenceValue_++;
}

bool CommandQueue::IsFenceComplete(uint64_t fenceValue) {
	// 最後に確認したフェンス値と比較することで、フェンス値のクエリを回避します。
	// max() は、最後に完了したフェンス値が後退する可能性のある、起こりそうにない競合状態から保護するためのもの。
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

		// Safety: 0 を渡さないようにする（D3D12は 0 を待つと常に満たされる -> デバッグ警告の原因）
		if (fenceValue == 0) {
			lastCompletedFenceValue_ = fence_->GetCompletedValue();
			return;
		}

		fence_->SetEventOnCompletion(fenceValue, fenceEventHandle_);
		WaitForSingleObject(fenceEventHandle_, INFINITE);
		lastCompletedFenceValue_ = fenceValue;
	}
}

uint64_t CommandQueue::ExecuteCommandList(ID3D12CommandList* list) {
	std::lock_guard<std::mutex> lockGuard(fenceMutex_);

	HRESULT hr = ((ID3D12GraphicsCommandList*)list)->Close();
	if (FAILED(hr)) {
		Log::DebugPrint("commandList close failed", VerbosityLevel::kCritical);
	}

	commandQueue_->ExecuteCommandLists(1, &list);

	commandQueue_->Signal(fence_.Get(), nextFenceValue_);

	return nextFenceValue_++;
}

ID3D12CommandAllocator* CommandQueue::RequestAllocator(void) {
	uint64_t CompletedFence = fence_->GetCompletedValue();

	return allocatorPool_.RequestAllocator(CompletedFence);
}

void CommandQueue::DiscardAllocator(uint64_t fenceValueForReset, ID3D12CommandAllocator* allocator) {
	allocatorPool_.DiscardAllocator(fenceValueForReset, allocator);
}
}

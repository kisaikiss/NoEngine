#include "CommandAllocatorPool.h"

namespace NoEngine {
CommandAllocatorPool::CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE type) :
	commandListType_(type),
	device_(nullptr)
{
}

CommandAllocatorPool::~CommandAllocatorPool() {
	Shutdown();
}

void CommandAllocatorPool::Create(ID3D12Device* device) {
	device_ = device;
}

void CommandAllocatorPool::Shutdown() {
    allocatorPool_.clear();
}

ID3D12CommandAllocator* CommandAllocatorPool::RequestAllocator(uint64_t completedFenceValue) {
    std::lock_guard<std::mutex> LockGuard(allocatorMutex_);

    // 返り値用のアロケータのポインタ
    ID3D12CommandAllocator* allocator = nullptr;

    // 再利用できるアロケータがあった場合は、そのアロケータを返り値用のポインタに入れます。
    if (!readyAllocators_.empty()) {
        // アロケータのキューから最初に入れられた値を取り出します。
        std::pair<uint64_t, ID3D12CommandAllocator*>& AllocatorPair = readyAllocators_.front();

        if (AllocatorPair.first <= completedFenceValue) {
            allocator = AllocatorPair.second;
            HRESULT hr = allocator->Reset();
            if (FAILED(hr)) {
                assert(false);
            }
            readyAllocators_.pop();
        }
    }

    // 再利用できるアロケータがない場合は、新しいアロケータを作成します。
    if (allocator == nullptr) {
        // アロケータのラッパークラスを生成します。
        std::unique_ptr<CommandAllocator> commandAllocator = std::make_unique<CommandAllocator>(device_);

        // アロケータの取得します。
        allocator = commandAllocator->GetCommandAllocator();

        // 名前を付けます。
        wchar_t AllocatorName[32];
        swprintf(AllocatorName, 32, L"CommandAllocator %zu", allocatorPool_.size());
        allocator->SetName(AllocatorName);

        // アロケータプールにpush_backします。
        allocatorPool_.push_back(std::move(commandAllocator));
    }

    return allocator;
}

void CommandAllocatorPool::DiscardAllocator(uint64_t fenceValue, ID3D12CommandAllocator* allocator) {
    std::lock_guard<std::mutex> LockGuard(allocatorMutex_);

    // このフェンス値は、アロケータを自由にリセットできることを示しています。
    readyAllocators_.push(std::make_pair(fenceValue, allocator));
}

}
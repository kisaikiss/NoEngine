#pragma once
#include "CommandAllocator.h"

namespace NoEngine {
/// <summary>
/// コマンドアロケータを管理するクラス
/// </summary>
class CommandAllocatorPool {

public:
    CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE type);
    ~CommandAllocatorPool();

    void Create(ID3D12Device* device);
    void Shutdown();

    /// <summary>
    /// 利用可能なコマンドアロケータをリクエストします。
    /// </summary>
    /// <param name="completedFenceValue"></param>
    /// <returns>利用可能なコマンドアロケータ</returns>
    ID3D12CommandAllocator* RequestAllocator(uint64_t completedFenceValue);

    /// <summary>
    /// 指定したフェンス値に基づいて、指定のID3D12CommandAllocatorを破棄または再利用可能にします。
    /// </summary>
    /// <param name="fenceValue">GPUフェンスの値。アロケータを破棄または再利用可能と判断する基準となる値。</param>
    /// <param name="allocator">破棄または再利用する対象のID3D12CommandAllocatorへのポインタ。</param>
    void DiscardAllocator(uint64_t fenceValue, ID3D12CommandAllocator* allocator);

    inline size_t Size() { return allocatorPool_.size(); }

private:
    const D3D12_COMMAND_LIST_TYPE commandListType_;

    ID3D12Device* device_;
    std::vector<std::unique_ptr<CommandAllocator>> allocatorPool_;
    std::queue<std::pair<uint64_t, ID3D12CommandAllocator*>> readyAllocators_;
    std::mutex allocatorMutex_;
};

}

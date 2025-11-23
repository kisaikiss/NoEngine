#include "ContextManager.h"

#include "Command/CommandContext.h"

namespace NoEngine {
ContextManager::ContextManager(void) {}
ContextManager::~ContextManager() {}

CommandContext* ContextManager::AllocateContext(D3D12_COMMAND_LIST_TYPE type) {
    std::lock_guard<std::mutex> LockGuard(contextAllocationMutex_);

    auto& availableContexts = availableContexts_[type];

    CommandContext* ret = nullptr;
    if (availableContexts.empty()) {
        ret = new CommandContext(type);
        contextPool_[type].emplace_back(ret);
        ret->Initialize();
    } else {
        ret = availableContexts.front();
        availableContexts.pop();
        ret->Reset();
    }
    assert(ret != nullptr);

    assert(ret->type_ == type);

    return ret;
}
void ContextManager::FreeContext(CommandContext* usedContext) {
    assert(usedContext != nullptr);
    std::lock_guard<std::mutex> LockGuard(contextAllocationMutex_);
    availableContexts_[usedContext->type_].push(usedContext);
}
void ContextManager::DestroyAllContexts() {
    for (uint32_t i = 0; i < kContextNum; ++i) {
        contextPool_[i].clear();
    }
}
}
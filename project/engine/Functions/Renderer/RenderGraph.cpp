#include "stdafx.h"
#include "RenderGraph.h"
#include "engine/Utilities/Conversion/ConvertString.h"

namespace NoEngine {
ColorBuffer* RenderGraphRegistry::CreateColorBuffer(const std::string& name, uint32_t width, uint32_t height, uint32_t depthOrArraySize, DXGI_FORMAT format) {
    colorBuffers_[name].Create(ConvertString(name), width, height, depthOrArraySize, 1, format);
    colorBufferDescs_.push_back({ name, depthOrArraySize, format });
    return &colorBuffers_[name];
}
ColorBuffer* RenderGraphRegistry::CreateColorBuffer(const std::string& name, float width, float height, uint32_t depthOrArraySize, DXGI_FORMAT format) {
    return CreateColorBuffer(name, static_cast<uint32_t>(width), static_cast<uint32_t>(height), depthOrArraySize, format);
}
ColorBuffer* RenderGraphRegistry::CreateColorBuffer(const std::string& name, uint32_t width, uint32_t height, DXGI_FORMAT format) {
    return CreateColorBuffer(name, width, height, 1, format);
}
ColorBuffer* RenderGraphRegistry::CreateColorBuffer(const std::string& name, float width, float height, DXGI_FORMAT format) {
    return CreateColorBuffer(name, static_cast<uint32_t>(width), static_cast<uint32_t>(height), format);
}

DepthBuffer* RenderGraphRegistry::CreateDepthBuffer(const std::string& name, uint32_t width, uint32_t height, DXGI_FORMAT format) {
    depthBuffers_[name] = DepthBuffer(1.f);
    depthBuffers_[name].Create(ConvertString(name), width, height, 1, format);
    depthBufferDescs_.push_back({ name, format });
    return &depthBuffers_[name];
}
DepthBuffer* RenderGraphRegistry::CreateDepthBuffer(const std::string& name, float width, float height, DXGI_FORMAT format) {
    return CreateDepthBuffer(name, static_cast<uint32_t>(width), static_cast<uint32_t>(height), format);
}

void RenderGraphRegistry::ResizeAll(uint32_t width, uint32_t height) {
    for (auto& desc : colorBufferDescs_) {
        // 既にRTV/SRV/UAVのディスクリプタスロットが割り当て済みのオブジェクトに
        // 対して再度Createすると、ColorBuffer::CreateDerivedViews内の
        // 「srvHandle_.ptr が UNKNOWN でなければ再アロケートしない」判定により
        // 同じディスクリプタスロットを再利用したまま中身のリソースだけ差し替わる。
        // → ImGuiのImTextureID（sSceneTexture等）を再取得する必要がない
        colorBuffers_[desc.name].Create(ConvertString(desc.name), width, height, desc.depthOrArraySize, 1, desc.format);
    }
    for (auto& desc : depthBufferDescs_) {
        depthBuffers_[desc.name].Create(ConvertString(desc.name), width, height, 1, desc.format);
    }
}
}

#include "stdafx.h"
#include "RenderResourceManager.h"
#include "engine/Utilities/Conversion/ConvertString.h"

namespace NoEngine {
ColorBuffer* RenderResourceManager::Create(const std::string& name, uint32_t width, uint32_t height, DXGI_FORMAT format) {
    colorBuffers_[name].Create(ConvertString(name), width, height, 1, format);
    return &colorBuffers_[name];
}

ColorBuffer* RenderResourceManager::Create(const std::string& name, float width, float height, DXGI_FORMAT format) {
    return Create(name, static_cast<uint32_t>(width), static_cast<uint32_t>(height), format);
}

ColorBuffer* RenderResourceManager::GetColorBuffer(const std::string& name) {
    auto it = colorBuffers_.find(name);
    return it != colorBuffers_.end() ? &it->second : nullptr;
}
}
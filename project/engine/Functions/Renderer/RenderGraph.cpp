#include "stdafx.h"
#include "RenderGraph.h"
#include "engine/Utilities/Conversion/ConvertString.h"

namespace NoEngine {

ColorBuffer* NoEngine::RenderGraphRegistry::Create(const std::string& name, uint32_t width, uint32_t height, DXGI_FORMAT format) {
    resources_[name].Create(ConvertString(name), width, height, 1, format);
    return &resources_[name];
}
ColorBuffer* RenderGraphRegistry::Create(const std::string& name, float width, float height, DXGI_FORMAT format) {
    return Create(name, static_cast<uint32_t>(width), static_cast<uint32_t>(height), format);
}
}

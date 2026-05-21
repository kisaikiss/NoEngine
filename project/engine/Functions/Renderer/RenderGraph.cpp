#include "stdafx.h"
#include "RenderGraph.h"
#include "engine/Utilities/Conversion/ConvertString.h"

namespace NoEngine {

ColorBuffer* NoEngine::RenderGraphRegistry::CreateColorBuffer(const std::string& name, uint32_t width, uint32_t height, DXGI_FORMAT format) {
    colorBuffers_[name].Create(ConvertString(name), width, height, 1, format);
    return &colorBuffers_[name];
}
ColorBuffer* RenderGraphRegistry::CreateColorBuffer(const std::string& name, float width, float height, DXGI_FORMAT format) {
    return CreateColorBuffer(name, static_cast<uint32_t>(width), static_cast<uint32_t>(height), format);
}
DepthBuffer* RenderGraphRegistry::CreateDepthBuffer(const std::string& name, uint32_t width, uint32_t height, DXGI_FORMAT format) {
    depthBuffers_[name] = DepthBuffer(1.f);
    depthBuffers_[name].Create(ConvertString(name), width, height, 1, format);
    return &depthBuffers_[name];
}
DepthBuffer* RenderGraphRegistry::CreateDepthBuffer(const std::string& name, float width, float height, DXGI_FORMAT format) {
    return CreateDepthBuffer(name, static_cast<uint32_t>(width), static_cast<uint32_t>(height), format);
}
}

#include "GraphicsContext.h"

#include "../GpuResource/PixelBuffer/ColorBuffer.h"

namespace NoEngine {
void GraphicsContext::ClearColor(ColorBuffer& target) {
	FlushResourceBarriers();
	const Color& colorVec = target.GetClearColor();
	float color[] = { colorVec.r, colorVec.g, colorVec.b, colorVec.a };
	commandList_->ClearRenderTargetView(target.GetRTV(), color, 0, nullptr);
}
void GraphicsContext::ClearColor(ColorBuffer& target, float color[4]) {
	FlushResourceBarriers();
	commandList_->ClearRenderTargetView(target.GetRTV(), color, 0, nullptr);
}

void GraphicsContext::SetRenderTargets(UINT NumRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[]) {
	commandList_->OMSetRenderTargets(NumRTVs, RTVs, FALSE, nullptr);
}
void GraphicsContext::SetRenderTargets(UINT NumRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[], D3D12_CPU_DESCRIPTOR_HANDLE DSV) {
	commandList_->OMSetRenderTargets(NumRTVs, RTVs, FALSE, &DSV);
}
}

#include "GraphicsSwapChain.h"

#include "../GraphicsCore.h"
#include "../Command/CommandListManager.h"

#include "../../Debug/Logger/Log.h"

namespace NoEngine {
namespace Graphics {

namespace {
DXGI_FORMAT SwapChainFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
}

GraphicsSwapChain::GraphicsSwapChain(HWND hwnd, float windowWidth, float windowHeight, uint32_t bufferCount) {
	Initialize(hwnd, static_cast<UINT>(windowWidth), static_cast<UINT>(windowHeight), static_cast<UINT>(bufferCount));
}

GraphicsSwapChain::GraphicsSwapChain(HWND hwnd, uint32_t windowWidth, uint32_t windowHeight, uint32_t bufferCount) {
	Initialize(hwnd, static_cast<UINT>(windowWidth), static_cast<UINT>(windowHeight), static_cast<UINT>(bufferCount));
}

GraphicsSwapChain::~GraphicsSwapChain() {
	Destroy();
}


void GraphicsSwapChain::Resize(float windowWidth, float windowHeight) {
	Resize(static_cast<UINT>(windowWidth), static_cast<UINT>(windowHeight));
}

void GraphicsSwapChain::Resize(UINT windowWidth, UINT windowHegiht) {
	HRESULT hr = swapChain_->ResizeBuffers(
		2,
		windowWidth,
		windowHegiht,
		DXGI_FORMAT_UNKNOWN,
		SwapChainFormat
	);
	if (FAILED(hr)) {
		Log::DebugPrint("SwapChain resize failed", VerbosityLevel::kCritical);
		assert(false);
	}
}

void GraphicsSwapChain::Destroy() {
	swapChain_.Reset();
}

void GraphicsSwapChain::Initialize(HWND hwnd, UINT windowWidth, UINT windowHeight, UINT bufferCount) {
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = windowWidth;
	swapChainDesc.Height = windowHeight;
	swapChainDesc.Format = SwapChainFormat;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = bufferCount;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
	fsSwapChainDesc.Windowed = TRUE;


	HRESULT hr = GraphicsCore::gGraphicsInfrastructures->GetDXGIFactory()->
		CreateSwapChainForHwnd(
			GraphicsCore::gCommandListManager.GetCommandQueue(),
			hwnd,
			&swapChainDesc,
			&fsSwapChainDesc,
			nullptr,
			reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf()));
	if (FAILED(hr)) {
		Log::DebugPrint("SwapChain.create.failed", VerbosityLevel::kCritical);
		assert(false);
	}
}

}
}
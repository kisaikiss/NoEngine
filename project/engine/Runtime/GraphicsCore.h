#pragma once
#include "Graphics/GraphicsInfrastructures.h"
#include "Graphics/GraphicsDevice.h"
#include "DescriptorHeap/DescriptorAllocator.h"
#include "Command/CommandListManager.h"
#include "../Window/WindowManager.h"
#include "ContextManager.h"

namespace NoEngine {
/// <summary>
/// Direct3D 12のグラフィックス関連の基盤
/// </summary>
namespace GraphicsCore {
	void Initialize();
	void Shutdown(void);

	extern std::unique_ptr<Graphics::GraphicsInfrastructures> gGraphicsInfrastructures;
	extern std::unique_ptr<Graphics::GraphicsDevice> gGraphicsDevice;
	extern CommandListManager gCommandListManager;
	extern ContextManager gContextManager;
	extern WindowManager gWindowManager;

	// ディスクリプタアロケータ(ディスクリプタのメモリ管理)配列。数はD3D12_DESCRIPTOR_HEAP_TYPE
	extern DescriptorAllocator gDescriptorAllocator[];

	/// <summary>
	/// デバッグレイヤーを有効化
	/// </summary>
	void EnableDebugLayer();

	/// <summary>
	/// デバッグレイヤーの設定
	/// </summary>
	void SettingDebugLayer();

	/// <summary>
	/// ディスクリプタを割り当てます。
	/// </summary>
	/// <param name="Type">ディスクリプタヒープのタイプ</param>
	/// <param name="Count">割り当てる数</param>
	/// <returns>割り当てたディスクリプタのハンドル</returns>
	inline D3D12_CPU_DESCRIPTOR_HANDLE AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT Count = 1) {
		return gDescriptorAllocator[Type].Allocate(Count);
	}
}
}

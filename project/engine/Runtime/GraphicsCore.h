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
class GraphicsCore {
public:
	static void Initialize(float windowWidth = 1280.f, float windowHeight = 720.f);
	static void Shutdown(void);

	static std::unique_ptr<Graphics::GraphicsInfrastructures> sGraphicsInfrastructures;
	static std::unique_ptr<Graphics::GraphicsDevice> sGraphicsDevice;
	static CommandListManager sCommandListManager;
	static ContextManager sContextManager;
	static WindowManager sWindowManager;

	// ディスクリプタアロケータ(ディスクリプタのメモリ管理)配列。数はD3D12_DESCRIPTOR_HEAP_TYPE
	static DescriptorAllocator sDescriptorAllocator[];

	/// <summary>
	/// デバッグレイヤーを有効化
	/// </summary>
	static void EnableDebugLayer();

	/// <summary>
	/// デバッグレイヤーの設定
	/// </summary>
	static void SettingDebugLayer();

	/// <summary>
	/// フレーム開始
	/// </summary>
	/// <param name="context">グラフィックス用のコマンドリストラッパークラス</param>
	static void StartFrame(GraphicsContext& context);

	/// <summary>
	/// フレーム終了
	/// </summary>
	/// <param name="context">グラフィックス用のコマンドリストラッパークラス</param>
	static void EndFrame(GraphicsContext& context);

	/// <summary>
	/// ディスクリプタを割り当てます。
	/// </summary>
	/// <param name="Type">ディスクリプタヒープのタイプ</param>
	/// <param name="Count">割り当てる数</param>
	/// <returns>割り当てたディスクリプタのハンドル</returns>
	static inline D3D12_CPU_DESCRIPTOR_HANDLE AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT Count = 1) {
		return sDescriptorAllocator[Type].Allocate(Count);
	}
private:
	/// <summary>
	/// 描画用のバッファ作成
	/// </summary>
	static void CreatePixelBuffer();
	
	/// <summary>
	/// 描画用バッファの破棄
	/// </summary>
	static void DestroyPixelBuffer();

	/// <summary>
	/// 最終的なフルスクリーン描画
	/// </summary>
	static void FullScreenDraw(GraphicsContext& context);

	static void InitPostEffect();
};
}

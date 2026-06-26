#pragma once
#include "Graphics/GraphicsInfrastructures.h"
#include "Graphics/GraphicsDevice.h"
#include "DescriptorHeap/DescriptorAllocator.h"
#include "Command/CommandListManager.h"
#include "../Window/WindowManager.h"
#include "ContextManager.h"
#include "engine/Math/Types/Vector2.h"

namespace NoEngine {
/// <summary>
/// Direct3D 12のグラフィックス関連の基盤
/// </summary>
class GraphicsCore {
public:
	GraphicsCore(const GraphicsCore& obj) = delete;
	GraphicsCore& operator=(const GraphicsCore& obj) = delete;

	static void Initialize(float windowWidth = 1280.f, float windowHeight = 720.f);
	static void Shutdown(void);

	static std::unique_ptr<Graphics::GraphicsInfrastructures> sGraphicsInfrastructures;
	static std::unique_ptr<Graphics::GraphicsDevice> sGraphicsDevice;
	static CommandListManager sCommandListManager;
	static ContextManager sContextManager;
	static WindowManager sWindowManager;

	static Math::Vector2 GetWindowSize();

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
	static void EndFrame(GraphicsContext& context, ColorBuffer& finalColor);

	/// <summary>
	/// ディスクリプタを割り当てます。
	/// </summary>
	/// <param name="Type">ディスクリプタヒープのタイプ</param>
	/// <param name="Count">割り当てる数</param>
	/// <returns>割り当てたディスクリプタのハンドル</returns>
	static inline D3D12_CPU_DESCRIPTOR_HANDLE AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT Count = 1) {
		return sDescriptorAllocator[Type].Allocate(Count);
	}


	/// <summary>
	/// レイトレーシングが有効かどうかを判定する静的関数です。
	/// </summary>
	/// <returns>レイトレーシングが有効な場合は true、無効な場合は false を返します。</returns>
	static bool IsEnableRaytracing();

	/// <summary>
	/// Deviceの状態を確認
	/// </summary>
	/// <returns>true : 正常, false : 異常(RemoveDevice)</returns>
	static void CheckDeviceStatus();
private:
	GraphicsCore() = default;
	~GraphicsCore() = default;

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
	static void FullScreenDraw(GraphicsContext& context, ColorBuffer& finalColor);

	static void InitPostEffect();

	/// <summary>
	/// Raytracingのサポートのチェックを行う
	/// </summary>
	static void CheckRaytracingEnable();
};
}

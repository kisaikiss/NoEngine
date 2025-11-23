#pragma once
#include "Graphics/GraphicsInfrastructures.h"
#include "Graphics/GraphicsDevice.h"

namespace NoEngine {
class CommandListManager;
class ContextManager;

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

	/// <summary>
	/// デバッグレイヤーを有効化
	/// </summary>
	void EnableDebugLayer();

	/// <summary>
	/// デバッグレイヤーの設定
	/// </summary>
	void SettingDebugLayer();
}
}

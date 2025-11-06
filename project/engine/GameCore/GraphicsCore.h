#pragma once
#include "Graphics/GraphicsInfrastructures.h"
#include "Graphics/GraphicsDevice.h"

namespace NoEngine {
namespace Graphics {
/// <summary>
/// DirectX12のグラフィックス関連の初期化を行う
/// </summary>
class GraphicsCore {
public:
	void Initialize();
private:
	std::unique_ptr<GraphicsInfrastructures> graphicsInfrastructures_;
	std::unique_ptr<GraphicsDevice> graphicsDevice_;
};
}
}

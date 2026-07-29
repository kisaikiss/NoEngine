#pragma once
#include "../RenderPass.h"

namespace NoEngine {
namespace Render {

class DepthOfFieldPass : public RenderPass {
public:
	void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) override;

	// 合焦距離（ビュー空間Z, 正の値）
	void SetFocusDistance(float focusDistance) { focusDistance_ = focusDistance; }
	// この距離までは合焦とみなす範囲（大きいほどボケ始めが緩やか）
	void SetFocusRange(float focusRange) { focusRange_ = focusRange; }
	// 最大ボケ半径（UV空間, 例: 0.01 = 画面幅の1%）
	void SetMaxCoCRadius(float maxCoCRadius) { maxCoCRadius_ = maxCoCRadius; }

private:
	float focusDistance_ = 30.0f;
	float focusRange_ = 20.0f;
	float maxCoCRadius_ = 0.01f;
};

}
}

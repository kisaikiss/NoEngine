#pragma once
#include "engine/Functions/Renderer/RenderPass/RenderPass.h"
#include "application/TD2_3/Component/BallTrailComponent.h"
#include "engine/Math/Types/Vector4.h"

#include <vector>


class BallTrailPass final : public NoEngine::Render::RenderPass
{
public:
	BallTrailPass();
	~BallTrailPass();

	void Execute(NoEngine::GraphicsContext& gfx, NoEngine::ECS::Registry& registry) override;

private:
	struct TrailVertex
	{
		NoEngine::Vector4 position; // ワールド位置.xyz, w unused
		NoEngine::Color color;
		float age;        // サンプルの経過時間
	};
	std::vector<TrailVertex> vertices_;
	std::vector<uint16_t> indices_;

	void CollectAndGenerate(NoEngine::ECS::Registry& registry);
};

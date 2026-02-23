#include "GridRenderSystem.h"
#include "../Component/GridCellComponent.h"
#include "engine/Functions/Renderer/Primitive.h"

void GridRenderSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);// 未使用パラメータ警告回避

	auto view = registry.View<GridCellComponent>();

	// 赤色の線
	No::Color lineColor = { 1.0f, 0.0f, 0.0f, 1.0f };

	for (auto entity : view) {
		auto* cell = registry.GetComponent<GridCellComponent>(entity);

		No::Vector3 nodePos = GridToWorld(cell->gridX, cell->gridY);

		// 右接続があれば右方向に線を描画
		if (cell->hasConnectionRight) {
			No::Vector3 rightPos = GridToWorld(cell->gridX + 1, cell->gridY);
			NoEngine::Primitive::DrawLine(nodePos, rightPos, lineColor);
		}

		// 下接続があれば下方向に線を描画
		// （上接続は上のノードが描画するので、重複を避けるため下のみ）
		if (cell->hasConnectionDown) {
			No::Vector3 downPos = GridToWorld(cell->gridX, cell->gridY - 1);
			NoEngine::Primitive::DrawLine(nodePos, downPos, lineColor);
		}
	}
}

No::Vector3 GridRenderSystem::GridToWorld(int x, int y) {
	return No::Vector3{
		static_cast<float>(x),
		static_cast<float>(y),
		0.0f
	};
}

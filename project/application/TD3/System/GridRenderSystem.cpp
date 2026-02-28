#include "GridRenderSystem.h"
#include "../Component/GridCellComponent.h"
#include "../Utility/GridUtils.h"
#include "engine/Functions/Renderer/Primitive.h"

void GridRenderSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime); // 未使用パラメータ警告回避

	auto view = registry.View<GridCellComponent>();

	// 通常道: 黒、敵専用道: 赤
	No::Color normalColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	No::Color enemyOnlyColor = { 1.0f, 0.0f, 0.0f, 1.0f };

	for (auto entity : view) {
		auto* cell = registry.GetComponent<GridCellComponent>(entity);

		No::Vector3 nodePos = GridUtils::GridToWorld(cell->gridX, cell->gridY);

		// 右接続があれば右方向に線を描画
		if (cell->hasConnectionRight) {
			No::Vector3 rightPos = GridUtils::GridToWorld(cell->gridX + 1, cell->gridY);
			// 自分または隣が敵専用ノードならば赤で描画
			bool isEnemyEdge = cell->isEnemyOnly;
			if (!isEnemyEdge) {
				// 隣のノードも確認
				auto* rightCell = GridUtils::GetGridCell(registry, cell->gridX + 1, cell->gridY);
				if (rightCell && rightCell->isEnemyOnly) isEnemyEdge = true;
			}
			NoEngine::Primitive::DrawLine(nodePos, rightPos, isEnemyEdge ? enemyOnlyColor : normalColor);
		}

		// 下接続があれば下方向に線を描画
		// （上接続は上のノードが描画するので、重複を避けるため下のみ）
		if (cell->hasConnectionDown) {
			No::Vector3 downPos = GridUtils::GridToWorld(cell->gridX, cell->gridY - 1);
			bool isEnemyEdge = cell->isEnemyOnly;
			if (!isEnemyEdge) {
				auto* downCell = GridUtils::GetGridCell(registry, cell->gridX, cell->gridY - 1);
				if (downCell && downCell->isEnemyOnly) isEnemyEdge = true;
			}
			NoEngine::Primitive::DrawLine(nodePos, downPos, isEnemyEdge ? enemyOnlyColor : normalColor);
		}
	}
}
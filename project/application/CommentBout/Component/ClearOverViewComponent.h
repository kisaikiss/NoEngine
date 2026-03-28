#pragma once
#include "engine/NoEngine.h"
#include <array>

/// <summary>
/// ClearOverViewSystem が生成した UI スプライトエンティティを保持するコンポーネント。
/// ClearOverStateComponent と同一エンティティに付与する。
/// </summary>
struct ClearOverViewComponent {
	No::Entity fadeEntity  = No::nullEntity;  // 暗幕オーバーレイ
	No::Entity logoEntity  = No::nullEntity;  // クリア/オーバーロゴ
	std::array<No::Entity, 2> itemEntities{ No::nullEntity, No::nullEntity };  // [0]=リスタート [1]=タイトル
};

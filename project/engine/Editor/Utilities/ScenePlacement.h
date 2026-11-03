#pragma once
#include "engine/Functions/ECS/Registry.h"
#include "engine/Math/Types/Vector4.h"
#include <span>

namespace NoEngine::Editor {

// 回転・拡縮済みの頂点が接地面を突き抜けないよう、原点の配置位置を求める。
bool FindGroundedTranslation(std::span<const Math::Vector3> vertices,
	const Math::Vector3& contact, const Math::Vector3& normal, Math::Vector3& translation);

// Sceneの画像を登録した直後に呼び、ドラッグ中のプレビューと配置確定を処理する。
void DrawScenePlacement(ECS::Registry& registry, const Math::Vector4& sceneRect);
bool IsScenePlacementActive();

}

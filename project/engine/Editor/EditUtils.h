#pragma once
#include "Reflection.h"
#include "engine/Functions/ECS/Registry.h"

namespace NoEngine {
namespace Editor {
struct EditTag {
	std::string name;
	bool isDrawHierarchy = true;
};
/// <summary>
/// ImGuiウィンドウに2Dカメラで描画したシーンでのマウスポインタのワールド座標を取得します
/// </summary>
/// <param name="registry">ECSレジストリ</param>
/// <returns>マウスポインタのワールド座標</returns>
Math::Vector2 Get2DSceneMousePosition(ECS::Registry& registry);
bool IsMouseOverSceneWindow();

/// <summary>
/// 2Dグリッドを描画する。
/// </summary>
/// <param name="gridSize">グリッドの幅と高さを表す2次元ベクトル。</param>
void DrawGrid2D(Math::Vector2 gridSize);
}

void DrawComponentUI(ECS::Registry& registry, ECS::Entity e);
void DrawFieldUI(const FieldInfo& field, void* ptr);
}
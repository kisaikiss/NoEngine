#pragma once
#include "externals/nlohmann/json.hpp"
#include "engine/Functions/ECS/Registry.h"
#include "../ComponentRegistry.h"

namespace NoEngine {
namespace Editor {

// Entityとコンポーネントのプリセット保存と読み込みを行う関数群

/// <summary>
/// プリセット保存を行います
/// </summary>
/// <param name="registry">ECSレジストリ</param>
/// <param name="e">ECSエンティティ</param>
void SavePreset(ECS::Registry& registry, ECS::Entity e);

/// <summary>
/// 指定したプリセットのインスタンス化を行います
/// </summary>
/// <param name="registry">ECSレジストリ</param>
/// <param name="presetName">プリセット名</param>
/// <returns>インスタンス化したプリセットのECSエンティティ</returns>
ECS::Entity InstantiatePreset(ECS::Registry& registry, const std::string& presetPath);

/// <summary>
/// ディレクトリからすべてのPrefabを読み込みます。
/// </summary>
void LoadPrefabsFromDirectory();

/// <summary>
/// ImGuiでPrefabWindowを描画します。Releaseビルドでは何もしません
/// </summary>
/// <param name="registry">ECSレジストリ</param>
void DrawPrefabWindow(ECS::Registry& registry);
}
}
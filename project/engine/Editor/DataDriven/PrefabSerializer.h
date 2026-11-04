#pragma once
#include "externals/nlohmann/json.hpp"
#include "engine/Functions/ECS/Registry.h"
#include "../Reflection/ComponentRegistry.h"

namespace NoEngine {
namespace Editor {

struct PrefabInfo {
	std::string name;
	std::string path;
};

// 専用シリアライザで保存し、通常のComponent UIからは編集させない。
struct PrefabInstanceTag {
	std::string prefabPath;
	// 最後に反映したPrefabの値。現在値との差から個別変更を判定する。
	nlohmann::json base;
};

void LinkPrefabInstance(ECS::Registry& registry, ECS::Entity entity, const std::string& path);
// 差分はJSON Pointerのパスで識別する。配列はフィールド全体を一つの差分とする。
nlohmann::json GetPrefabOverrides(ECS::Registry& registry, ECS::Entity entity);
nlohmann::json GetPrefabOverrides(const nlohmann::json& base, const nlohmann::json& current);
// fieldPathが空なら全差分を対象にする。名前と階層は常に保持する。
void RevertPrefabOverrides(ECS::Registry& registry, ECS::Entity entity, const std::string& fieldPath = "");
void ApplyInstanceOverrides(ECS::Registry& registry, ECS::Entity entity, const std::string& fieldPath = "");
void RefreshPrefabInstance(ECS::Registry& registry, ECS::Entity entity);
void SavePrefabFile(const std::string& path, const nlohmann::json& data);

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

const std::vector<PrefabInfo>& GetPrefabs();

/// <summary>
/// ImGuiでPrefabWindowを描画します。Releaseビルドでは何もしません
/// </summary>
/// <param name="registry">ECSレジストリ</param>
void DrawPrefabWindow(ECS::Registry& registry);

ECS::Entity LoadPrefabForEditing(ECS::Registry& registry, const std::string& prefabPath);
void ApplyPrefabToInstances(ECS::Registry& registry, const std::string& prefabPath, const nlohmann::json& prefabJson);
void SetEditPrefabCallback(std::function<void(ECS::Registry&, const std::string&)> cb);
void RequestEditPrefab(ECS::Registry& registry, const std::string& prefabPath);
}
}

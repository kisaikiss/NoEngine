#include "stdafx.h"
#include "PrefabSerializer.h"
#include "SceneSerializer.h"
#include "engine/Functions/ECS/Component/Common/TransformComponent.h"
#include "../ReflectionMacros.h"
#include <functional>

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI


REFLECT_STRUCT_BEGIN(NoEngine::Editor::PrefabInstanceTag)
REFLECT_FIELD(prefabPath)
REFLECT_STRUCT_END(NoEngine::Editor::PrefabInstanceTag)
namespace NoEngine {
using namespace ECS;

namespace Editor {
namespace {
std::vector<PrefabInfo> sPrefabs;


const std::string sFilePath = "resources/game/Prefabs";

// Prefab一覧側(DrawPrefabWindowなど)からの「このPrefabを編集したい」という要求を、
// EditSystem側が受け取れるようにするためのコールバック。
// EditUtils.cppのsGizmoCallbackと同じ考え方。
std::function<void(ECS::Registry&, const std::string&)> sEditPrefabCallback;

}

void SavePreset(ECS::Registry& registry, ECS::Entity e) {
	if (!registry.Has<Editor::EditTag>(e)) {
		LogError("プリセットを保存するエンティティに有効な名前が付けられていません。");
		return;
	}
	std::filesystem::create_directory(sFilePath);
	nlohmann::json j;
	auto* tag = registry.GetComponent<Editor::EditTag>(e);
	std::string presetPath = sFilePath + "/" + tag->name + ".json";
	j = SaveEntityToJson(registry, e);

	std::ofstream ofs(presetPath);
	ofs << j.dump(4);
	LoadPrefabsFromDirectory();
}

Entity InstantiatePreset(ECS::Registry& registry, const std::string& presetPath) {
	nlohmann::json j;
	std::ifstream ifs(presetPath);
	ifs >> j;
	Entity root = registry.GenerateEntity();
	LoadEntityFromJson(registry, root, j);

	// Prefabから生成された場合、"_Prefab"と名前についているので外しておく
	auto& name = registry.GetComponent<EditTag>(root)->name;
	size_t x = name.find("_Prefab");
	if (x != std::string::npos) {
		name.erase(x);
	}

	// このEntityがどのPrefabから配置されたかを記録しておく。
	// Prefab編集時にシーン上の同じPrefab由来のEntityへ変更を反映するために使う。
	auto* prefabTag = registry.AddComponent<PrefabInstanceTag>(root);
	prefabTag->prefabPath = presetPath;

	return root;
}

Entity LoadPrefabForEditing(ECS::Registry& registry, const std::string& prefabPath) {
	// Prefab自体を「編集対象の実体」としてロードする。
	// InstantiatePresetと違い、これは配置物ではなくPrefab本体の編集なので
	// PrefabInstanceTagは付けない。
	nlohmann::json j;
	std::ifstream ifs(prefabPath);
	ifs >> j;
	Entity e = registry.GenerateEntity();
	LoadEntityFromJson(registry, e, j);
	auto& name = registry.GetComponent<EditTag>(e)->name;
	if (name.find("_Prefab") == std::string::npos) {
		name += "_Prefab";
	}

	return e;
}

void ApplyPrefabToInstances(ECS::Registry& registry, const std::string& prefabPath, const nlohmann::json& prefabJson) {
	// PrefabInstanceTag.prefabPath が一致するEntityすべてに、保存されたPrefabの内容を反映する。
	// Transformは各配置ごとの位置・回転・スケールを保ちたいので、退避してから戻す。
	auto view = registry.View<PrefabInstanceTag>();
	for (auto e : view) {
		auto* tag = registry.GetComponent<PrefabInstanceTag>(e);
		if (!tag || tag->prefabPath != prefabPath) continue;

		Component::TransformComponent savedTransform{};
		bool hasTransform = false;
		if (auto* t = registry.GetComponent<Component::TransformComponent>(e)) {
			savedTransform = *t;
			hasTransform = true;
		}

		LoadEntityFromJson(registry, e, prefabJson);

		if (hasTransform) {
			if (auto* t = registry.GetComponent<Component::TransformComponent>(e)) {
				*t = savedTransform;
			}
		}

		// "Prefab"を外す
		auto& name = registry.GetComponent<EditTag>(e)->name;
		size_t x = name.find("_Prefab");
		if (x != std::string::npos) {
			name.erase(x);
		}
	}
}

void SetEditPrefabCallback(std::function<void(ECS::Registry&, const std::string&)> cb) {
	sEditPrefabCallback = std::move(cb);
}

void RequestEditPrefab(ECS::Registry& registry, const std::string& prefabPath) {
	if (sEditPrefabCallback) {
		sEditPrefabCallback(registry, prefabPath);
	}
}

void LoadPrefabsFromDirectory() {
	sPrefabs.clear();
	// Prefabフォルダー内をクロール
	for (auto& p : std::filesystem::directory_iterator(sFilePath)) {
		if (!p.is_regular_file()) continue;
		auto ext = p.path().extension().string();
		if (ext != ".json") continue;
		PrefabInfo info;
		info.path = p.path().string();
		info.name = p.path().stem().string();

		sPrefabs.push_back(std::move(info));
	}
}

const std::vector<PrefabInfo>& GetPrefabs() {
	return sPrefabs;
}

void DrawPrefabWindow(ECS::Registry& registry) {
#ifdef USE_IMGUI
	ImGui::Begin("Prefabs");
	for (auto& prefab : sPrefabs) {
		ImGui::PushID(prefab.path.c_str());
		ImGui::Selectable(prefab.name.c_str());
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
			// ペイロードは null 終端文字列を渡す
			ImGui::SetDragDropPayload("PREFAB_PATH", prefab.path.c_str(), prefab.path.size() + 1);
			ImGui::Text("Place %s", prefab.name.c_str());
			ImGui::EndDragDropSource();
		}
		ImGui::SameLine();
		if (ImGui::SmallButton("Edit")) {
			// 実際の編集処理はEditSystem側に委譲する（ComponentUIの編集関数を再利用するため）
			RequestEditPrefab(registry, prefab.path);
		}
		ImGui::PopID();
	}
	ImGui::End();
#else
	static_cast<void>(registry);
#endif // USE_IMGUI

}
}
}
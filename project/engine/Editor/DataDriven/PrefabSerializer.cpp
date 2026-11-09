#include "stdafx.h"
#include "PrefabSerializer.h"
#include "SceneSerializer.h"
#include <chrono>
#include <system_error>

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI


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

using Json = nlohmann::json;

Json PrefabData(const Json& value) {
	Json result = { {"components", value.value("components", Json::object())} };
	auto& components = result["components"];
	components.erase("EditTag");
	components.erase("PrefabInstanceTag");
	for (const auto* name : { "TransformComponent", "Transform2DComponent" }) {
		if (components.contains(name)) components[name].erase("parent");
	}
	return result;
}

Json Definition(Json data) {
	data.erase("prefabInstance");
	data.erase("prefabBase");
	data.erase("prefabOverrides");
	data["components"].erase("PrefabInstanceTag");
	return data;
}

Json ReadPrefab(const std::string& path) {
	std::ifstream input(path);
	if (!input) throw std::runtime_error("Prefabを開けません: " + path);
	Json data; input >> data;
	return Definition(std::move(data));
}

Json OverlayFields(Json defaults, const Json& fields) {
	if (!defaults.is_object() || !fields.is_object()) return fields;
	for (const auto& [key, value] : fields.items()) {
		defaults[key] = defaults.contains(key) ? OverlayFields(defaults[key], value) : value;
	}
	return defaults;
}

Json CanonicalData(const Json& definition) {
	ECS::Registry scratch;
	const auto entity = scratch.GenerateEntity();
	LoadEntityFromJson(scratch, entity, Definition(definition));
	// シーン内のEntity参照名は、一時Registryで解決できなくても元の文字列を保持する。
	return OverlayFields(PrefabData(SaveEntityToJson(scratch, entity)), PrefabData(definition));
}

std::string EscapeToken(const std::string& key) {
	std::string result;
	for (char c : key) result += c == '~' ? "~0" : c == '/' ? "~1" : std::string(1, c);
	return result;
}

void CollectChanges(const Json& base, const Json& current, const std::string& path, Json& changes) {
	if (base == current) return;
	if (base.is_object() && current.is_object()) {
		for (const auto& [key, oldValue] : base.items()) {
			const auto childPath = path + "/" + EscapeToken(key);
			if (!current.contains(key)) changes.push_back({ {"op", "remove"}, {"path", childPath} });
			else CollectChanges(oldValue, current[key], childPath, changes);
		}
		for (const auto& [key, value] : current.items()) {
			if (!base.contains(key)) changes.push_back({ {"op", "add"}, {"path", path + "/" + EscapeToken(key)}, {"value", value} });
		}
	} else {
		// 配列は要素数と順序も含めた一つのフィールドとして保持する。
		changes.push_back({ {"op", "replace"}, {"path", path}, {"value", current} });
	}
}

Json MergeChanges(Json target, const Json& changes, const Json& current, const std::string& selected = "") {
	for (const auto& change : changes) {
		const std::string path = change.at("path");
		if (!selected.empty() && selected != path) continue;
		const Json::json_pointer pointer(path);
		if (change.at("op") == "remove") {
			const auto parent = pointer.parent_pointer();
			if (target.contains(parent) && target[parent].is_object()) target[parent].erase(pointer.back());
		} else {
			// 元Prefabから親コンポーネントが削除されても、個別変更した内容は失わない。
			std::vector<Json::json_pointer> parents;
			auto parent = pointer.parent_pointer();
			while (!parent.empty()) { parents.push_back(parent); parent = parent.parent_pointer(); }
			for (auto i = parents.rbegin(); i != parents.rend(); ++i) {
				if (!target.contains(*i) || !target[*i].is_object()) target[*i] = current.contains(*i) ? current[*i] : Json::object();
			}
			target[pointer] = change.at("value");
		}
	}
	return target;
}

bool SamePrefab(const std::string& a, const std::string& b) {
	return std::filesystem::path(a).lexically_normal() == std::filesystem::path(b).lexically_normal();
}

void EnsureBase(ECS::Registry& registry, Entity entity) {
	auto* tag = registry.GetComponent<PrefabInstanceTag>(entity);
	if (tag && tag->base.is_null()) tag->base = CanonicalData(ReadPrefab(tag->prefabPath));
}

void LoadInstanceData(ECS::Registry& registry, Entity entity, Json data, const Json& base) {
	const auto snapshot = SaveEntityToJson(registry, entity);
	const auto path = registry.GetComponent<PrefabInstanceTag>(entity)->prefabPath;
	// 名前と階層はインスタンス固有の情報であり、Apply/Revertの対象にしない。
	if (snapshot["components"].contains("EditTag")) data["components"]["EditTag"] = snapshot["components"]["EditTag"];
	for (const auto* name : { "TransformComponent", "Transform2DComponent" }) {
		if (!data["components"].contains(name)) continue;
		if (snapshot["components"].contains(name) && snapshot["components"][name].contains("parent")) {
			data["components"][name]["parent"] = snapshot["components"][name]["parent"];
		} else if (snapshot["components"].contains("EditTag")) {
			data["components"][name]["parent"] = snapshot["components"]["EditTag"].value("parent", Json());
		}
	}
	data["prefabInstance"] = path;
	data["prefabBase"] = base;
	LoadEntityFromJson(registry, entity, data, true);
}

void UpdateInstance(ECS::Registry& registry, Entity entity, const Json& nextBase) {
	EnsureBase(registry, entity);
	const auto base = registry.GetComponent<PrefabInstanceTag>(entity)->base;
	const auto current = PrefabData(SaveEntityToJson(registry, entity));
	const auto changes = GetPrefabOverrides(base, current);
	LoadInstanceData(registry, entity, MergeChanges(nextBase, changes, current), nextBase);
}

}

void SavePrefabFile(const std::string& path, const nlohmann::json& data) {
	const std::filesystem::path target(path);
	auto temporary = target;
	temporary += "." + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".tmp";
	try {
		std::ofstream output;
		output.exceptions(std::ios::failbit | std::ios::badbit);
		output.open(temporary, std::ios::binary | std::ios::trunc);
		output << Definition(data).dump(4);
		output.close();
		if (!MoveFileExW(temporary.c_str(), target.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
			throw std::system_error(static_cast<int>(GetLastError()), std::system_category());
	} catch (...) {
		std::error_code ignored; std::filesystem::remove(temporary, ignored);
		throw;
	}
}

void LinkPrefabInstance(ECS::Registry& registry, Entity entity, const std::string& path) {
	auto* tag = registry.GetComponent<PrefabInstanceTag>(entity);
	if (!tag) tag = registry.AddComponent<PrefabInstanceTag>(entity);
	tag->prefabPath = path;
	tag->base = PrefabData(SaveEntityToJson(registry, entity));
}

nlohmann::json GetPrefabOverrides(const nlohmann::json& base, const nlohmann::json& current) {
	Json changes = Json::array();
	CollectChanges(base, PrefabData(current), "", changes);
	return changes;
}

nlohmann::json GetPrefabOverrides(ECS::Registry& registry, Entity entity) {
	if (!registry.Has<PrefabInstanceTag>(entity)) return Json::array();
	EnsureBase(registry, entity);
	return GetPrefabOverrides(registry.GetComponent<PrefabInstanceTag>(entity)->base, SaveEntityToJson(registry, entity));
}

void RefreshPrefabInstance(ECS::Registry& registry, Entity entity) {
	const auto* tag = registry.GetComponent<PrefabInstanceTag>(entity);
	if (tag) {
		const auto nextBase = CanonicalData(ReadPrefab(tag->prefabPath));
		UpdateInstance(registry, entity, nextBase);
	}
}

void RevertPrefabOverrides(ECS::Registry& registry, Entity entity, const std::string& fieldPath) {
	EnsureBase(registry, entity);
	const auto* tag = registry.GetComponent<PrefabInstanceTag>(entity);
	if (!tag) return;
	const auto base = CanonicalData(ReadPrefab(tag->prefabPath));
	const auto current = PrefabData(SaveEntityToJson(registry, entity));
	auto changes = GetPrefabOverrides(tag->base, current);
	Json remaining = Json::array();
	if (!fieldPath.empty()) for (const auto& change : changes) if (change.at("path") != fieldPath) remaining.push_back(change);
	LoadInstanceData(registry, entity, MergeChanges(base, remaining, current), base);
}

void ApplyInstanceOverrides(ECS::Registry& registry, Entity entity, const std::string& fieldPath) {
	if (!registry.Has<PrefabInstanceTag>(entity)) throw std::invalid_argument("Prefabインスタンスではありません");
	EnsureBase(registry, entity);
	const auto path = registry.GetComponent<PrefabInstanceTag>(entity)->prefabPath;
	auto definition = ReadPrefab(path);
	const auto latest = CanonicalData(definition);
	const auto current = PrefabData(SaveEntityToJson(registry, entity));
	const auto changes = GetPrefabOverrides(registry.GetComponent<PrefabInstanceTag>(entity)->base, current);
	const auto applied = MergeChanges(latest, changes, current, fieldPath);
	auto editTag = definition["components"].value("EditTag", Json::object());
	definition = applied;
	definition["components"]["EditTag"] = std::move(editTag);
	SavePrefabFile(path, definition);
	ApplyPrefabToInstances(registry, path, definition);
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

	SavePrefabFile(presetPath, j);
	LoadPrefabsFromDirectory();
}

Entity InstantiatePreset(ECS::Registry& registry, const std::string& presetPath) {
	nlohmann::json j;
	std::ifstream ifs(presetPath);
	ifs >> j;
	j = Definition(std::move(j));
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
	LinkPrefabInstance(registry, root, presetPath);

	return root;
}

Entity LoadPrefabForEditing(ECS::Registry& registry, const std::string& prefabPath) {
	// Prefab自体を「編集対象の実体」としてロードする。
	// InstantiatePresetと違い、これは配置物ではなくPrefab本体の編集なので
	// PrefabInstanceTagは付けない。
	nlohmann::json j;
	std::ifstream ifs(prefabPath);
	ifs >> j;
	j = Definition(std::move(j));
	Entity e = registry.GenerateEntity();
	LoadEntityFromJson(registry, e, j);
	auto& name = registry.GetComponent<EditTag>(e)->name;
	if (name.find("_Prefab") == std::string::npos) {
		name += "_Prefab";
	}

	return e;
}

void ApplyPrefabToInstances(ECS::Registry& registry, const std::string& prefabPath, const nlohmann::json& prefabJson) {
	const auto nextBase = CanonicalData(prefabJson);
	for (auto entity : registry.View<PrefabInstanceTag>()) {
		const auto* tag = registry.GetComponent<PrefabInstanceTag>(entity);
		if (tag && SamePrefab(tag->prefabPath, prefabPath)) UpdateInstance(registry, entity, nextBase);
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

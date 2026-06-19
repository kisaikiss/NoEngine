#include "stdafx.h"
#include "PrefabSerializer.h"
#include "SceneSerializer.h"

namespace NoEngine {
using namespace ECS;

namespace Editor {
namespace {
const std::string sFilePath = "resources/game/Prefabs";

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
}

Entity InstantiatePreset(ECS::Registry& registry, const std::string& presetPath) {
	nlohmann::json j;
	std::ifstream ifs(presetPath);
	ifs >> j;
	Entity root = registry.GenerateEntity();
	LoadEntityFromJson(registry, root, j);

	return root;
}
}
}

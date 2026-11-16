#pragma once
#include <unordered_map>
#include <utility>
#include "Reflection.h"
#include "externals/nlohmann/json.hpp"

namespace NoEngine {
// コンポーネント固有の処理は、その型を所有する側から登録する。
struct ComponentHooks {
	std::function<void(ECS::Registry&, ECS::Entity, const TypeInfo&, void*)> drawInspector;
	// 通常のフィールド復元前に、既存値の初期化や旧形式JSONの変換を行う。
	std::function<void(void*, nlohmann::json&)> beforeDeserialize;
};

class ComponentHookRegistry final {
public:
	static void Register(size_t typeId, ComponentHooks hooks) {
		Hooks()[typeId] = std::move(hooks);
	}

	static const ComponentHooks* Find(size_t typeId) {
		const auto& hooks = Hooks();
		const auto it = hooks.find(typeId);
		return it == hooks.end() ? nullptr : &it->second;
	}

private:
	static std::unordered_map<size_t, ComponentHooks>& Hooks() {
		static std::unordered_map<size_t, ComponentHooks> hooks;
		return hooks;
	}
};
}

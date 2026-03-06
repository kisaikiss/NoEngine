#pragma once
#include "Reflection.h"

namespace NoEngine {
/// <summary>
/// コンポーネントの情報を格納するクラス
/// </summary>
class ComponentRegistry final {
public:
	ComponentRegistry(const ComponentRegistry& obj) = delete;
	ComponentRegistry& operator=(const ComponentRegistry& obj) = delete;

	static void Register(const TypeInfo& info) { 
		Get().components_.push_back(info);
	} 
	
	static const std::vector<TypeInfo>& GetAll() { return Get().components_; }
	
	static TypeInfo* FindByName(std::string name) {
		for (auto& comp : Get().components_) {
			if (comp.name == name) {
				return &comp;
			}
		}
		return nullptr;
	}
private:
	std::vector<TypeInfo> components_;
	static ComponentRegistry& Get() {
		static ComponentRegistry instance;
		return instance; 
	}
	ComponentRegistry() = default;
	~ComponentRegistry() = default;
};
}


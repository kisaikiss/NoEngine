#pragma once
#include "engine/Functions/ECS/Entity.h"

namespace NoEngine {
namespace Editor {
struct EditTag {
	std::string name;
	ECS::Entity parent = ECS::INVALID_ENTITY;
};

struct EditSelectedTag {};
// このタグを付けるとシーンクリックで選択できなくなる
struct NoClickSelectTag{};
}
}
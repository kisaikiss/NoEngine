#pragma once
#include "engine/Functions/ECS/Entity.h"

namespace NoEngine {
namespace Editor {
struct EditTag {
	std::string name;
	ECS::Entity parent = ECS::INVALID_ENTITY;
};

struct EditSelectedTag {};
// 代表選択とは別に、Inspectorとギズモで共有する複数選択を保持する。
struct EditMultiSelectedTag {};
// このタグを付けるとシーンクリックで選択できなくなる
struct NoClickSelectTag{};
}
}

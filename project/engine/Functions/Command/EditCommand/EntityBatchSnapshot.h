#pragma once
#include "engine/Functions/ECS/Registry.h"
#include "externals/nlohmann/json.hpp"
#include <vector>

namespace NoEngine {
namespace Command {

struct EntityBatchSnapshotItem {
	ECS::Entity entity = ECS::INVALID_ENTITY;
	nlohmann::json snapshotJson;
	int parentIndexInBatch = -1; // 親もバッチに含まれる場合のインデックス。含まれなければ-1。
};

// 入力順を保ってスナップショットとバッチ内の親子関係を保存する。
std::vector<EntityBatchSnapshotItem> CaptureEntityBatch(ECS::Registry& registry, const std::vector<ECS::Entity>& entities);

// 全件を再生成し、バッチ内の親を新しいIDへ張り直して最後のEntityを選択する。
void RestoreEntityBatch(ECS::Registry& registry, std::vector<EntityBatchSnapshotItem>& items);

}
}

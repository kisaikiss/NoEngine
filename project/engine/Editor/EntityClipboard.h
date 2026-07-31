#pragma once
#include "engine/Functions/ECS/Registry.h"
#include "externals/nlohmann/json.hpp"
#include <vector>

namespace NoEngine {
namespace Editor {

// 選択Entity＋その子孫を丸ごとコピー＆ペーストするためのクリップボード。
// 「親をコピペすると子も全てコピペされる」を、親子関係をインデックス参照でJSON化することで実現する。
class EntityClipboard {
public:
	// root自身とその子孫を再帰的にコピーする
	void Copy(ECS::Registry& registry, ECS::Entity root);

	// コピーした階層をそのまま新規Entityとして貼り付ける。
	// 戻り値は生成された全Entity（先頭がコピー元ルートに対応する新規Entity）。
	std::vector<ECS::Entity> Paste(ECS::Registry& registry);

	bool HasContent() const { return !copyObject_.empty() && copyObject_.contains("nodes"); }

private:
	nlohmann::json copyObject_;
};

}
}

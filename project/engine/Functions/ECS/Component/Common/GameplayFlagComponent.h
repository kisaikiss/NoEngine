#pragma once

namespace NoEngine::Component {
// グラフの所有者は、シーン固有のフラグを保持する。シーンエンティティが再作成されると、これらの値はリセットされる。
struct GameplayFlagComponent {
	std::unordered_map<std::string, bool> values;
};
}

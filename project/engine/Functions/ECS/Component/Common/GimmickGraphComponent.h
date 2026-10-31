#pragma once
#include "engine/Editor/Reflection/ReflectionMacros.h"

namespace NoEngine::Component {
// シーンEntityと別ファイルに置いたギミックグラフJSONへの参照。
// Triggerなどイベントを発行するEntityに付けると、EventLinkSystemがこのパスを実行する。
struct GimmickGraphComponent {
	std::string graphPath;
};
}

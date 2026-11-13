#pragma once

namespace NoEngine::Event {
// 更新中のシステムを破棄しないよう、シーンの更新後に再読み込みを要求する。
struct SceneReloadEvent {};
}

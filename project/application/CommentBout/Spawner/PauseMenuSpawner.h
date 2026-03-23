#pragma once
#include "engine/NoEngine.h"

// Pauseメニュー用のエンティティを生成クラス
// このクラス自身は状態を持たない。
namespace PauseMenuSpawner {
	void Create(No::Registry& registry, const NoEngine::TextureRef& whiteTexture);
}
#pragma once
#include "engine/NoEngine.h"

// GameScene / TitleScene 両方から使う、オプションメニュー用エンティティ生成
// このクラス自身は状態を持たない。
namespace OptionMenuSpawner {
	void Create(No::Registry& registry, const NoEngine::TextureRef& whiteTexture);
}
#pragma once
#include "engine/Assets/Texture/TextureManager.h"

/// <summary>
/// ゲーム全体で共有する描画リソース。
/// 各Systemが使う共通テクスチャを保持する
/// </summary>
struct GameResourceComponent {
	NoEngine::TextureRef whiteTexture;
};

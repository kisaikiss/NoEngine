#pragma once
#include "TextureManager.h"

namespace NoEngine {
bool IsGifTexture(const std::filesystem::path& path);
TextureRef LoadGifFirstFrame(const std::wstring& path);

// 同じGIFのテクスチャを共有し、再生位置はプレイヤーごとに保持する。
class GifTexturePlayer {
public:
	bool Load(const std::wstring& path);
	bool IsLoaded() const { return static_cast<bool>(clip_); }
	TextureRef GetFrame() const;
	void Advance(float deltaTime);

private:
	struct Clip;
	std::shared_ptr<const Clip> clip_;
	double time_ = 0.0;
};
}

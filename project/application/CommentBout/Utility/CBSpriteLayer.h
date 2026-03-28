#pragma once
#include <cstdint>

namespace CommentBout {
	enum class SpriteLayer : uint32_t {
		PlayerAttack = 19,
		Gameplay = 20,
		Effect = 30,
		RailProgressBar = 60,
		HpBar = 90,
		RewardOrb = 91,		//吹き出しエフェクト

		TitleBackground = 880,
		TitlePanel = 885,
		TitleLogo = 890,
		TitleItem = 900,
		TitleCursor = 910,

		PauseDim = 900,
		PauseMenuBackground = 905,
		PausePanelLine = 907,
		PauseTitle = 910,
		PauseItem = 920,
		PauseCursor = 930,

		OptionDim = 940,
		OptionBackground = 945,
		OptionItem = 950,
		OptionLabel = 951,
		OptionCursor = 952,
		OptionBarBase = 955,
		OptionBarFill = 956,
		OptionToggle = 957,
		OptionLine = 958,
		OptionItemOverlay = 959,

		ClearOverFade   = 960,
		ClearOverLogo   = 965,
		ClearOverItem   = 970,
	};

	inline uint32_t ToLayer(SpriteLayer layer) {
		return static_cast<uint32_t>(layer);
	}
}

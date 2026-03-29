#pragma once

struct ClearOverStateComponent {
	enum class Phase {
		Inactive = 0,
		FadeIn,      // 暗幕フェードイン
		LogoAppear,  // ロゴ出現
		MenuAppear,  // メニュー出現
		MenuActive,  // 選択中
	};

	enum class Result { Clear, Over };
	enum class Action { None, Restart, BackToTitle };

	Phase  phase           = Phase::Inactive;
	Result result          = Result::Clear;
	Action requestedAction = Action::None;

	float phaseTimer = 0.f;
	int   selectedIndex = 0;  // 0=リスタート, 1=タイトルへ

	bool  isConfirmAnimating = false;
	float confirmAnimTime    = 0.f;
	int   confirmIndex       = -1;
};

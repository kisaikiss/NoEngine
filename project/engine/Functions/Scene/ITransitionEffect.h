#pragma once
#include "engine/Functions/ECS/Registry.h"

namespace NoEngine {
namespace Scene {

// シーン遷移演出の基底クラス。SceneManagerはphase(FadingOut/FadingIn)とt(0〜1)の
// 進行だけを管理し、実際に何を描画するかはこのクラスの実装に委ねる。
// アプリ側で独自演出を作りたい場合はこれを継承し、
// SceneManager::RegisterTransitionEffect で名前を付けて登録する。
class ITransitionEffect {
public:
	virtual ~ITransitionEffect() = default;

	// FadingOutフェーズ開始時に一度だけ（registryは旧シーンのもの）
	virtual void OnFadingOutStart(ECS::Registry& registry) { static_cast<void>(registry); }
	// FadingOut中、毎フレーム t(0→1)
	virtual void UpdateFadingOut(ECS::Registry& registry, float t) = 0;

	// シーン切替直後、FadingIn開始時に一度だけ（registryは新シーンのもの）
	virtual void OnFadingInStart(ECS::Registry& registry) { static_cast<void>(registry); }
	// FadingIn中、毎フレーム t(0→1)
	virtual void UpdateFadingIn(ECS::Registry& registry, float t) = 0;

	// 遷移完全終了時（後始末用）
	virtual void OnFinished(ECS::Registry& registry) { static_cast<void>(registry); }
};

}
}
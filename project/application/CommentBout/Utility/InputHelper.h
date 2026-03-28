#pragma once
#include "engine/NoEngine.h"

namespace InputHelper {

// --- 定数 ---
static constexpr float kStickDeadZone        = 0.3f;
static constexpr float kStickRepeatFirstDelay = 0.40f;  // 最初のリピートまでの待機秒数
static constexpr float kStickRepeatInterval   = 0.12f;  // 2回目以降のリピート間隔

// --- スティックナビゲーション用リピート状態（1方向分）---
struct StickRepeatAxis {
	bool  wasActive     = false;
	float timer         = 0.0f;
	bool  firedThisFrame = false;
};

struct StickNavState {
	StickRepeatAxis up, down, left, right;
};

inline StickNavState& GetStickNavState() {
	static StickNavState s;
	return s;
}

// 1軸分のリピート更新（isActive: その方向にスティックが倒れているか）
inline void UpdateRepeatAxis(bool isActive, StickRepeatAxis& axis, float deltaTime) {
	axis.firedThisFrame = false;
	if (!isActive) {
		axis.wasActive = false;
		axis.timer     = 0.0f;
		return;
	}
	if (!axis.wasActive) {
		// 入力開始フレーム → 即座に1発
		axis.wasActive      = true;
		axis.timer          = kStickRepeatFirstDelay;
		axis.firedThisFrame = true;
	} else {
		axis.timer -= deltaTime;
		if (axis.timer <= 0.0f) {
			axis.timer          = kStickRepeatInterval;
			axis.firedThisFrame = true;
		}
	}
}

// InputHelperSystem から毎フレーム呼ぶ
inline void Update(float deltaTime) {
	auto& s = GetStickNavState();
	const auto& stick = No::Pad::GetStick();
	// 標準ゲームパッドは Y 軸正方向 = 上。スクリーンは Y 正方向 = 下のため反転。
	UpdateRepeatAxis(stick.leftStickY >  kStickDeadZone, s.up,    deltaTime);
	UpdateRepeatAxis(stick.leftStickY < -kStickDeadZone, s.down,  deltaTime);
	UpdateRepeatAxis(stick.leftStickX < -kStickDeadZone, s.left,  deltaTime);
	UpdateRepeatAxis(stick.leftStickX >  kStickDeadZone, s.right, deltaTime);
}

// --- 入力クエリ ---

inline bool IsConfirmTrigger() {
	return No::Keyboard::IsTrigger(VK_SPACE)
		|| No::Pad::IsTrigger(No::GamepadButton::A);
}

inline bool IsPauseTrigger() {
	return No::Keyboard::IsTrigger(VK_TAB)
		|| No::Pad::IsTrigger(No::GamepadButton::Start);
}

inline bool IsMoveUpTrigger() {
	return No::Keyboard::IsTrigger('W')
		|| No::Keyboard::IsTrigger(VK_UP)
		|| No::Pad::IsTrigger(No::GamepadButton::Up)
		|| GetStickNavState().up.firedThisFrame;
}

inline bool IsMoveDownTrigger() {
	return No::Keyboard::IsTrigger('S')
		|| No::Keyboard::IsTrigger(VK_DOWN)
		|| No::Pad::IsTrigger(No::GamepadButton::Down)
		|| GetStickNavState().down.firedThisFrame;
}

inline bool IsMoveLeftTrigger() {
	return No::Keyboard::IsTrigger('A')
		|| No::Keyboard::IsTrigger(VK_LEFT)
		|| No::Pad::IsTrigger(No::GamepadButton::Left)
		|| GetStickNavState().left.firedThisFrame;
}

inline bool IsMoveRightTrigger() {
	return No::Keyboard::IsTrigger('D')
		|| No::Keyboard::IsTrigger(VK_RIGHT)
		|| No::Pad::IsTrigger(No::GamepadButton::Right)
		|| GetStickNavState().right.firedThisFrame;
}

inline bool IsMoveUpPress() {
	return No::Keyboard::IsPress('W')
		|| No::Keyboard::IsPress(VK_UP)
		|| No::Pad::IsPress(No::GamepadButton::Up)
		|| (No::Pad::GetStick().leftStickY > kStickDeadZone);
}

inline bool IsMoveDownPress() {
	return No::Keyboard::IsPress('S')
		|| No::Keyboard::IsPress(VK_DOWN)
		|| No::Pad::IsPress(No::GamepadButton::Down)
		|| (No::Pad::GetStick().leftStickY < -kStickDeadZone);
}

inline bool IsMoveLeftPress() {
	return No::Keyboard::IsPress('A')
		|| No::Keyboard::IsPress(VK_LEFT)
		|| No::Pad::IsPress(No::GamepadButton::Left)
		|| (No::Pad::GetStick().leftStickX < -kStickDeadZone);
}

inline bool IsMoveRightPress() {
	return No::Keyboard::IsPress('D')
		|| No::Keyboard::IsPress(VK_RIGHT)
		|| No::Pad::IsPress(No::GamepadButton::Right)
		|| (No::Pad::GetStick().leftStickX > kStickDeadZone);
}

// 自機移動用アナログ入力（正規化前）
// スクリーン座標系: Y 正方向 = 下。スティック Y 正方向 = 上 なので反転。
inline No::Vector2 GetMoveInput() {
	No::Vector2 input{ 0.0f, 0.0f };

	// キーボード
	if (No::Keyboard::IsPress('W') || No::Keyboard::IsPress(VK_UP))    { input.y -= 1.0f; }
	if (No::Keyboard::IsPress('S') || No::Keyboard::IsPress(VK_DOWN))  { input.y += 1.0f; }
	if (No::Keyboard::IsPress('A') || No::Keyboard::IsPress(VK_LEFT))  { input.x -= 1.0f; }
	if (No::Keyboard::IsPress('D') || No::Keyboard::IsPress(VK_RIGHT)) { input.x += 1.0f; }

	// Dパッド
	if (No::Pad::IsPress(No::GamepadButton::Up))    { input.y -= 1.0f; }
	if (No::Pad::IsPress(No::GamepadButton::Down))  { input.y += 1.0f; }
	if (No::Pad::IsPress(No::GamepadButton::Left))  { input.x -= 1.0f; }
	if (No::Pad::IsPress(No::GamepadButton::Right)) { input.x += 1.0f; }

	// 左スティック（デッドゾーン適用 / Y 軸反転）
	const auto& stick = No::Pad::GetStick();
	if (std::abs(stick.leftStickX) > kStickDeadZone) { input.x += stick.leftStickX; }
	if (std::abs(stick.leftStickY) > kStickDeadZone) { input.y -= stick.leftStickY; }

	return input;
}

} // namespace InputHelper

#pragma once

#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"

// 各Stateは状態を持たない共有オブジェクトとし、プレイヤーごとの値はECS側に置く。
struct PlayerStateContext {
	PlayerComponent& player;
	bool grounded = false;
	bool moving = false;
	bool airDashing = false;
};

class IPlayerState {
public:
	virtual ~IPlayerState() = default;
	virtual PlayerState Update(const PlayerStateContext& context) const = 0;
	virtual int AnimationIndex(const PlayerStateContext& context) const = 0;
	virtual bool CanMoveHorizontally() const { return true; }
	virtual bool IsJumpState() const { return false; }
	virtual void OnEnter(PlayerStateContext&) const {}
	virtual void OnExit(PlayerStateContext&) const {}
};

namespace PlayerStates {

class WaitState final : public IPlayerState {
public:
	PlayerState Update(const PlayerStateContext& c) const override {
		if (c.airDashing) return PlayerState::kAirDash;
		if (!c.grounded) {
			if (c.player.yVelocity > 0.f) return PlayerState::kJump;
			return c.player.coyoteTimer >= c.player.coyoteTime ? PlayerState::kFall : PlayerState::kWait;
		}
		return c.moving ? PlayerState::kWalk : PlayerState::kWait;
	}
	int AnimationIndex(const PlayerStateContext&) const override { return 2; }
};

class WalkState final : public IPlayerState {
public:
	PlayerState Update(const PlayerStateContext& c) const override {
		if (c.airDashing) return PlayerState::kAirDash;
		if (!c.grounded) {
			if (c.player.yVelocity > 0.f) return PlayerState::kJump;
			return c.player.coyoteTimer >= c.player.coyoteTime ? PlayerState::kFall : PlayerState::kWalk;
		}
		return c.moving ? PlayerState::kWalk : PlayerState::kWait;
	}
	int AnimationIndex(const PlayerStateContext&) const override { return 3; }
};

class JumpState final : public IPlayerState {
public:
	PlayerState Update(const PlayerStateContext& c) const override {
		if (c.airDashing) return PlayerState::kAirDash;
		if (c.grounded) return c.moving ? PlayerState::kWalk : PlayerState::kWait;
		if (c.player.yVelocity <= 0.f && c.player.coyoteTimer >= c.player.coyoteTime)
			return PlayerState::kFall;
		return PlayerState::kJump;
	}
	int AnimationIndex(const PlayerStateContext& c) const override {
		switch (c.player.jumpKind) {
		case PlayerJumpKind::kHighJump:
		case PlayerJumpKind::kNormal:
		default:
			return 0;
		}
	}
	bool IsJumpState() const override { return true; }
};

class FallState final : public IPlayerState {
public:
	PlayerState Update(const PlayerStateContext& c) const override {
		if (c.airDashing) return PlayerState::kAirDash;
		if (c.grounded) return c.moving ? PlayerState::kWalk : PlayerState::kWait;
		return c.player.yVelocity > 0.f ? PlayerState::kJump : PlayerState::kFall;
	}
	int AnimationIndex(const PlayerStateContext&) const override { return 0; }
};

class AirDashState final : public IPlayerState {
public:
	PlayerState Update(const PlayerStateContext& c) const override {
		if (c.airDashing) return PlayerState::kAirDash;
		if (c.grounded) return c.moving ? PlayerState::kWalk : PlayerState::kWait;
		return c.player.yVelocity > 0.f ? PlayerState::kJump : PlayerState::kFall;
	}
	int AnimationIndex(const PlayerStateContext&) const override { return 0; }
};

class SleepState final : public IPlayerState {
public:
	PlayerState Update(const PlayerStateContext& c) const override {
		if (c.airDashing) return PlayerState::kAirDash;
		if (!c.grounded) {
			if (c.player.yVelocity > 0.f) return PlayerState::kJump;
			return c.player.coyoteTimer >= c.player.coyoteTime ? PlayerState::kFall : PlayerState::kSleep;
		}
		return PlayerState::kSleep;
	}
	int AnimationIndex(const PlayerStateContext&) const override { return 1; }
	bool CanMoveHorizontally() const override { return false; }
};

inline const IPlayerState& Get(PlayerState state) {
	static const WaitState wait;
	static const WalkState walk;
	static const JumpState jump;
	static const AirDashState airDash;
	static const FallState fall;
	static const SleepState sleep;
	switch (state) {
	case PlayerState::kWalk: return walk;
	case PlayerState::kJump: return jump;
	case PlayerState::kAirDash: return airDash;
	case PlayerState::kFall: return fall;
	case PlayerState::kSleep: return sleep;
	case PlayerState::kWait:
	default: return wait;
	}
}

inline void Update(PlayerStateContext& context) {
	const IPlayerState& current = Get(context.player.state);
	const PlayerState nextState = current.Update(context);
	if (nextState == context.player.state) return;
	current.OnExit(context);
	context.player.state = nextState;
	Get(nextState).OnEnter(context);
}

} // namespace PlayerStates

#pragma once
#include <algorithm>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace NoEngine::ECS {

// 通常のstatic変数を自動列挙せず、復元が必要な状態の所有者を登録する。
class IPlayModeStaticState {
public:
	virtual ~IPlayModeStaticState() = default;
	virtual void Capture() = 0;
	virtual void Restore() = 0;
	virtual void Discard() = 0;
};

class PlayModeStaticStates {
public:
	static PlayModeStaticStates& Get() {
		static PlayModeStaticStates states;
		return states;
	}

	void Register(IPlayModeStaticState& state) {
		if (active_) state.Capture();
		states_.push_back(&state);
	}
	void Unregister(IPlayModeStaticState& state) {
		std::erase(states_, &state);
	}
	void Capture() {
		if (active_) throw std::logic_error("Play mode static snapshot already active");
		try {
			for (auto* state : states_) state->Capture();
		} catch (...) {
			for (auto* state : states_) state->Discard();
			throw;
		}
		active_ = true;
	}
	void Restore() {
		if (!active_) return;
		for (auto* state : states_) state->Restore();
		active_ = false;
	}

private:
	bool active_ = false;
	std::vector<IPlayModeStaticState*> states_;
};

// 再生中に初めて構築された関数内staticも、その初期値へ戻す。
template<typename T>
class PlayModeStaticState final : public IPlayModeStaticState {
public:
	template<typename... Args>
	explicit PlayModeStaticState(Args&&... args) : value_(std::forward<Args>(args)...) {
		PlayModeStaticStates::Get().Register(*this);
	}
	~PlayModeStaticState() override { PlayModeStaticStates::Get().Unregister(*this); }
	PlayModeStaticState(const PlayModeStaticState&) = delete;
	PlayModeStaticState& operator=(const PlayModeStaticState&) = delete;

	using Value = std::remove_reference_t<T>;
	Value& Get() { return value_; }
	const Value& Get() const { return value_; }
	void Capture() override { snapshot_.emplace(value_); }
	void Restore() override {
		if (snapshot_) {
			value_ = std::move(*snapshot_);
			snapshot_.reset();
		}
	}
	void Discard() override { snapshot_.reset(); }

private:
	T value_;
	std::optional<Value> snapshot_;
};
}

// 変数の宣言直後に置く。関数内staticは初回利用時、名前空間の変数は起動時に登録する。
#define NOENGINE_PLAY_MODE_STATIC_IMPL(Variable, Id) \
	static ::NoEngine::ECS::PlayModeStaticState<decltype(Variable)&> noEnginePlayModeState##Id{Variable}
#define NOENGINE_PLAY_MODE_STATIC_EXPAND(Variable, Id) NOENGINE_PLAY_MODE_STATIC_IMPL(Variable, Id)
#define REGISTER_PLAY_MODE_STATIC(Variable) NOENGINE_PLAY_MODE_STATIC_EXPAND(Variable, __COUNTER__)

#pragma once
#include "engine/Runtime/Command/ComputeContext.h"
#include "../Registry.h"
#include <stdexcept>
#include <type_traits>
namespace NoEngine {
namespace ECS {
class ISystem {
public:
	virtual ~ISystem() = default;
	virtual void Update(ComputeContext& ctx, ECS::Registry& registry, float deltaTime) {
		static_cast<void>(ctx);
		Update(registry, deltaTime);
	}
	virtual void Update(ECS::Registry& registry, float deltaTime) = 0;

	// コピー不可のリソースや共有所有する可変データは、派生クラスで複製方法を指定する。
	virtual std::unique_ptr<ISystem> CloneForPlayMode() const { return nullptr; }
	// 復元した実体へ、外部に登録したコールバックなどを結び直す。
	virtual void OnPlayModeRestore(Registry&) {}

	bool GetStopInPause() { return stopInPause_; }
	bool GetStopInGameStop() { return stopInGameStop_; }
protected:

	void SetStopInPause(bool stopInPause) { stopInPause_ = stopInPause; }
	void SetStopInGameStop(bool stopInGameStop) { stopInGameStop_ = stopInGameStop; }

private:
	bool stopInPause_ = true;
	bool stopInGameStop_ = true;
};

template<typename T>
std::unique_ptr<ISystem> CloneSystemForPlayMode(const ISystem& source) {
	const auto& system = static_cast<const T&>(source);
	if (auto clone = system.CloneForPlayMode()) return clone;
	if constexpr (std::is_copy_constructible_v<T>) {
		return std::make_unique<T>(system);
	} else {
		throw std::logic_error("System requires CloneForPlayMode");
	}
}
}
}

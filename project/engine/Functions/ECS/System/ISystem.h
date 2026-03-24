#pragma once
#include "engine/Runtime/Command/ComputeContext.h"
#include "../Registry.h"
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

	bool GetStopInPause() { return stopInPause_; }
	bool GetStopInGameStop() { return stopInGameStop_; }
protected:

	void SetStopInPause(bool stopInPause) { stopInPause_ = stopInPause; }
	void SetStopInGameStop(bool stopInGameStop) { stopInGameStop_ = stopInGameStop; }

private:
	bool stopInPause_ = false;
	bool stopInGameStop_ = true;
};
}
}
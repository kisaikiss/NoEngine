#pragma once
#include "engine/NoEngine.h"

namespace MainStageProgress {
void QueueCapture();
void CaptureIfQueued(No::Registry& registry);
void Reset();
bool Restore(No::Registry& registry);
void RecordCollectedItem(No::Registry& registry, No::Entity item);
}

class MainStageProgressRestoreSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:
	bool restored_ = false;
};

class MainStageProgressCaptureSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
};

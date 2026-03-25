#include "IScene.h"

namespace NoEngine {
namespace Scene {
void IScene::Update(ComputeContext& ctx, float deltaTime) {
	NotSystemUpdate();
	systemManager_->UpdateAll(ctx, *registry_, deltaTime);
	registry_->FlushDestroy();
}

}
}
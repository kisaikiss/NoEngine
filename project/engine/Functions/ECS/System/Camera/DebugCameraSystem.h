#include "../ISystem.h"
#include "../../Component/Common/TransformComponent.h"


namespace NoEngine {
namespace ECS {
class DebugCameraSystem : public ISystem {
public:
	DebugCameraSystem() { SetStopInGameStop(false); SetStopInPause(false); }
	void Update(Registry& registry, float deltaTime) override;
private:
	void BlenderMove(Registry& registry, Entity entity, float deltaTime);
	void UnityMove(Registry& registry, Entity entity, float deltaTime);
};
}
}

#include "ISystem.h"
#include "../Component/TransformComponent.h"


namespace NoEngine {
namespace ECS {
class DebugCameraSystem : public ISystem {
public:
	void Update(Registry& registry, float deltaTime) override;
};
}
}

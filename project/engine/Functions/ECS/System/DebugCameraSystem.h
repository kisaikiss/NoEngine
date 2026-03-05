#include "ISystem.h"

namespace NoEngine {
namespace ECS {
class DebugCameraSystem : public ISystem {
public:
	void Update(Registry& registry, float deltaTime) override;
};
}
}

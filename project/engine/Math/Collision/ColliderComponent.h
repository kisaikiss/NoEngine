#pragma once
#include "../Types/Vector3.h"

namespace NoEngine {
namespace Collision {

struct AABBColliderComponent {
	Math::Vector3 min;
	Math::Vector3 max;
};

struct CapsuleColliderComponent {
	Math::Vector3 localP0;
	Math::Vector3 localP1;
	float radius;
};

}
}
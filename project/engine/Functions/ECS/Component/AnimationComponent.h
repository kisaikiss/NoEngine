#pragma once
#include "engine/Assets/Skeleton.h"
#include "engine/Assets/Animation.h"

namespace NoEngine {
namespace Component {
struct AnimationComponent {
	Animation* animation;
	Skeleton* skeleton;
	float time;
};

}
}
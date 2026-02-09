#pragma once
#include "engine/Math/MathInclude.h"

namespace NoEngine {

template <typename tValue>
struct Keyframe {
	float time;
	tValue value;
};

using KeyframeVector3 = Keyframe<Math::Vector3>;
using KeyframeQuaternion = Keyframe<Math::Quaternion>;

template <typename tValue>
struct AnimationCurve {
	std::vector<tValue> keyframes;
};

struct NodeAnimation {
	AnimationCurve<KeyframeVector3> translate;
	AnimationCurve<KeyframeQuaternion> rotation;
	AnimationCurve<KeyframeVector3> scale;
};



struct Animation {
	float duration;
	std::map<std::string, NodeAnimation> nodeAnimations;
};

}
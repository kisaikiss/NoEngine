#pragma once
#include "engine/Math/MathInclude.h"
namespace NoEngine {
namespace Component {
/// <summary>
/// TransformComponentのtranslateを保存したものとは関係なく初期位置を指定するためのコンポーネント
/// </summary>
struct StartTransformComponent {
	Math::Vector3 translate;
	Math::Quaternion rotation = Math::Quaternion::IDENTITY;
	Math::Vector3 scale = Math::Vector3::UNIT_SCALE;
};


/// <summary>
/// Transform2DComponentのtranslateを保存したものとは関係なく初期位置を指定するためのコンポーネント
/// </summary>
struct StartTransform2DComponent {
	Math::Vector2 translate;
	float rotation;
	Math::Vector2 scale = Math::Vector2::UNIT_SCALE;
};

}
}
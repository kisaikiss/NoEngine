#pragma once
#include "engine/Math/MathInclude.h"
namespace NoEngine {
namespace Component {
/// <summary>
/// TransformComponentのtranslateを保存したものとは関係なく初期位置を指定するためのコンポーネント
/// </summary>
struct StartPositionComponent {
	Math::Vector3 startPosition;
};


/// <summary>
/// Transform2DComponentのtranslateを保存したものとは関係なく初期位置を指定するためのコンポーネント
/// </summary>
struct StartPosition2DComponent {
	Math::Vector2 startPosition;
};

}
}
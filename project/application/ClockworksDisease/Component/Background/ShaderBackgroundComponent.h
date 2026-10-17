#pragma once
#include "engine/NoEngine.h"

struct ShaderBackgroundComponent {
	std::string shaderPath;          
	bool isVisible = true;
	float timeScale = 1.0f;
	No::Color color = No::Color::WHITE;  // シェーダー側で乗算できるティント
	No::Vector4 param = { 0.f, 0.f, 0.f, 0.f }; // シェーダーごとの自由パラメータ
};
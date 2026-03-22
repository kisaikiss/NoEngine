#pragma once
#include "engine/NoEngine.h"
#include <string>
#include <vector>

struct RailCameraComponent {
	std::string railFilePath = "resources/game/td_3105/RailData/sample_rail.json";
	std::vector<No::Vector3> controlPoints;
	std::vector<float> arcLengthTable;

	float totalLength = 0.0f;
	float speed = 6.0f;
	float distance = 0.0f;

	bool isLoaded = false;
	bool isPlaying = true;
	bool isFinished = false;
	bool drawRailDebug = true;
	bool drawCameraDebug = true;
	int samplesPerSegment = 24;
	int debugRailSamplesPerSegment = 12;
};

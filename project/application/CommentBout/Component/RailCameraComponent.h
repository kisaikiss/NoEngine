#pragma once
#include "engine/NoEngine.h"
#include <string>
#include <vector>

enum class RailEventType {
	SpawnEnemy = 0,
	RailStop = 1,
	RailResume = 2
};

enum class RailResumeConditionType {
	None = 0,
	AfterSeconds = 1,
	EnemiesCleared = 2
};

struct RailEnemySpawnEventParams {
	int count = 1;
	int hp = 3;
	float moveSpeed = 3.0f;
	float spawnSpacing = 1.0f;
	No::Vector3 spawnPosition = { 0.0f, 1.0f, 10.0f };
	No::Vector3 moveDirection = { 0.0f, 0.0f, -1.0f };
	int spawnGroupId = 0;
};

struct RailEventData {
	RailEventType type = RailEventType::SpawnEnemy;
	float triggerDistance = 0.0f;

	RailResumeConditionType resumeCondition = RailResumeConditionType::None;
	float resumeAfterSeconds = 1.0f;
	int targetGroupId = 0;

	RailEnemySpawnEventParams spawn;

	bool fired = false;
	bool waitingCondition = false;
	float waitingElapsedSeconds = 0.0f;
};

struct RailEnemyComponent {
	int hp = 3;
	float moveSpeed = 3.0f;
	No::Vector3 moveDirection = { 0.0f, 0.0f, -1.0f };
	int groupId = 0;
	bool wasCollidingWithAttack = false;
};

struct RailCameraComponent {
	std::string railFilePath = "resources/game/td_3105/RailData/sample_rail.json";
	std::string stageName = "Stage_01";
	std::vector<No::Vector3> controlPoints;
	std::vector<float> arcLengthTable;

	std::vector<RailEventData> events;
	int selectedEventIndex = -1;

	float totalLength = 0.0f;
	float speed = 6.0f;
	float distance = 0.0f;

	bool isLoaded = false;
	bool needsRebuildArcLength = false;
	bool isPlaying = true;
	bool isFinished = false;
	bool drawRailDebug = true;
	bool drawCameraDebug = true;
	bool drawControlPointsDebug = true;
	int selectedControlPointIndex = -1;
	float controlPointDebugRadius = 0.1f;
	int samplesPerSegment = 24;
	int debugRailSamplesPerSegment = 12;
};

#include "stdafx.h"
#include "RailDataIO.h"

#include "application/CommentBout/Component/RailCameraComponent.h"
#include "application/CommentBout/Data/EnemyConfig.h"
#include "application/CommentBout/Data/EnemyDataIO.h"
#include "externals/nlohmann/json.hpp"
#include <filesystem>
#include <fstream>
#include <algorithm>

namespace {
const char* ToEventTypeString(RailEventType type) {
	switch (type) {
	case RailEventType::SpawnEnemy: return "SpawnEnemy";
	case RailEventType::RailStop: return "RailStop";
	default: return "SpawnEnemy";
	}
}

bool TryParseEventType(const std::string& typeName, RailEventType& outType) {
	if (typeName == "SpawnEnemy") {
		outType = RailEventType::SpawnEnemy;
		return true;
	}
	if (typeName == "RailStop") {
		outType = RailEventType::RailStop;
		return true;
	}
	return false;
}

const char* ToResumeConditionString(RailResumeConditionType condition) {
	switch (condition) {
	case RailResumeConditionType::None: return "None";
	case RailResumeConditionType::AfterSeconds: return "AfterSeconds";
	case RailResumeConditionType::EnemiesCleared: return "EnemiesCleared";
	default: return "None";
	}
}

RailResumeConditionType ParseResumeCondition(const std::string& conditionName) {
	if (conditionName == "AfterSeconds") {
		return RailResumeConditionType::AfterSeconds;
	}
	if (conditionName == "EnemiesCleared") {
		return RailResumeConditionType::EnemiesCleared;
	}
	return RailResumeConditionType::None;
}

const char* ToEnemyTypeString(RailEnemyType type) {
	switch (type) {
	case RailEnemyType::MoveOnly: return "MoveOnly";
	case RailEnemyType::MoveAndShoot: return "MoveAndShoot";
	case RailEnemyType::Boss: return "Boss";
	default: return "MoveOnly";
	}
}

RailEnemyType ParseEnemyType(const std::string& typeName) {
	if (typeName == "MoveAndShoot") {
		return RailEnemyType::MoveAndShoot;
	}
	if (typeName == "Boss") {
		return RailEnemyType::Boss;
	}
	return RailEnemyType::MoveOnly;
}

struct LegacyResumeEventData {
	float triggerDistance = 0.0f;
	RailResumeConditionType resumeCondition = RailResumeConditionType::None;
	float resumeAfterSeconds = 0.0f;
	int targetGroupId = 0;
};

void SortEventsByTriggerAndType(std::vector<RailEventData>& events) {
	std::stable_sort(events.begin(), events.end(), [](const RailEventData& a, const RailEventData& b) {
		if (a.triggerDistance != b.triggerDistance) {
			return a.triggerDistance < b.triggerDistance;
		}
		if (a.type == b.type) {
			return false;
		}
		if (a.type == RailEventType::SpawnEnemy && b.type == RailEventType::RailStop) {
			return true;
		}
		return false;
	});
}

void ApplyLegacyResumeMigration(std::vector<RailEventData>& events, const std::vector<LegacyResumeEventData>& legacyResumes) {
	for (const auto& legacy : legacyResumes) {
		int bestStopIndex = -1;
		float bestDistance = -1.0f;
		for (size_t i = 0; i < events.size(); ++i) {
			const auto& e = events[i];
			if (e.type != RailEventType::RailStop) {
				continue;
			}
			if (e.triggerDistance > legacy.triggerDistance) {
				continue;
			}
			if (bestStopIndex < 0 || e.triggerDistance >= bestDistance) {
				bestStopIndex = static_cast<int>(i);
				bestDistance = e.triggerDistance;
			}
		}

		if (bestStopIndex >= 0) {
			auto& stopEvent = events[static_cast<size_t>(bestStopIndex)];
			stopEvent.resumeCondition = legacy.resumeCondition;
			stopEvent.resumeAfterSeconds = legacy.resumeAfterSeconds;
			stopEvent.targetGroupId = legacy.targetGroupId;
		} else {
			RailEventData stopEvent;
			stopEvent.type = RailEventType::RailStop;
			stopEvent.triggerDistance = legacy.triggerDistance;
			stopEvent.resumeCondition = legacy.resumeCondition;
			stopEvent.resumeAfterSeconds = legacy.resumeAfterSeconds;
			stopEvent.targetGroupId = legacy.targetGroupId;
			events.push_back(stopEvent);
		}
	}

	SortEventsByTriggerAndType(events);
}
}

static const char* kStageDataBase = "resources/game/td_3105/Data/StageData/";

std::string MakeRailFilePath(const std::string& stageName) {
	return std::string(kStageDataBase) + "RailData/" + stageName + "_rail.json";
}

std::string MakeEventFilePath(const std::string& stageName) {
	return std::string(kStageDataBase) + "EventData/" + stageName + "_events.json";
}

std::string MakeStageWrapperPath(const std::string& stageName) {
	return std::string(kStageDataBase) + stageName + ".json";
}

void ResetEventRuntime(RailCameraComponent& rail) {
	for (auto& eventData : rail.events) {
		eventData.fired = false;
	}
	rail.runtimeState = RailRuntimeState::Playing;
	rail.isPlaying = true;
	rail.waitingResumeCondition = RailResumeConditionType::None;
	rail.waitingResumeAfterSeconds = 0.0f;
	rail.waitingTargetGroupId = 0;
	rail.waitingResumeElapsedSeconds = 0.0f;
	rail.waitingSourceEventIndex = -1;
}

bool SaveRailToJson(const RailCameraComponent& rail, const std::string& stageName) {
	nlohmann::json json;
	json["stageName"] = stageName;
	json["defaultSpeed"] = rail.speed;
	json["controlPoints"] = nlohmann::json::array();
	for (const auto& p : rail.controlPoints) {
		json["controlPoints"].push_back({ p.x, p.y, p.z });
	}

	const std::string filePath = MakeRailFilePath(stageName);
	std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());
	std::ofstream ofs(filePath);
	if (!ofs) {
		return false;
	}
	ofs << json.dump(2);
	return true;
}

bool SaveEventsToJson(const RailCameraComponent& rail, const std::string& stageName) {
	nlohmann::json json;
	json["stageName"] = stageName;
	json["events"] = nlohmann::json::array();

	for (const auto& e : rail.events) {
		nlohmann::json eventJson;
		eventJson["type"] = ToEventTypeString(e.type);
		eventJson["triggerDistance"] = e.triggerDistance;
		eventJson["resumeCondition"] = ToResumeConditionString(e.resumeCondition);
		eventJson["resumeAfterSeconds"] = e.resumeAfterSeconds;
		eventJson["targetGroupId"] = e.targetGroupId;
		eventJson["spawn"] = {
			{ "count", e.spawn.count },
			{ "hp", e.spawn.hp },
			{ "moveSpeed", e.spawn.moveSpeed },
			{ "spawnOffset", { e.spawn.spawnOffset.x, e.spawn.spawnOffset.y, e.spawn.spawnOffset.z } },
			{ "spawnPosition", { e.spawn.spawnPosition.x, e.spawn.spawnPosition.y, e.spawn.spawnPosition.z } },
			{ "moveDirection", { e.spawn.moveDirection.x, e.spawn.moveDirection.y, e.spawn.moveDirection.z } },
			{ "spawnGroupId", e.spawn.spawnGroupId },
			{ "enemyType", ToEnemyTypeString(e.spawn.enemyType) },
			{ "boss", {
				{ "offsetLocal", { e.spawn.boss.offsetLocal.x, e.spawn.boss.offsetLocal.y, e.spawn.boss.offsetLocal.z } },
				{ "figure8Amplitude", { e.spawn.boss.figure8Amplitude.x, e.spawn.boss.figure8Amplitude.y } },
				{ "figure8Period", e.spawn.boss.figure8Period },
				{ "stopDuration", e.spawn.boss.stopDuration },
				{ "burstShotInterval", e.spawn.boss.burstShotInterval }
			} }
		};
		json["events"].push_back(eventJson);
	}

	const std::string filePath = MakeEventFilePath(stageName);
	std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());
	std::ofstream ofs(filePath);
	if (!ofs) {
		return false;
	}
	ofs << json.dump(2);
	return true;
}

bool SaveStageWrapper(const std::string& stageName) {
	const std::string filePath = MakeStageWrapperPath(stageName);
	nlohmann::json json;
	json["stageName"]        = stageName;
	json["railDataPath"]        = MakeRailFilePath(stageName);
	json["eventDataPath"]       = MakeEventFilePath(stageName);
	json["fieldObjectDataPath"] = std::string(kStageDataBase) + "FieldObjectData/" + stageName + "_field_objects.json";

	std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());
	std::ofstream ofs(filePath);
	if (!ofs) {
		return false;
	}
	ofs << json.dump(2);
	return true;
}

bool LoadStageWrapper(const std::string& stageName) {
	const std::string filePath = MakeStageWrapperPath(stageName);
	std::ifstream ifs(filePath);
	return ifs.good();
}

bool LoadRailToComponent(RailCameraComponent& rail, const std::string& stageName) {
	const std::string filePath = MakeRailFilePath(stageName);
	std::ifstream ifs(filePath);
	if (!ifs) {
		return false;
	}

	nlohmann::json json;
	ifs >> json;

	auto pointsIt = json.find("controlPoints");
	if (pointsIt == json.end() || !pointsIt->is_array()) {
		return false;
	}

	rail.controlPoints.clear();
	for (const auto& p : *pointsIt) {
		if (!p.is_array() || p.size() < 3) {
			continue;
		}
		No::Vector3 point{};
		point.x = p[0].get<float>();
		point.y = p[1].get<float>();
		point.z = p[2].get<float>();
		rail.controlPoints.push_back(point);
	}

	if (rail.controlPoints.size() < 2) {
		return false;
	}

	const auto speedIt = json.find("defaultSpeed");
	if (speedIt != json.end() && speedIt->is_number()) {
		rail.speed = speedIt->get<float>();
	}

	rail.stageName = stageName;
	rail.railFilePath = filePath;
	rail.selectedControlPointIndex = (rail.controlPoints.empty()) ? -1 : 0;
	rail.distance = 0.0f;
	rail.isFinished = false;
	rail.isPlaying = true;
	rail.isLoaded = false;
	rail.needsRebuildArcLength = true;
	ResetEventRuntime(rail);
	return true;
}

bool LoadEventsToComponent(RailCameraComponent& rail, const std::string& stageName) {
	const std::string filePath = MakeEventFilePath(stageName);
	std::ifstream ifs(filePath);
	if (!ifs) {
		return false;
	}

	EnemyConfigMap presets = EnemyDataIO::Load();

	nlohmann::json json;
	ifs >> json;

	auto eventsIt = json.find("events");
	if (eventsIt == json.end() || !eventsIt->is_array()) {
		return false;
	}

	bool hasLegacyResumeEvent = false;
	std::vector<LegacyResumeEventData> legacyResumes;

	rail.events.clear();
	for (const auto& eventJson : *eventsIt) {
		RailEventData eventData;
		std::string typeName = "SpawnEnemy";

		const auto typeIt = eventJson.find("type");
		if (typeIt != eventJson.end() && typeIt->is_string()) {
			typeName = typeIt->get<std::string>();
		}

		const auto triggerIt = eventJson.find("triggerDistance");
		if (triggerIt != eventJson.end() && triggerIt->is_number()) {
			eventData.triggerDistance = triggerIt->get<float>();
		}

		const auto conditionIt = eventJson.find("resumeCondition");
		if (conditionIt != eventJson.end() && conditionIt->is_string()) {
			eventData.resumeCondition = ParseResumeCondition(conditionIt->get<std::string>());
		}
		const auto afterIt = eventJson.find("resumeAfterSeconds");
		if (afterIt != eventJson.end() && afterIt->is_number()) {
			eventData.resumeAfterSeconds = afterIt->get<float>();
		}
		const auto targetGroupIt = eventJson.find("targetGroupId");
		if (targetGroupIt != eventJson.end() && targetGroupIt->is_number_integer()) {
			eventData.targetGroupId = targetGroupIt->get<int>();
		}

		if (typeName == "RailResume") {
			hasLegacyResumeEvent = true;
			LegacyResumeEventData legacy;
			legacy.triggerDistance = eventData.triggerDistance;
			legacy.resumeCondition = eventData.resumeCondition;
			legacy.resumeAfterSeconds = eventData.resumeAfterSeconds;
			legacy.targetGroupId = eventData.targetGroupId;
			legacyResumes.push_back(legacy);
			continue;
		}

		if (!TryParseEventType(typeName, eventData.type)) {
			continue;
		}

		const auto spawnIt = eventJson.find("spawn");
		if (spawnIt != eventJson.end() && spawnIt->is_object()) {
			const auto countIt = spawnIt->find("count");
			if (countIt != spawnIt->end() && countIt->is_number_integer()) {
				eventData.spawn.count = countIt->get<int>();
			}
			const auto hpIt = spawnIt->find("hp");
			if (hpIt != spawnIt->end() && hpIt->is_number_integer()) {
				eventData.spawn.hp = hpIt->get<int>();
			}
			const auto speedIt = spawnIt->find("moveSpeed");
			if (speedIt != spawnIt->end() && speedIt->is_number()) {
				eventData.spawn.moveSpeed = speedIt->get<float>();
			}
			const auto spawnOffsetIt = spawnIt->find("spawnOffset");
			if (spawnOffsetIt != spawnIt->end() && spawnOffsetIt->is_array() && spawnOffsetIt->size() >= 3) {
				eventData.spawn.spawnOffset.x = (*spawnOffsetIt)[0].get<float>();
				eventData.spawn.spawnOffset.y = (*spawnOffsetIt)[1].get<float>();
				eventData.spawn.spawnOffset.z = (*spawnOffsetIt)[2].get<float>();
			} else {
				const auto spacingIt = spawnIt->find("spawnSpacing");
				if (spacingIt != spawnIt->end() && spacingIt->is_number()) {
					eventData.spawn.spawnOffset = { 0.0f, 0.0f, spacingIt->get<float>() };
				}
			}
			const auto spawnPosIt = spawnIt->find("spawnPosition");
			if (spawnPosIt != spawnIt->end() && spawnPosIt->is_array() && spawnPosIt->size() >= 3) {
				eventData.spawn.spawnPosition.x = (*spawnPosIt)[0].get<float>();
				eventData.spawn.spawnPosition.y = (*spawnPosIt)[1].get<float>();
				eventData.spawn.spawnPosition.z = (*spawnPosIt)[2].get<float>();
			}
			const auto moveDirIt = spawnIt->find("moveDirection");
			if (moveDirIt != spawnIt->end() && moveDirIt->is_array() && moveDirIt->size() >= 3) {
				eventData.spawn.moveDirection.x = (*moveDirIt)[0].get<float>();
				eventData.spawn.moveDirection.y = (*moveDirIt)[1].get<float>();
				eventData.spawn.moveDirection.z = (*moveDirIt)[2].get<float>();
			}
			const auto spawnGroupIt = spawnIt->find("spawnGroupId");
			if (spawnGroupIt != spawnIt->end() && spawnGroupIt->is_number_integer()) {
				eventData.spawn.spawnGroupId = spawnGroupIt->get<int>();
			}
			const auto enemyTypeIt = spawnIt->find("enemyType");
			if (enemyTypeIt != spawnIt->end() && enemyTypeIt->is_string()) {
				eventData.spawn.enemyType = ParseEnemyType(enemyTypeIt->get<std::string>());
			}
			const EnemyConfig preset = EnemyDataIO::GetOrDefault(presets, eventData.spawn.enemyType);
			if (eventData.spawn.hp <= 0) {
				eventData.spawn.hp = std::max(1, preset.minHp);
			}
			const auto bossIt = spawnIt->find("boss");
			if (bossIt != spawnIt->end() && bossIt->is_object()) {
				const auto offsetIt = bossIt->find("offsetLocal");
				if (offsetIt != bossIt->end() && offsetIt->is_array() && offsetIt->size() >= 3) {
					eventData.spawn.boss.offsetLocal.x = (*offsetIt)[0].get<float>();
					eventData.spawn.boss.offsetLocal.y = (*offsetIt)[1].get<float>();
					eventData.spawn.boss.offsetLocal.z = (*offsetIt)[2].get<float>();
				}
				const auto ampIt = bossIt->find("figure8Amplitude");
				if (ampIt != bossIt->end() && ampIt->is_array() && ampIt->size() >= 2) {
					eventData.spawn.boss.figure8Amplitude.x = (*ampIt)[0].get<float>();
					eventData.spawn.boss.figure8Amplitude.y = (*ampIt)[1].get<float>();
				}
				const auto periodIt = bossIt->find("figure8Period");
				if (periodIt != bossIt->end() && periodIt->is_number()) {
					eventData.spawn.boss.figure8Period = periodIt->get<float>();
				}
				const auto stopIt = bossIt->find("stopDuration");
				if (stopIt != bossIt->end() && stopIt->is_number()) {
					eventData.spawn.boss.stopDuration = stopIt->get<float>();
				}
				const auto burstIt = bossIt->find("burstShotInterval");
				if (burstIt != bossIt->end() && burstIt->is_number()) {
					eventData.spawn.boss.burstShotInterval = burstIt->get<float>();
				}
			}
		}

		rail.events.push_back(eventData);
	}

	if (!legacyResumes.empty()) {
		ApplyLegacyResumeMigration(rail.events, legacyResumes);
	}

	if (hasLegacyResumeEvent) {
		SaveEventsToJson(rail, stageName);
	}

	rail.selectedEventIndex = rail.events.empty() ? -1 : 0;
	ResetEventRuntime(rail);
	return true;
}

#include "stdafx.h"
#include "RailCameraSystem.h"
#include "application/CommentBout/Component/RailCameraComponent.h"
#include "application/CommentBout/Component/EnemyComponent.h"
#include "application/CommentBout/GameTag.h"
#include "application/CommentBout/Utility/CBCollisionMask.h"
#include "application/TestApp/Component/Collider3DComponent.h"
#include "application/TestApp/Component/ProjectedColliderComponent.h"
#include "engine/Functions/ECS/Component/CameraComponent.h"
#include "engine/Functions/Renderer/Primitive.h"
#include "externals/nlohmann/json.hpp"
#include <fstream>
#include <algorithm>
#include <cmath>

namespace No {
using ::NoEngine::Primitive;
}

namespace {
No::Vector3 CatmullRom(const No::Vector3& p0, const No::Vector3& p1, const No::Vector3& p2, const No::Vector3& p3, float t) {
	const float t2 = t * t;
	const float t3 = t2 * t;
	return 0.5f * ((2.0f * p1) + (-p0 + p2) * t + (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 + (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
}

No::Vector3 NormalizeOrDefault(const No::Vector3& v, const No::Vector3& fallback) {
	if (v.LengthSquared() <= 0.000001f) {
		return fallback;
	}
	return v.Normalize();
}

No::Vector3 EvaluatePositionByT(const RailCameraComponent& rail, float t) {
	const size_t pointCount = rail.controlPoints.size();
	if (pointCount == 0) {
		return No::Vector3::ZERO;
	}
	if (pointCount == 1) {
		return rail.controlPoints[0];
	}

	if (t < 0.0f) {
		t = 0.0f;
	} else if (t > 1.0f) {
		t = 1.0f;
	}

	const int segmentCount = static_cast<int>(pointCount) - 1;
	float scaled = t * static_cast<float>(segmentCount);
	int seg = static_cast<int>(std::floor(scaled));
	if (seg >= segmentCount) {
		seg = segmentCount - 1;
		scaled = static_cast<float>(segmentCount);
	}
	const float localT = scaled - static_cast<float>(seg);

	const int i0 = std::max(seg - 1, 0);
	const int i1 = seg;
	const int i2 = std::min(seg + 1, segmentCount);
	const int i3 = std::min(seg + 2, segmentCount);

	return CatmullRom(
		rail.controlPoints[static_cast<size_t>(i0)],
		rail.controlPoints[static_cast<size_t>(i1)],
		rail.controlPoints[static_cast<size_t>(i2)],
		rail.controlPoints[static_cast<size_t>(i3)],
		localT
	);
}

float DistanceToNormalizedT(const RailCameraComponent& rail, float distance) {
	if (rail.arcLengthTable.empty() || rail.totalLength <= 0.0f) {
		return 0.0f;
	}

	if (distance <= 0.0f) {
		return 0.0f;
	}
	if (distance >= rail.totalLength) {
		return 1.0f;
	}

	auto it = std::lower_bound(rail.arcLengthTable.begin(), rail.arcLengthTable.end(), distance);
	if (it == rail.arcLengthTable.begin()) {
		return 0.0f;
	}
	if (it == rail.arcLengthTable.end()) {
		return 1.0f;
	}

	const size_t highIndex = static_cast<size_t>(it - rail.arcLengthTable.begin());
	const size_t lowIndex = highIndex - 1;
	const float lowDistance = rail.arcLengthTable[lowIndex];
	const float highDistance = rail.arcLengthTable[highIndex];
	const float span = highDistance - lowDistance;
	float lerpT = 0.0f;
	if (span > 0.00001f) {
		lerpT = (distance - lowDistance) / span;
	}

	const float sampleCount = static_cast<float>(rail.arcLengthTable.size() - 1);
	const float tLow = static_cast<float>(lowIndex) / sampleCount;
	const float tHigh = static_cast<float>(highIndex) / sampleCount;
	return tLow + (tHigh - tLow) * lerpT;
}

void ApplyTransformFromDistance(RailCameraComponent& rail, No::TransformComponent* transform) {
	if (!transform || rail.totalLength <= 0.0f) {
		return;
	}

	if (rail.distance < 0.0f) {
		rail.distance = 0.0f;
	} else if (rail.distance > rail.totalLength) {
		rail.distance = rail.totalLength;
	}

	const float t = DistanceToNormalizedT(rail, rail.distance);
	const No::Vector3 position = EvaluatePositionByT(rail, t);

	const float epsilon = 0.001f;
	const float nextT = (t + epsilon > 1.0f) ? 1.0f : t + epsilon;
	const float prevT = (t - epsilon < 0.0f) ? 0.0f : t - epsilon;
	No::Vector3 tangent = EvaluatePositionByT(rail, nextT) - EvaluatePositionByT(rail, prevT);
	if (tangent.LengthSquared() > 0.000001f) {
		tangent = tangent.Normalize();
		transform->rotation.LookRotation(tangent, No::Vector3::UP);
	}
	transform->translate = position;
}

bool RebuildRailCache(RailCameraComponent& rail, bool resetPlayState) {
	if (rail.controlPoints.size() < 2) {
		rail.arcLengthTable.clear();
		rail.totalLength = 0.0f;
		rail.isLoaded = false;
		rail.needsRebuildArcLength = false;
		return false;
	}

	const int segmentCount = static_cast<int>(rail.controlPoints.size()) - 1;
	const int samplesPerSegment = std::max(rail.samplesPerSegment, 4);
	const int sampleCount = segmentCount * samplesPerSegment + 1;

	rail.arcLengthTable.clear();
	rail.arcLengthTable.reserve(static_cast<size_t>(sampleCount));

	float accumulated = 0.0f;
	No::Vector3 prev = EvaluatePositionByT(rail, 0.0f);
	rail.arcLengthTable.push_back(0.0f);
	for (int i = 1; i < sampleCount; ++i) {
		const float t = static_cast<float>(i) / static_cast<float>(sampleCount - 1);
		const No::Vector3 current = EvaluatePositionByT(rail, t);
		accumulated += current.Distance(prev);
		rail.arcLengthTable.push_back(accumulated);
		prev = current;
	}

	rail.totalLength = accumulated;
	if (resetPlayState) {
		rail.distance = 0.0f;
		rail.isFinished = false;
	} else {
		rail.distance = std::max(0.0f, std::min(rail.distance, rail.totalLength));
		if (rail.distance >= rail.totalLength) {
			rail.isFinished = true;
			rail.isPlaying = false;
		}
	}

	if (rail.selectedControlPointIndex >= static_cast<int>(rail.controlPoints.size())) {
		rail.selectedControlPointIndex = static_cast<int>(rail.controlPoints.size()) - 1;
	}
	if (rail.selectedControlPointIndex < -1) {
		rail.selectedControlPointIndex = -1;
	}
	if (rail.selectedEventIndex >= static_cast<int>(rail.events.size())) {
		rail.selectedEventIndex = static_cast<int>(rail.events.size()) - 1;
	}
	if (rail.selectedEventIndex < -1) {
		rail.selectedEventIndex = -1;
	}

	rail.isLoaded = rail.totalLength > 0.0f;
	rail.needsRebuildArcLength = false;
	return rail.isLoaded;
}

bool LoadRailFromJson(RailCameraComponent& rail) {
	std::ifstream ifs(rail.railFilePath);
	if (!ifs) {
		return false;
	}

	nlohmann::json json;
	ifs >> json;

	rail.controlPoints.clear();
	const auto pointsIt = json.find("controlPoints");
	if (pointsIt == json.end() || !pointsIt->is_array()) {
		return false;
	}

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

	return RebuildRailCache(rail, true);
}

void DrawRailDebug(const RailCameraComponent& rail) {
	if (!rail.drawRailDebug || rail.controlPoints.size() < 2) {
		return;
	}

	const int segmentCount = static_cast<int>(rail.controlPoints.size()) - 1;
	const int samplesPerSegment = std::max(rail.debugRailSamplesPerSegment, 2);
	const int sampleCount = segmentCount * samplesPerSegment;
	if (sampleCount <= 0) {
		return;
	}

	No::Vector3 prev = EvaluatePositionByT(rail, 0.0f);
	for (int i = 1; i <= sampleCount; ++i) {
		const float t = static_cast<float>(i) / static_cast<float>(sampleCount);
		const No::Vector3 current = EvaluatePositionByT(rail, t);
		No::Primitive::DrawLine(prev, current, NoEngine::Math::Color(1.0f, 0.0f, 0.0f, 1.0f));
		prev = current;
	}
}

void DrawControlPointDebug(const RailCameraComponent& rail) {
	if (!rail.drawControlPointsDebug) {
		return;
	}

	const float radius = (rail.controlPointDebugRadius > 0.0f) ? rail.controlPointDebugRadius : 0.1f;
	for (size_t i = 0; i < rail.controlPoints.size(); ++i) {
		const bool isSelected = (static_cast<int>(i) == rail.selectedControlPointIndex);
		const NoEngine::Math::Color color = isSelected
			? NoEngine::Math::Color(1.0f, 0.0f, 0.0f, 1.0f)
			: NoEngine::Math::Color(0.2f, 1.0f, 0.2f, 1.0f);
		No::Primitive::DrawSphere(rail.controlPoints[i], radius, color, 10, 10);
	}
}

void DrawEventPointDebug(const RailCameraComponent& rail) {
	if (!rail.drawEventPointsDebug || rail.events.empty() || rail.totalLength <= 0.0f) {
		return;
	}

	const float radius = (rail.eventPointDebugRadius > 0.0f) ? rail.eventPointDebugRadius : 0.12f;
	for (size_t i = 0; i < rail.events.size(); ++i) {
		const RailEventData& eventData = rail.events[i];
		float distance = eventData.triggerDistance;
		if (distance < 0.0f) {
			distance = 0.0f;
		}
		if (distance > rail.totalLength) {
			distance = rail.totalLength;
		}

		const float t = DistanceToNormalizedT(rail, distance);
		const No::Vector3 position = EvaluatePositionByT(rail, t);
		const bool selected = (static_cast<int>(i) == rail.selectedEventIndex);
		const NoEngine::Math::Color color = selected
			? NoEngine::Math::Color(1.0f, 1.0f, 0.2f, 1.0f)
			: NoEngine::Math::Color(0.2f, 0.6f, 1.0f, 1.0f);
		No::Primitive::DrawSphere(position, radius, color, 10, 10);
	}
}

void DrawRailCameraGizmo(RailCameraComponent& rail, No::TransformComponent* transform) {
	if (!rail.drawCameraDebug || !transform) {
		return;
	}

	No::Matrix4x4 world = transform->MakeAffineMatrix4x4();
	No::Vector3 origin = transform->GetWorldPosition();
	No::Vector3 forward = world.TransformNormal(No::Vector3::FORWARD);
	No::Vector3 right = world.TransformNormal(No::Vector3::RIGHT);
	No::Vector3 up = world.TransformNormal(No::Vector3::UP);

	if (forward.LengthSquared() <= 0.000001f || right.LengthSquared() <= 0.000001f || up.LengthSquared() <= 0.000001f) {
		return;
	}
	forward = forward.Normalize();
	right = right.Normalize();
	up = up.Normalize();

	const float bodyLength = 0.9f;
	const float halfWidth = 0.28f;
	const float halfHeight = 0.20f;
	const No::Vector3 tip = origin + forward * bodyLength;
	const No::Vector3 c0 = tip + up * halfHeight - right * halfWidth;
	const No::Vector3 c1 = tip + up * halfHeight + right * halfWidth;
	const No::Vector3 c2 = tip - up * halfHeight + right * halfWidth;
	const No::Vector3 c3 = tip - up * halfHeight - right * halfWidth;

	const NoEngine::Math::Color cameraColor(1.0f, 0.95f, 0.2f, 1.0f);
	No::Primitive::DrawLine(origin, c0, cameraColor);
	No::Primitive::DrawLine(origin, c1, cameraColor);
	No::Primitive::DrawLine(origin, c2, cameraColor);
	No::Primitive::DrawLine(origin, c3, cameraColor);
	No::Primitive::DrawLine(c0, c1, cameraColor);
	No::Primitive::DrawLine(c1, c2, cameraColor);
	No::Primitive::DrawLine(c2, c3, cameraColor);
	No::Primitive::DrawLine(c3, c0, cameraColor);
}

int CountAliveRailEnemies(No::Registry& registry) {
	int count = 0;
	auto enemyView = registry.View<CBRailEnemyTag, EnemyComponent>();
	for (auto entity : enemyView) {
		(void)entity;
		++count;
	}
	return count;
}

int CountAliveRailEnemiesByGroup(No::Registry& registry, int groupId) {
	int count = 0;
	auto enemyView = registry.View<CBRailEnemyTag, EnemyComponent>();
	for (auto entity : enemyView) {
		auto* enemy = registry.GetComponent<EnemyComponent>(entity);
		if (!enemy) {
			continue;
		}
		if (enemy->groupId == groupId) {
			++count;
		}
	}
	return count;
}

void SpawnEnemies(No::Registry& registry, const RailEnemySpawnEventParams& params) {
	const No::Vector3 direction = NormalizeOrDefault(params.moveDirection, No::Vector3(0.0f, 0.0f, -1.0f));
	const int spawnCount = std::max(1, params.count);

	for (int i = 0; i < spawnCount; ++i) {
		auto enemyEntity = registry.GenerateEntity();
		registry.AddComponent<CBRailEnemyTag>(enemyEntity);

		auto* transform = registry.AddComponent<No::TransformComponent>(enemyEntity);
		transform->translate = params.spawnPosition + direction * (params.spawnSpacing * static_cast<float>(i));
		transform->scale = { 0.7f, 0.7f, 0.7f };

		auto* mesh = registry.AddComponent<No::MeshComponent>(enemyEntity);
		auto* material = registry.AddComponent<No::MaterialComponent>(enemyEntity);
		No::ModelLoader::LoadModel("commentbout_rail_enemy_cube", "resources/game/td_3105/Model/cube/cube.obj", mesh);
		material->materials = No::ModelLoader::GetMaterial("commentbout_rail_enemy_cube");
		material->color = { 0.9f, 0.2f, 0.2f, 1.0f };
		material->psoName = L"Renderer : Default PSO";
		material->psoId = NoEngine::Render::GetPSOID(material->psoName);
		material->rootSigId = NoEngine::Render::GetRootSignatureID(material->psoName);

		auto* enemy = registry.AddComponent<EnemyComponent>(enemyEntity);
		enemy->hp = std::max(1, params.hp);
		enemy->moveSpeed = std::max(0.0f, params.moveSpeed);
		enemy->moveDirection = direction;
		enemy->groupId = params.spawnGroupId;

		auto* collider3D = registry.AddComponent<TestApp::Collider3DComponent>(enemyEntity);
		collider3D->shapeType = TestApp::ShapeType3D::Box;
		collider3D->useScaleAsBox = true;
		collider3D->boxSizeMultiplier = { 1.0f, 1.0f, 1.0f };
		collider3D->collisionLayer = CommentBout::CollisionLayer::CBEnemy;
		collider3D->collisionMask = CommentBout::CollisionLayer::CBPlayerAttack;

		auto* projected = registry.AddComponent<TestApp::ProjectedColliderComponent>(enemyEntity);
		projected->source3DEntity = enemyEntity;
	}
}

void UpdateRailEvents(No::Registry& registry, RailCameraComponent& rail, float deltaTime) {
	for (auto& eventData : rail.events) {
		if (!eventData.fired && !eventData.waitingCondition && rail.distance >= eventData.triggerDistance) {
			switch (eventData.type) {
			case RailEventType::SpawnEnemy:
				SpawnEnemies(registry, eventData.spawn);
				eventData.fired = true;
				break;
			case RailEventType::RailStop:
				rail.isPlaying = false;
				eventData.fired = true;
				break;
			case RailEventType::RailResume:
				if (eventData.resumeCondition == RailResumeConditionType::None) {
					rail.isPlaying = true;
					rail.isFinished = false;
					eventData.fired = true;
				} else {
					eventData.waitingCondition = true;
					eventData.waitingElapsedSeconds = 0.0f;
				}
				break;
			}
		}

		if (eventData.waitingCondition && !eventData.fired) {
			bool shouldResume = false;
			switch (eventData.resumeCondition) {
			case RailResumeConditionType::AfterSeconds:
				eventData.waitingElapsedSeconds += deltaTime;
				shouldResume = (eventData.waitingElapsedSeconds >= std::max(0.0f, eventData.resumeAfterSeconds));
				break;
			case RailResumeConditionType::EnemiesCleared:
				shouldResume = (CountAliveRailEnemiesByGroup(registry, eventData.targetGroupId) == 0);
				break;
			default:
				shouldResume = true;
				break;
			}

			if (shouldResume) {
				rail.isPlaying = true;
				rail.isFinished = false;
				eventData.fired = true;
				eventData.waitingCondition = false;
			}
		}
	}
}
}

void RailCameraSystem::Update(No::Registry& registry, float deltaTime) {
	auto view = registry.View<RailCameraComponent, No::TransformComponent>();
	for (auto entity : view) {
		auto* rail = registry.GetComponent<RailCameraComponent>(entity);
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		if (!rail || !transform) {
			continue;
		}

		if (rail->needsRebuildArcLength) {
			RebuildRailCache(*rail, false);
		}

		if (!rail->isLoaded) {
			if (rail->controlPoints.size() >= 2) {
				RebuildRailCache(*rail, false);
			} else {
				LoadRailFromJson(*rail);
			}
		}
		if (!rail->isLoaded || rail->totalLength <= 0.0f) {
			continue;
		}

		if (rail->isPlaying && !rail->isFinished) {
			rail->distance += rail->speed * deltaTime;
			if (rail->distance >= rail->totalLength) {
				rail->distance = rail->totalLength;
				rail->isPlaying = false;
				rail->isFinished = true;
			}
		}

		UpdateRailEvents(registry, *rail, deltaTime);
		ApplyTransformFromDistance(*rail, transform);
		DrawRailDebug(*rail);
		DrawControlPointDebug(*rail);
		DrawEventPointDebug(*rail);
		DrawRailCameraGizmo(*rail, transform);
	}
}

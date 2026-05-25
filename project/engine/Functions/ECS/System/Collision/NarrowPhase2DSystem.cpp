#include "stdafx.h"
#include "NarrowPhase2DSystem.h"
#include "../../Component/Transform2DComponent.h"
#include "engine/Math/Collision/CollisionHelper.h"

namespace NoEngine {
namespace ECS {
using namespace Math;


void NarrowPhase2DSystem::Update(Registry& registry, float deltaTime) {
	// 前フレームからの経過時間は使用しない
	static_cast<void>(deltaTime);

	// CollisionResolutionSystemからゲームへ送られるはずのイベントを消費しておく
	// ゲームアプリケーション用衝突応答のシステムが存在しない場合の対策
	registry.PollEvent<Event::Contact2DEvent>();

	// 衝突情報
	Event::Contact2DEvent contactEvent;

	auto pairs = BroadPhaseGrid(registry, 150.0f /* cell size */);

	for (auto pair : pairs) {
		Entity boxA = pair.a;
		auto* boxATransform = registry.GetComponent<Component::Transform2DComponent>(boxA);
		auto* boxACollider = registry.GetComponent<Math::AABBCollider2D>(boxA);

		Entity boxB = pair.b;
		auto* boxBTransform = registry.GetComponent<Component::Transform2DComponent>(boxB);
		auto* boxBCollider = registry.GetComponent<Math::AABBCollider2D>(boxB);

		Math::Collision2D collide;

		switch (axis_) {
		case NoEngine::ECS::NarrowPhase2DSystem::TestAxis::Horizontal:
			collide = Math::TestAABB2DHorizontal(boxATransform, boxACollider, boxBTransform, boxBCollider);
			break;
		case NoEngine::ECS::NarrowPhase2DSystem::TestAxis::Vertical:
			collide = Math::TestAABB2DVertical(boxATransform, boxACollider, boxBTransform, boxBCollider);
			break;
		case NoEngine::ECS::NarrowPhase2DSystem::TestAxis::All:
			collide = Math::TestAABB2D(boxATransform, boxACollider, boxBTransform, boxBCollider);
			break;
		}

		if (!collide.hit) continue;

		Contact2D contact;
		contact.a = boxA;
		contact.b = boxB;
		contact.normal = collide.normal;

		auto* body = registry.GetComponent<Component::CollisionBody>(boxB);
		if (body && body->type == Component::BodyType::Through) {
			contact.penetration = 0.f;
		} else {
			contact.penetration = collide.penetration;
		}
		contact.contactPosition = ClassifyContact(collide.normal);

		

		contactEvent.contacts.push_back(contact);
	}

	if (!contactEvent.contacts.empty()) {
		// 衝突情報をイベントとして次のシステムへ送る
		registry.EmitEvent(contactEvent);
	}

}

ContactPosition NarrowPhase2DSystem::ClassifyContact(const Math::Vector2& normal) {
	if (normal.y > 0.5f) return ContactPosition::DOWN;
	if (normal.y < -0.5f) return ContactPosition::UP;
	return ContactPosition::SIDE;
}

std::vector<NarrowPhase2DSystem::BroadPhasePair> NarrowPhase2DSystem::BroadPhaseGrid(Registry& registry, float cellSize) {

	auto PackCellKey = [](int32_t x, int32_t y) -> int64_t {
		return (static_cast<int64_t>(x) << 32) | (static_cast<uint32_t>(y));
		};

	std::unordered_map<int64_t, std::vector<Entity>> grid;
	grid.reserve(1024);

	auto view = registry.View<Math::AABBCollider2D, Component::Transform2DComponent>();

	// 1. AABB をグリッドに登録
	for (auto e : view) {
		auto* t = registry.GetComponent<Component::Transform2DComponent>(e);
		auto* c = registry.GetComponent<Math::AABBCollider2D>(e);

		auto world = GetWorldAABB2D(t, c);

		int32_t minX = static_cast<int32_t>(std::floorf(world.min.x / cellSize));
		int32_t minY = static_cast<int32_t>(std::floorf(world.min.y / cellSize));
		int32_t maxX = static_cast<int32_t>(std::floorf(world.max.x / cellSize));
		int32_t maxY = static_cast<int32_t>(std::floorf(world.max.y / cellSize));

		for (int y = minY; y <= maxY; ++y) {
			for (int x = minX; x <= maxX; ++x) {
				int64_t key = PackCellKey(x, y);
				grid[key].push_back(e);
			}
		}
	}

	// 2. 同じセル内のペアだけ NarrowPhase に渡す
	std::vector<BroadPhasePair> pairs;
	pairs.reserve(1024);

	for (auto& kv : grid) {
		auto& list = kv.second;
		if (list.size() < 2) continue;
		for (size_t i = 0; i < list.size(); ++i) {
			for (size_t j = i + 1; j < list.size(); ++j) {
				pairs.push_back({ list[i], list[j] });
			}
		}
	}

	return pairs;
}

}
}

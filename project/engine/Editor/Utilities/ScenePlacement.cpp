#include "stdafx.h"
#include "ScenePlacement.h"
#include <limits>

namespace NoEngine::Editor {

bool FindGroundedTranslation(std::span<const Math::Vector3> vertices,
	const Math::Vector3& contact, const Math::Vector3& normal, Math::Vector3& translation) {
	if (vertices.empty() || normal.y < 0.2f) return false;
	float support = std::numeric_limits<float>::max();
	for (const auto& vertex : vertices) {
		const float distance = vertex.Dot(normal);
		if (!std::isfinite(distance)) return false;
		support = std::min(support, distance);
	}
	// 水平方向の狙いを保ちながら、斜面に対して最も低い頂点を接地させる。
	translation = contact + Math::Vector3{ 0.f, -support / normal.y, 0.f };
	return std::isfinite(translation.x) && std::isfinite(translation.y) && std::isfinite(translation.z);
}

}

#ifdef USE_IMGUI
#include "EditUtils.h"
#include "engine/Assets/AssetManager.h"
#include "engine/Assets/Model/ModelSaver.h"
#include "engine/Editor/DataDriven/SceneSerializer.h"
#include "engine/Editor/DataDriven/PrefabSerializer.h"
#include "engine/Editor/Commands/EditorCommandOperator.h"
#include "engine/Functions/Command/EditCommand/InstantiateEntityCommand.h"
#include "engine/Functions/ECS/Component/Common/CameraComponent.h"
#include "engine/Functions/ECS/Component/Common/TransformComponent.h"
#include "engine/Functions/ECS/Component/Common/Transform2DComponent.h"
#include "engine/Functions/ECS/Component/Asset/SpriteComponent.h"
#include "engine/Math/Collision/BVH.h"
#include "engine/Math/Collision/CollisionFanc.h"
#include "externals/imgui/imgui.h"

namespace NoEngine::Editor {
namespace {
using Math::Vector3;

struct PlacementPreview {
	std::string key;
	std::string asset;
	std::string type;
	nlohmann::json prefab;
	std::vector<Vector3> vertices;
	std::vector<uint32_t> indices;
	Vector3 scale = Vector3::UNIT_SCALE;
	Math::Quaternion rotation = Math::Quaternion::IDENTITY;
	bool is2D = false;
	bool hasTransform3D = true;
	bool ready = false;
	std::string error;
};
PlacementPreview sPreview;
bool sPlacementActive = false;
bool sDragCancelled = false;

struct RayMesh {
	std::vector<Math::TriangleCollider> triangles;
	std::shared_ptr<Math::BVHNode> bvh;
};
std::unordered_map<uint32_t, RayMesh> sRayMeshes;

std::string ResolveModelPath(const std::string& name) {
	if (std::filesystem::is_regular_file(name)) return name;
	return AssetManager::GetFilePathFromAddressableName(name);
}

void PreparePreview(const std::string& type, const std::string& asset) {
	sPreview = {};
	sPreview.key = type + asset + std::to_string(AssetManager::GetAddressableGeneration());
	sPreview.asset = asset;
	sPreview.type = type;
	try {
		std::string modelName = asset;
		if (type == "PREFAB_PATH") {
			std::ifstream input(asset);
			input >> sPreview.prefab;
			const auto& components = sPreview.prefab.at("components");
			sPreview.hasTransform3D = components.contains("TransformComponent");
			sPreview.is2D = !sPreview.hasTransform3D && components.contains("Transform2DComponent");
			// 設定専用Prefabなど、座標を持たないものの生成も引き続き許可する。
			if (!sPreview.hasTransform3D) {
				sPreview.ready = true;
				return;
			}
			if (sPreview.hasTransform3D) {
				const auto& transform = components.at("TransformComponent");
				const auto scale = transform.value("scale", std::vector<float>{1.f, 1.f, 1.f});
				const auto rotation = transform.value("rotation", std::vector<float>{0.f, 0.f, 0.f, 1.f});
				sPreview.scale = { scale.at(0), scale.at(1), scale.at(2) };
				sPreview.rotation = { rotation.at(0), rotation.at(1), rotation.at(2), rotation.at(3) };
				if (!components.contains("MeshComponent")) {
					sPreview.vertices = { Vector3::ZERO };
					sPreview.ready = true;
					return;
				}
				modelName = components.at("MeshComponent").at("meshName").get<std::string>();
			}
		} else {
			sPreview.is2D = type == "ASSET_TEXTURE_NAME";
		}
		if (sPreview.is2D) {
			sPreview.ready = true;
			return;
		}
		// GPUやシーンに触れず、描画と同じ左手系の頂点だけを読み込む。
		Assimp::Importer importer;
		const auto* scene = importer.ReadFile(ResolveModelPath(modelName), aiProcess_Triangulate | aiProcess_FlipWindingOrder);
		if (!scene || !scene->HasMeshes()) {
			sPreview.error = "Model could not be loaded";
			return;
		}
		Math::Matrix4x4 linear;
		linear.MakeAffine(sPreview.scale, sPreview.rotation, Vector3::ZERO);
		for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
			const auto* mesh = scene->mMeshes[m];
			const auto base = static_cast<uint32_t>(sPreview.vertices.size());
			for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
				const auto& p = mesh->mVertices[v];
				sPreview.vertices.push_back(linear.Transform({ -p.x, p.y, p.z }));
			}
			for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
				const auto& face = mesh->mFaces[f];
				if (face.mNumIndices != 3) continue;
				for (unsigned int i = 0; i < 3; ++i) sPreview.indices.push_back(base + face.mIndices[i]);
			}
		}
		sPreview.ready = !sPreview.vertices.empty() && !sPreview.indices.empty();
	} catch (const std::exception&) {
		sPreview.error = "Prefab or model data is invalid";
	}
}

RayMesh& GetRayMesh(const Component::MeshComponent& component, const Mesh& mesh) {
	auto [it, inserted] = sRayMeshes.try_emplace(component.handle.id);
	if (!inserted) return it->second;
	auto& result = it->second;
	for (const auto& sub : mesh.subMeshes) {
		for (uint32_t i = 0; i + 2 < sub.indexCount; i += 3) {
			Math::TriangleCollider triangle{};
			for (uint32_t v = 0; v < 3; ++v) {
				triangle.v[v] = mesh.vertices.at(sub.vertexStart + mesh.indices.at(sub.indexStart + i + v)).position;
			}
			triangle.normal = (triangle.v[1] - triangle.v[0]).Cross(triangle.v[2] - triangle.v[0]).Normalize();
			result.triangles.push_back(triangle);
		}
	}
	if (!result.triangles.empty()) result.bvh = Math::BuildBVH(result.triangles);
	return result;
}

bool RaycastMesh(RayMesh& mesh, const Vector3& origin, const Vector3& direction, float maxDistance, Math::RayTriangleHit& closest) {
	std::vector<const Math::BVHNode*> stack;
	if (mesh.bvh) stack.push_back(mesh.bvh.get());
	closest.distance = maxDistance;
	while (!stack.empty()) {
		const auto* node = stack.back();
		stack.pop_back();
		const auto center = (node->bounds.min + node->bounds.max) * 0.5f;
		const auto half = (node->bounds.max - node->bounds.min) * 0.5f;
		if (!node->bounds.Contains(origin) && !Math::IntersectRayBox(origin, direction, closest.distance, center, half).hit) continue;
		if (node->left) stack.push_back(node->left.get());
		if (node->right) stack.push_back(node->right.get());
		for (int index : node->triIndices) {
			const auto hit = Math::IntersectRayTriangle(origin, direction, closest.distance, mesh.triangles[index]);
			if (hit.hit) closest = hit;
		}
	}
	return closest.hit;
}

Math::RayTriangleHit FindSurface(ECS::Registry& registry, const Vector3& origin, const Vector3& direction, float maxDistance) {
	Math::RayTriangleHit closest;
	closest.distance = maxDistance;
	for (auto e : registry.View<Component::MeshComponent, Component::TransformComponent, Component::MaterialComponent>()) {
		const auto* component = registry.GetComponent<Component::MeshComponent>(e);
		if (!component->isVisible) continue;
		const auto* mesh = ModelSaver::Get().GetMesh(component->handle);
		if (!mesh) continue;
		auto world = registry.GetComponent<Component::TransformComponent>(e)->MakeAffineMatrix4x4(registry);
		if (auto* animator = registry.GetComponent<Component::AnimatorComponent>(e)) world = animator->local.MakeAffineMatrix4x4(registry) * world;
		const Vector3 x{ world.m[0][0], world.m[0][1], world.m[0][2] };
		const Vector3 y{ world.m[1][0], world.m[1][1], world.m[1][2] };
		const Vector3 z{ world.m[2][0], world.m[2][1], world.m[2][2] };
		if (std::abs(x.Dot(y.Cross(z))) < 1e-8f) continue;
		auto inverse = world;
		inverse.Inverse();
		const auto localDirection = inverse.TransformNormal(direction);
		const float length = localDirection.Length();
		if (length < 1e-8f) continue;
		Math::RayTriangleHit localHit;
		if (!RaycastMesh(GetRayMesh(*component, *mesh), inverse.Transform(origin), localDirection / length,
			closest.distance * length, localHit)) continue;
		closest = localHit;
		closest.position = world.Transform(localHit.position);
		closest.distance = (closest.position - origin).Dot(direction);
		inverse.Transpose();
		closest.normal = inverse.TransformNormal(localHit.normal).Normalize();
		if (closest.normal.Dot(direction) > 0.f) closest.normal = -closest.normal;
	}
	// 空のシーンでも配置できるよう、メッシュに当たらない場合だけY=0を作業面にする。
	if (!closest.hit && direction.y < -1e-6f) {
		const float distance = -origin.y / direction.y;
		if (distance >= 0.f && distance <= maxDistance) {
			closest = { true, distance, origin + direction * distance, Vector3::UP };
		}
	}
	return closest;
}

bool Project(const Vector3& point, const Math::Matrix4x4& vp, const Math::Vector4& rect, ImVec2& screen) {
	const auto& m = vp.m;
	const float w = point.x * m[0][3] + point.y * m[1][3] + point.z * m[2][3] + m[3][3];
	if (w < 1e-5f) return false;
	const float x = (point.x * m[0][0] + point.y * m[1][0] + point.z * m[2][0] + m[3][0]) / w;
	const float y = (point.x * m[0][1] + point.y * m[1][1] + point.z * m[2][1] + m[3][1]) / w;
	screen = { rect.x + (x + 1.f) * 0.5f * rect.z, rect.y + (1.f - y) * 0.5f * rect.w };
	return true;
}

void DrawGhost(const Vector3& translation, const Math::Matrix4x4& vp, const Math::Vector4& rect) {
	auto* draw = ImGui::GetWindowDrawList();
	draw->PushClipRect(ImVec2(rect.x, rect.y), ImVec2(rect.x + rect.z, rect.y + rect.w), true);
	// 大きなモデルは輪郭用の境界ボックスに置き換え、プレビュー描画の負荷を制限する。
	std::vector<Vector3> box;
	std::vector<uint32_t> boxIndices;
	const auto* vertices = &sPreview.vertices;
	const auto* indices = &sPreview.indices;
	if (indices->size() > 30000) {
		Math::AABBCollider bounds;
		for (const auto& vertex : *vertices) bounds.Expand(vertex);
		for (int i = 0; i < 8; ++i) box.push_back({ (i & 1) ? bounds.max.x : bounds.min.x,
			(i & 2) ? bounds.max.y : bounds.min.y, (i & 4) ? bounds.max.z : bounds.min.z });
		boxIndices = { 0,1,3,0,3,2,4,6,7,4,7,5,0,4,5,0,5,1,2,3,7,2,7,6,0,2,6,0,6,4,1,5,7,1,7,3 };
		vertices = &box;
		indices = &boxIndices;
	}
	for (size_t i = 0; i + 2 < indices->size(); i += 3) {
		ImVec2 screen[3];
		bool visible = true;
		for (size_t v = 0; v < 3; ++v) visible &= Project((*vertices)[(*indices)[i + v]] + translation, vp, rect, screen[v]);
		if (!visible) continue;
		draw->AddTriangleFilled(screen[0], screen[1], screen[2], IM_COL32(60, 210, 255, 36));
		draw->AddTriangle(screen[0], screen[1], screen[2], IM_COL32(80, 220, 255, 110));
	}
	draw->PopClipRect();
}

void CommitPlacement(ECS::Registry& registry, const Vector3& translation) {
	const auto entity = registry.GenerateEntity();
	try {
		if (sPreview.type == "PREFAB_PATH") {
			LoadEntityFromJson(registry, entity, sPreview.prefab);
			LinkPrefabInstance(registry, entity, sPreview.asset);
		} else if (sPreview.is2D) {
			registry.AddComponent<Component::SpriteComponent>(entity)->textureName = sPreview.asset;
			registry.GetComponent<Component::SpriteComponent>(entity)->matchScaleToTexture = true;
			registry.AddComponent<Component::Transform2DComponent>(entity);
		} else {
			registry.AddComponent<Component::MeshComponent>(entity)->meshName = sPreview.asset;
			registry.AddComponent<Component::MaterialComponent>(entity);
			registry.AddComponent<Component::TransformComponent>(entity);
		}
		auto* tag = registry.GetComponent<EditTag>(entity);
		if (!tag) tag = registry.AddComponent<EditTag>(entity);
		if (tag->name.empty()) tag->name = std::filesystem::path(sPreview.asset).stem().string();
		if (sPreview.type == "PREFAB_PATH") {
			const auto suffix = tag->name.find("_Prefab");
			if (suffix != std::string::npos) tag->name.erase(suffix);
		}
		tag->name = MakeUniqueName(CollectEditTagNames(registry, entity), tag->name);
		tag->parent = ECS::INVALID_ENTITY;
		if (auto* transform = registry.GetComponent<Component::TransformComponent>(entity)) {
			transform->parent = ECS::INVALID_ENTITY;
			transform->translate = translation;
		} else if (auto* transform2D = registry.GetComponent<Component::Transform2DComponent>(entity)) {
			transform2D->parent = ECS::INVALID_ENTITY;
			transform2D->translate = Get2DSceneWindowMousePosition(registry);
		}
		registry.AddComponent<EditSelectedTag>(entity);
		EditorCommandOperator::AddCommand(std::make_unique<Command::InstantiateEntityCommand>(registry, entity));
	} catch (const std::exception& error) {
		registry.DestroyEntity(entity);
		LogError(std::string("配置データの読み込みに失敗しました: ") + error.what());
	}
}

}

bool IsScenePlacementActive() { return sPlacementActive; }

void DrawScenePlacement(ECS::Registry& registry, const Math::Vector4& sceneRect) {
	const auto* drag = ImGui::GetDragDropPayload();
	sPlacementActive = drag && (drag->IsDataType("PREFAB_PATH") || drag->IsDataType("ASSET_MODEL_NAME") || drag->IsDataType("ASSET_TEXTURE_NAME"));
	if (!sPlacementActive) {
		sPreview = {};
		sDragCancelled = false;
		return;
	}
	if (ImGui::IsKeyPressed(ImGuiKey_Escape)) sDragCancelled = true;
	if (sceneRect.z <= 0.f || sceneRect.w <= 0.f) return;
	if (!ImGui::BeginDragDropTarget()) return;
	const ImGuiPayload* payload = nullptr;
	std::string type;
	for (const char* name : { "PREFAB_PATH", "ASSET_MODEL_NAME", "ASSET_TEXTURE_NAME" }) {
		if (const auto* accepted = ImGui::AcceptDragDropPayload(name,
			ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoDrawDefaultRect)) {
			payload = accepted;
			type = name;
			break;
		}
	}
	if (payload && payload->DataSize > 0 && static_cast<const char*>(payload->Data)[payload->DataSize - 1] == '\0') {
		const std::string asset(static_cast<const char*>(payload->Data));
		if (sPreview.key != type + asset + std::to_string(AssetManager::GetAddressableGeneration())) PreparePreview(type, asset);
		bool valid = sPreview.ready && !sDragCancelled;
		Vector3 translation = Vector3::ZERO;
		if (valid && !sPreview.is2D && sPreview.hasTransform3D) {
			valid = false;
			auto cameras = registry.View<Component::CameraComponent, Component::DebugCameraComponent>();
			auto cameraIterator = cameras.begin();
			if (cameraIterator != cameras.end()) {
				const auto cameraEntity = *cameraIterator;
				const auto* camera = registry.GetComponent<Component::CameraComponent>(cameraEntity);
				auto vp = camera->view * camera->projection;
				auto inverse = vp;
				inverse.Inverse();
				const auto mouse = ImGui::GetMousePos();
				const float x = (mouse.x - sceneRect.x) / sceneRect.z * 2.f - 1.f;
				const float y = 1.f - (mouse.y - sceneRect.y) / sceneRect.w * 2.f;
				const auto origin = inverse.Transform({ x, y, 0.f });
				const auto direction = (inverse.Transform({ x, y, 1.f }) - origin).Normalize();
				const auto surface = FindSurface(registry, origin, direction, camera->farClip);
				valid = surface.hit && FindGroundedTranslation(sPreview.vertices, surface.position, surface.normal, translation);
				if (valid) {
					DrawGhost(translation, vp, sceneRect);
					ImVec2 contact;
					if (Project(surface.position, vp, sceneRect, contact)) {
						ImGui::GetWindowDrawList()->AddCircle(contact, 6.f, IM_COL32(80, 255, 180, 255));
					}
				}
			}
		} else if (valid && sPreview.is2D) {
			const auto mouse = ImGui::GetMousePos();
			ImGui::GetWindowDrawList()->AddRect(ImVec2(mouse.x - 20.f, mouse.y - 20.f), ImVec2(mouse.x + 20.f, mouse.y + 20.f), IM_COL32(60, 210, 255, 200));
		}
		ImGui::BeginTooltip();
		if (!sPreview.error.empty()) ImGui::TextUnformatted(sPreview.error.c_str());
		else ImGui::TextUnformatted(valid ? "Release to place / Esc to cancel" : "No ground surface / placement cancelled");
		ImGui::EndTooltip();
		if (payload->IsDelivery() && valid) CommitPlacement(registry, translation);
	}
	ImGui::EndDragDropTarget();
}

}
#else
namespace NoEngine::Editor {
void DrawScenePlacement(ECS::Registry&, const Math::Vector4&) {}
bool IsScenePlacementActive() { return false; }
}
#endif

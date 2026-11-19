#include "MaterialComponent.h"
#include "engine/Assets/AssetManager.h"
#include "engine/Assets/Model/ModelSaver.h"
#include "engine/Editor/Reflection/ReflectionMacros.h"
#include "engine/Runtime/GraphicsCore.h"

namespace NoEngine::Component {
Math::Matrix4x4 MeshMaterialSettings::MakeUVTransform() const {
	Math::Quaternion rotation;
	rotation.FromAxisAngle(Math::Vector3::FORWARD, uvRotate);
	Math::Matrix4x4 transform;
	transform.MakeAffine(
		Math::Vector3(uvScale.x, uvScale.y, 1.0f),
		rotation,
		Math::Vector3(uvPosition.x, uvPosition.y, 0.0f));
	return transform;
}

D3D12_CPU_DESCRIPTOR_HANDLE MeshMaterialSettings::ResolveTextureSRV(Asset::MaterialHandle modelMaterial) {
	if (!textureName.empty()) {
		const uint64_t generation = AssetManager::GetAddressableGeneration();
		if (loadedTextureGeneration != generation || loadedTextureName != textureName) {
			const std::string path = AssetManager::GetFilePathFromAddressableName(textureName);
			textureHandle = path.empty() ? TextureRef{} : TextureManager::LoadCovertTexture(path);
			loadedTextureName = textureName;
			loadedTextureGeneration = generation;
		}
		if (textureHandle.IsValid()) return textureHandle.GetSRV();
	}

	const auto* material = ModelSaver::Get().GetMaterial(modelMaterial);
	return material ? material->textureHandle.GetSRV() : Graphics::GetDefaultTexture(Graphics::kWhiteOpaque2D);
}

SubMeshMaterialOverride* MaterialComponent::FindSubMeshOverride(std::string_view name, uint32_t index) {
	auto it = std::ranges::find_if(subMeshOverrides, [&](const auto& entry) {
		return entry.meshName == name && entry.subMeshIndex == index;
	});
	return it == subMeshOverrides.end() ? nullptr : &*it;
}
}

#define REFLECT_MESH_MATERIAL_FIELDS \
	REFLECT_ASSET_FIELD(textureName, Texture), \
	REFLECT_ENUM_FIELD(blendMode), \
	REFLECT_ENUM_FIELD(renderMode), \
	REFLECT_FIELD(color), \
	REFLECT_FIELD(drawOutline), \
	REFLECT_FIELD(unlit), \
	REFLECT_FIELD(receiveShadow), \
	REFLECT_FIELD(shininess), \
	REFLECT_FIELD(environmentCoefficient), \
	REFLECT_FIELD(uvPosition), \
	REFLECT_FIELD(uvRotate), \
	REFLECT_FIELD(uvScale), \
	REFLECT_FIELD(emissiveIntensity), \
	REFLECT_FIELD(rimPower), \
	REFLECT_FIELD(noiseScrollSpeed), \
	REFLECT_FIELD(edgeColor), \
	REFLECT_ASSET_FIELD(noiseTextureName, Texture)

template<>
NoEngine::TypeInfo* NoEngine::GetTypeInfo<NoEngine::Component::SubMeshMaterialOverride>() {
	using ThisType = Component::SubMeshMaterialOverride;
	static TypeInfo typeInfo{
		"SubMeshMaterialOverride", sizeof(ThisType), Utilities::TypeID<ThisType>(), "Asset",
		{ REFLECT_FIELD(meshName), REFLECT_FIELD(subMeshIndex), REFLECT_MESH_MATERIAL_FIELDS }, {}, {}
	};
	return &typeInfo;
}

REFLECT_STRUCT_BEGIN(NoEngine::Component::MaterialComponent, "Asset")
[] {
	auto field = REFLECT_STRUCT_ARRAY_FIELD(subMeshOverrides);
	field.attributes.editable = false;
	return field;
}(),
REFLECT_MESH_MATERIAL_FIELDS,
REFLECT_FIELD(castShadow)
REFLECT_STRUCT_END(NoEngine::Component::MaterialComponent)

#undef REFLECT_MESH_MATERIAL_FIELDS

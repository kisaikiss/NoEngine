#include "MaterialComponent.h"
#include "engine/Editor/Reflection/ReflectionMacros.h"

namespace NoEngine::Component {
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

#pragma once
#include "ReflectionTypeTraits.h"
#include "ComponentRegistry.h"
#include "engine/Utilities/TypeIndex.h"
#include "engine/Utilities/FileUtilities.h"

namespace NoEngine {
template<typename T>
NoEngine::TypeInfo* GetTypeInfo();
}


// 構造体に TypeInfo を埋め込むマクロ
#define REFLECT_STRUCT_BEGIN(struct_type) \
    namespace NoEngine_Reflection_##struct_type { \
        using ThisType = struct_type; \
        static NoEngine::TypeInfo s_typeInfo = { \
            NoEngine::Utilities::ExtractTypeName(#struct_type), \
            sizeof(struct_type), \
            NoEngine::Utilities::TypeID<struct_type>(), \
            std::vector<NoEngine::FieldInfo>{

#define REFLECT_STRUCT_END(struct_type) \
            }, \
            [](NoEngine::ECS::Registry& registry, NoEngine::ECS::Entity e) -> void* { \
                return registry.GetComponent<ThisType>(e); \
            } \
        }; \
        static bool s_registered = [](){ \
            NoEngine::ComponentRegistry::Register(s_typeInfo); \
            return true; \
        }(); \
    } \
    template<> inline NoEngine::TypeInfo* NoEngine::GetTypeInfo<struct_type>() { \
        return &NoEngine_Reflection_##struct_type::s_typeInfo; \
    }

// 単一フィールド定義用マクロ
#define REFLECT_FIELD(field_name) \
    { \
        #field_name, \
        offsetof(ThisType, field_name), \
        sizeof(((ThisType*)0)->field_name), \
        NoEngine::FieldTypeResolver<decltype(((ThisType*)0)->field_name)>::value, \
        NoEngine::FieldAttributes{} \
    }
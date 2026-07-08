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
            }, \
            [](NoEngine::ECS::Registry& registry, NoEngine::ECS::Entity e) -> void* { \
                return registry.AddComponent<ThisType>(e); \
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

// enumフィールド用（type判定 + magic_enum連携関数を両方セットする）
#define REFLECT_ENUM_FIELD(field_name) \
    [] { \
        using EnumT = decltype(((ThisType*)0)->field_name); \
        static_assert(std::is_enum_v<EnumT>, #field_name " is not an enum type"); \
        NoEngine::FieldInfo info{}; \
        info.name       = #field_name; \
        info.offset     = offsetof(ThisType, field_name); \
        info.size       = sizeof(((ThisType*)0)->field_name); \
        info.type       = NoEngine::FieldType::Enum; \
        info.attributes = NoEngine::FieldAttributes{}; \
        info.enumNames = [] { \
            std::vector<std::string> names; \
            for (auto n : magic_enum::enum_names<EnumT>()) names.emplace_back(n); \
            return names; \
        }; \
        info.enumGetIndex = [](const void* ptr) -> int { \
            EnumT v = *reinterpret_cast<const EnumT*>(ptr); \
            auto idx = magic_enum::enum_index(v); \
            return idx ? static_cast<int>(*idx) : 0; \
        }; \
        info.enumSetIndex = [](void* ptr, int index) { \
            auto values = magic_enum::enum_values<EnumT>(); \
            if (index >= 0 && index < static_cast<int>(values.size())) \
                *reinterpret_cast<EnumT*>(ptr) = values[index]; \
        }; \
        info.enumToString = [](const void* ptr) -> std::string { \
            return std::string(magic_enum::enum_name(*reinterpret_cast<const EnumT*>(ptr))); \
        }; \
        info.enumFromString = [](void* ptr, const std::string& s) { \
            if (auto v = magic_enum::enum_cast<EnumT>(s)) \
                *reinterpret_cast<EnumT*>(ptr) = *v; \
        }; \
        return info; \
    }()
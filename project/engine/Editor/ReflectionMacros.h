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
    template<> NoEngine::TypeInfo* NoEngine::GetTypeInfo<struct_type>() { \
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
        info.enumOps = std::make_shared<NoEngine::EnumFieldOps>(); \
        info.enumOps->names = [] { \
            std::vector<std::string> names; \
            for (auto n : magic_enum::enum_names<EnumT>()) names.emplace_back(n); \
            return names; \
        }; \
        info.enumOps->getIndex = [](const void* ptr) -> int { \
            EnumT v = *reinterpret_cast<const EnumT*>(ptr); \
            auto idx = magic_enum::enum_index(v); \
            return idx ? static_cast<int>(*idx) : 0; \
        }; \
        info.enumOps->setIndex = [](void* ptr, int index) { \
            auto values = magic_enum::enum_values<EnumT>(); \
            if (index >= 0 && index < static_cast<int>(values.size())) \
                *reinterpret_cast<EnumT*>(ptr) = values[index]; \
        }; \
        info.enumOps->toString = [](const void* ptr) -> std::string { \
            return std::string(magic_enum::enum_name(*reinterpret_cast<const EnumT*>(ptr))); \
        }; \
        info.enumOps->fromString = [](void* ptr, const std::string& s) { \
            if (auto v = magic_enum::enum_cast<EnumT>(s)) \
                *reinterpret_cast<EnumT*>(ptr) = *v; \
        }; \
        return info; \
    }()

// ネストされた構造体フィールド用マクロ
// ※ field_name の型はあらかじめ REFLECT_STRUCT_BEGIN/END で登録済みであること
#define REFLECT_STRUCT_FIELD(field_name) \
    [] { \
        using FieldT = decltype(((ThisType*)0)->field_name); \
        NoEngine::FieldInfo info{}; \
        info.name       = #field_name; \
        info.offset     = offsetof(ThisType, field_name); \
        info.size       = sizeof(((ThisType*)0)->field_name); \
        info.type       = NoEngine::FieldType::Struct; \
        info.attributes = NoEngine::FieldAttributes{}; \
        info.structTypeInfo = [] { return NoEngine::GetTypeInfo<FieldT>(); }; \
        return info; \
    }()

// std::vector<T> 配列フィールド用（T が FieldTypeResolver 対応のプリミティブ/数学型の場合）
#define REFLECT_ARRAY_FIELD(field_name) \
    [] { \
        using ArrayT   = decltype(((ThisType*)0)->field_name); \
        using ElementT = ArrayT::value_type; \
        NoEngine::FieldInfo info{}; \
        info.name       = #field_name; \
        info.offset     = offsetof(ThisType, field_name); \
        info.size       = sizeof(((ThisType*)0)->field_name); \
        info.type       = NoEngine::FieldType::Array; \
        info.attributes = NoEngine::FieldAttributes{}; \
        info.arrayOps = std::make_shared<NoEngine::ArrayFieldOps>(); \
        info.arrayOps->elementType = NoEngine::FieldTypeResolver<ElementT>::value; \
        info.arrayOps->elementSize = sizeof(ElementT); \
        info.arrayOps->size = [](const void* ptr) -> size_t { \
            return static_cast<const ArrayT*>(ptr)->size(); \
        }; \
        info.arrayOps->getElement = [](void* ptr, size_t index) -> void* { \
            return &(*static_cast<ArrayT*>(ptr))[index]; \
        }; \
        info.arrayOps->addElement = [](void* ptr) { \
            static_cast<ArrayT*>(ptr)->emplace_back(); \
        }; \
        info.arrayOps->removeElement = [](void* ptr, size_t index) { \
            ArrayT* arr = static_cast<ArrayT*>(ptr); \
            if (index < arr->size()) arr->erase(arr->begin() + index); \
        }; \
        info.arrayOps->insertElement = [](void* ptr, size_t index) { \
            ArrayT* arr = static_cast<ArrayT*>(ptr); \
            if (index > arr->size()) index = arr->size(); \
            arr->emplace(arr->begin() + index); \
        }; \
        return info; \
    }()

// std::vector<Struct> 配列フィールド用（要素が REFLECT_STRUCT_BEGIN/END 登録済み構造体の場合）
#define REFLECT_STRUCT_ARRAY_FIELD(field_name) \
    [] { \
        using ArrayT   = decltype(((ThisType*)0)->field_name); \
        using ElementT = ArrayT::value_type; \
        NoEngine::FieldInfo info{}; \
        info.name       = #field_name; \
        info.offset     = offsetof(ThisType, field_name); \
        info.size       = sizeof(((ThisType*)0)->field_name); \
        info.type       = NoEngine::FieldType::Array; \
        info.attributes = NoEngine::FieldAttributes{}; \
        info.arrayOps = std::make_shared<NoEngine::ArrayFieldOps>(); \
        info.arrayOps->elementType = NoEngine::FieldType::Struct; \
        info.arrayOps->elementSize = sizeof(ElementT); \
        info.arrayOps->elementStructTypeInfo = [] { return NoEngine::GetTypeInfo<ElementT>(); }; \
        info.arrayOps->size = [](const void* ptr) -> size_t { \
            return static_cast<const ArrayT*>(ptr)->size(); \
        }; \
        info.arrayOps->getElement = [](void* ptr, size_t index) -> void* { \
            return &(*static_cast<ArrayT*>(ptr))[index]; \
        }; \
        info.arrayOps->addElement = [](void* ptr) { \
            static_cast<ArrayT*>(ptr)->emplace_back(); \
        }; \
        info.arrayOps->removeElement = [](void* ptr, size_t index) { \
            ArrayT* arr = static_cast<ArrayT*>(ptr); \
            if (index < arr->size()) arr->erase(arr->begin() + index); \
        }; \
        info.arrayOps->insertElement = [](void* ptr, size_t index) { \
            ArrayT* arr = static_cast<ArrayT*>(ptr); \
            if (index > arr->size()) index = arr->size(); \
            arr->emplace(arr->begin() + index); \
        }; \
        return info; \
    }()

// 他の構造体からネスト/配列要素として参照可能にするための前方宣言マクロ
// 対象構造体のヘッダ (.h) に置く
#define REFLECT_STRUCT_DECLARE(struct_type) \
    namespace NoEngine { \
        template<> NoEngine::TypeInfo* GetTypeInfo<struct_type>(); \
    }
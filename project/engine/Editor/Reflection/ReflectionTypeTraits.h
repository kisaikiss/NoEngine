#pragma once
#include "Reflection.h"
#include "engine/Math/MathInclude.h"
#include "engine/Functions/ECS/Entity.h"
#include <type_traits>

// 型推論用ヘルパー
namespace NoEngine {

// 第2テンプレート引数を追加してSFINAEを効かせる
template<typename T, typename Enable = void>
struct FieldTypeResolver {
    static constexpr FieldType value = FieldType::Unknown;
};

// enum型を自動的にFieldType::Enumとして検出
template<typename T>
struct FieldTypeResolver<T, std::enable_if_t<std::is_enum_v<T>>> {
    static constexpr FieldType value = FieldType::Enum;
};

template<>
struct FieldTypeResolver<float> {
    static constexpr FieldType value = FieldType::Float;
};

template<>
struct FieldTypeResolver<Math::Vector2> {
    static constexpr FieldType value = FieldType::Float2;
};

template<>
struct FieldTypeResolver<Math::Vector3> {
    static constexpr FieldType value = FieldType::Float3;
};

template<>
struct FieldTypeResolver<Math::Vector4> {
    static constexpr FieldType value = FieldType::Float4;
};

template<>
struct FieldTypeResolver<Math::Color> {
    static constexpr FieldType value = FieldType::Color;
};

template<>
struct FieldTypeResolver<Math::Quaternion> {
    static constexpr FieldType value = FieldType::Float4;
};

template<>
struct FieldTypeResolver<Rect> {
    static constexpr FieldType value = FieldType::Float4;
};

template<>
struct FieldTypeResolver<int8_t> {
    static constexpr FieldType value = FieldType::Int;
};

template<>
struct FieldTypeResolver<int16_t> {
    static constexpr FieldType value = FieldType::Int;
};

template<>
struct FieldTypeResolver<int32_t> {
    static constexpr FieldType value = FieldType::Int;
};


template<>
struct FieldTypeResolver<int64_t> {
    static constexpr FieldType value = FieldType::Int;
};

template<>
struct FieldTypeResolver<uint8_t> {
    static constexpr FieldType value = FieldType::Uint;
};

template<>
struct FieldTypeResolver<uint16_t> {
    static constexpr FieldType value = FieldType::Uint;
};

template<>
struct FieldTypeResolver<uint32_t> {
    static constexpr FieldType value = FieldType::Uint;
};

template<>
struct FieldTypeResolver<bool> {
    static constexpr FieldType value = FieldType::Bool;
};

template<>
struct FieldTypeResolver<std::string> {
    static constexpr FieldType value = FieldType::String;
};

template<>
struct FieldTypeResolver<std::wstring> {
    static constexpr FieldType value = FieldType::WString;
};

template<>
struct FieldTypeResolver<ECS::Entity> {
    static constexpr FieldType value = FieldType::Entity;
};
}
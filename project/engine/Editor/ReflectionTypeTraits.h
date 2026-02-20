#pragma once
#include "Reflection.h"
#include "engine/Math/MathInclude.h"

// 型推論用ヘルパー
namespace NoEngine {

template<typename T>
struct FieldTypeResolver {
    static constexpr FieldType value = FieldType::Unknown;
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
    static constexpr FieldType value = FieldType::Float4;
};

template<>
struct FieldTypeResolver<Math::Quaternion> {
    static constexpr FieldType value = FieldType::Float4;
};

template<>
struct FieldTypeResolver<int> {
    static constexpr FieldType value = FieldType::Int;
};

template<>
struct FieldTypeResolver<bool> {
    static constexpr FieldType value = FieldType::Bool;
};

template<>
struct FieldTypeResolver<std::string> {
    static constexpr FieldType value = FieldType::String;
};


}
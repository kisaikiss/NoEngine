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
struct FieldTypeResolver<uint64_t> {
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
struct FieldTypeResolver<Component::BodyType> {
    static constexpr FieldType value = FieldType::Int;
};


}
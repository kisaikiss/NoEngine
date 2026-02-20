#pragma once
#include "engine/Functions/ECS/Registry.h"
// データドリブンに必要な基本型・メタ情報定義ファイル

namespace NoEngine {
/// <summary>
/// 構造体のメンバ変数の種類
/// </summary>
enum class FieldType {
    Unknown,
    Float,
    Float2,
    Float3,
    Float4,
    Int,
    Bool,
    String,
    Struct,
};

/// <summary>
/// メンバ変数の属性
/// </summary>
struct FieldAttributes {
    bool editable = true;   // エディタで使うか
    bool hasRange = false;  // 幅を持つか
    float minValue = 0.0f;  // 最小値
    float maxValue = 0.0f;  // 最大値
    float valueSpeed = 0.1f; // エディタで動かすときの速度;
};

struct TypeInfo; // 前方宣言

/// <summary>
/// メンバ変数の情報
/// </summary>
struct FieldInfo {
    std::string     name;       // 名前
    size_t          offset;
    size_t          size;
    FieldType       type;       // 型の種類
    FieldAttributes attributes; // 属性
};

struct TypeInfo {
    std::string name;
    size_t      size;
    size_t      typeId;
    std::vector<FieldInfo> fields;

    // RegistryからComponentを取得する関数
    std::function<void* (ECS::Registry&, ECS::Entity)> getter;
};


}
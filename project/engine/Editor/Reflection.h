#pragma once
#include <memory>
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
    Color,
    Int,
    Uint,
    Bool,
    String,
    WString,
    Enum,
    Struct,
    Array,
    Entity,
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
/// FieldType::Enum のときのみ使用する拡張情報。
/// EnumフィールドでないFieldInfoはこれをshared_ptrで持たない（nullptr）ため、
/// std::function 5個分のサイズをFieldInfo本体から切り離せる。
/// </summary>
struct EnumFieldOps {
    std::function<std::vector<std::string>()>     names;
    std::function<int(const void*)>                getIndex;
    std::function<void(void*, int)>                 setIndex;
    std::function<std::string(const void*)>         toString;
    std::function<void(void*, const std::string&)>  fromString;
};

/// <summary>
/// FieldType::Array (std::vector<T>) のときのみ使用する拡張情報。
/// Arrayフィールドでない FieldInfo はこれをshared_ptrで持たない（nullptr）ため、
/// std::function 6個分 + 要素情報のサイズをFieldInfo本体から切り離せる。
/// </summary>
struct ArrayFieldOps {
    FieldType elementType = FieldType::Unknown;        // 要素の型
    size_t    elementSize = 0;                          // 要素1個分のサイズ
    std::function<TypeInfo* ()>           elementStructTypeInfo; // 要素がStructのときのみ有効
    std::function<size_t(const void*)>   size;
    std::function<void* (void*, size_t)> getElement;
    std::function<void(void*)>           addElement;    // デフォルト構築で1個追加
    std::function<void(void*, size_t)>   removeElement; // index番目を削除
    std::function<void(void*, size_t)>   insertElement;
};

/// <summary>
/// メンバ変数の情報
/// </summary>
struct FieldInfo {
    std::string     name;       // 名前
    size_t          offset;
    size_t          size;
    FieldType       type;       // 型の種類
    FieldAttributes attributes; // 属性

    // FieldType::Struct のときのみ有効（ネストされた構造体のTypeInfoを取得）
    std::function<TypeInfo* ()> structTypeInfo;

    // FieldType::Enum / Array のときのみ確保される。
    // std::vector<FieldInfo> をinitializer_listで構築する既存マクロ構造と
    // 互換性を保つため、unique_ptrではなくコピー可能なshared_ptrを採用。
    std::shared_ptr<EnumFieldOps>  enumOps;
    std::shared_ptr<ArrayFieldOps> arrayOps;
};

struct TypeInfo {
    std::string name;
    size_t      size;
    size_t      typeId;
    std::vector<FieldInfo> fields;

    // RegistryからComponentを取得する関数
    std::function<void* (ECS::Registry&, ECS::Entity)> getter;
    // RegistryからComponentを追加する関数 
    std::function<void* (ECS::Registry&, ECS::Entity)> adder;
};


}
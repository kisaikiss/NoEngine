#pragma once
#include <dxcapi.h>
namespace NoEngine {
/// <summary>
/// シェーダーリフレクションクラス。このクラスがシェーダーリフレクションを元にルートシグネチャの自動生成などを行います。
/// </summary>
class ShaderReflection final {
public:
    /// <summary>
   /// 入力レイアウト
   /// </summary>
    struct InputParameter {
        std::string semanticName;
        uint32_t semanticIndex = 0;
        DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
        uint32_t inputSlot = 0;
        uint32_t alignedByteOffset = 0;
    };
    std::vector<InputParameter> inputs_;

    /// <summary>
    /// リソースタイプ
    /// </summary>
    enum class ResourceType {
        kConstantBuffer,
        kTexture,
        kSampler,
        kRWTexture,
        kStructuredBuffer,
        kRWStructuredBuffer,
        kByteAddressBuffer,
        kRWByteAddressBuffer,
        kUnknown
    };

    /// <summary>
    /// リソースバインディング
    /// </summary>
    struct ResourceBinding {
        std::string name;
        ResourceType type = ResourceType::kUnknown;
        uint32_t bindPoint = 0;      // register(b0) の 0
        uint32_t space = 0;          // register(b0, space1)
        uint32_t bindCount = 1;      // 配列対応
    };
    std::vector<ResourceBinding> resources_;

    // コンピュートシェーダーに使用します。
    uint32_t threadGroupX_ = 0;
    uint32_t threadGroupY_ = 0;
    uint32_t threadGroupZ_ = 0;

    // メタ情報
    std::string entryPoint_;
    std::string targetProfile_;
    uint32_t shaderModelMajor_ = 0;
    uint32_t shaderModelMinor_ = 0;

    /// <summary>
    /// ルートシグネチャ自動生成用
    /// </summary>
    struct RootParameterInfo {
        ResourceType type;
        uint32_t bindPoint;
        uint32_t space;
    };
    std::vector<RootParameterInfo> rootParams_;

    ShaderReflection() = default;
    ~ShaderReflection() = default;

    void ReflectShader(const std::vector<uint8_t>& bytecode);

   
    const std::string& GetDebugDump() { return debugDump_; }
private:

    // デバッグ / Inspector用
    std::string debugDump_;

    /// <summary>
    /// リフレクションで取得したインプットレイアウトの情報からフォーマットを推定します。
    /// </summary>
    /// <param name="mask">リフレクションで取得したマスク</param>
    /// <param name="type">リフレクションで取得したタイプ</param>
    /// <returns>推定したフォーマット</returns>
    DXGI_FORMAT GuessFormatFromMask(UINT mask, D3D_REGISTER_COMPONENT_TYPE type);

    /// <summary>
    /// リフレクションで取得したシェーダーインプットタイプをこのクラスで定義したenumへ変換します。
    /// </summary>
    /// <param name="t">リフレクションで取得したシェーダーインプットタイプ</param>
    /// <returns>リフレクションクラスのリソースタイプ</returns>
    ResourceType ConvertResourceType(D3D_SHADER_INPUT_TYPE t);

    /// <summary>
    /// メンバ変数のデバッグダンプへ情報を書き込みます。
    /// </summary>
    void BuildDebugDump();

    /// <summary>
    /// リソースタイプを文字列へ変換します。
    /// </summary>
    /// <param name="t"></param>
    /// <returns></returns>
    std::string ResourceTypeToString(ShaderReflection::ResourceType t);
};
}
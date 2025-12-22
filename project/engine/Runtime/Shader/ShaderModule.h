#pragma once
#include "ShaderReflection.h"

namespace NoEngine {
/// <summary>
/// シェーダー一つ一つを表すクラス
/// </summary>
class ShaderModule final {
public:
	enum class Stage {
		Vertex,
		Pixel,
		Compute,
		Geometry,
		Hull,
		Domain
	};

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="stage">シェーダーの種類</param>
	/// <param name="filePath">シェーダーのfilePath</param>
	/// <param name="profile">シェーダーのprofile</param>
	ShaderModule(Stage stage, const std::wstring& filePath, const std::wstring& profile);
	~ShaderModule();

	static void Initialize();
	static void Shutdown();

	/// <summary>
	/// 必要な時に呼び出し、コンパイルします。
	/// </summary>
	/// <returns>コンパイルに成功したかどうか(成功 : true, 失敗 : false)</returns>
	bool CompileIfNeeded();

	/// <summary>
	/// バイナリ保存します。
	/// </summary>
	/// <param name="path"></param>
	void SaveBinary(const std::wstring& path) const;

	/// <summary>
	/// バイナリ読み込みます。
	/// </summary>
	/// <param name="path"></param>
	/// <returns></returns>
	bool LoadBinary(const std::wstring& path);

	/// <summary>
	/// PSOに渡すバイトコードを取得します。
	/// </summary>
	/// <returns></returns>
	D3D12_SHADER_BYTECODE GetBytecode() const;

	// リフレクション情報
   const ShaderReflection& GetReflection() const;

	/// <summary>
	/// ソースが更新されているかチェックします。
	/// </summary>
	/// <returns>更新されているかどうか(更新あり : true, 更新なし : false)</returns>
	bool CheckForSourceUpdate();

private:
	Stage stage_;
	std::wstring filePath_;
	std::wstring profile_;

	std::wstring cachePath_;

	std::vector<uint8_t> bytecode_;
	ShaderReflection reflection_;

	std::time_t sourceTimestamp_ = 0;
	size_t compileHash_ = 0;

	/// <summary>
	/// シェーダーのキャッシュ判定に使うハッシュを生成します。
	/// </summary>
	/// <returns></returns>
	size_t ComputeCompileHash();

	/// <summary>
	/// シェーダーをコンパイルします。
	/// </summary>
	/// <param name="outBlob">コンパイル結果</param>
	/// <returns>成功したかどうか(成功 : true, 失敗 : false)</returns>
	bool CompileShader(Microsoft::WRL::ComPtr<IDxcBlob>& outBlob);
};
}

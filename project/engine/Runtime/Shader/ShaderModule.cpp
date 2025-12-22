#include "ShaderModule.h"
#include "../../Utilities/Conversion/ConvertString.h"
#include "../../Utilities/FileUtilities.h"
#include "engine/Debug/Logger/Log.h"

#pragma comment(lib, "dxcompiler.lib")

namespace NoEngine {

namespace {
Microsoft::WRL::ComPtr<IDxcUtils> sDxcUtils;
Microsoft::WRL::ComPtr <IDxcCompiler3> sDxcCompiler;
Microsoft::WRL::ComPtr <IDxcIncludeHandler> sIncludeHandler;
bool isInitialized = false;
}

ShaderModule::ShaderModule(Stage stage, const std::wstring& filePath, const std::wstring& profile) :
	stage_(stage),
	filePath_(filePath),
	profile_(profile) {
	// ソースファイルのタイムスタンプを取得
	namespace fs = std::filesystem;
	auto ftime = fs::last_write_time(filePath_);
	auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
		ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
	);
	sourceTimestamp_ = std::chrono::system_clock::to_time_t(sctp);

	// コンパイル結果のハッシュを計算
	compileHash_ = ComputeCompileHash();

	// キャッシュを保存するディレクトリパス
	std::wstring cacheDir = L"resources/engine/cache";
	std::filesystem::create_directory(cacheDir.c_str());
	// hlslソースファイル名だけを抽出します。（拡張子付き）
	std::wstring fileName = std::filesystem::path(filePath).filename().wstring();
	// 拡張子を除いたベース名を取得します。
	std::wstring stem = std::filesystem::path(fileName).stem().wstring();
	// compileHash を 16進数文字列に変換します。
	wchar_t hashStr[32];
	swprintf(hashStr, 32, L"%016llX", compileHash_);
	// キャッシュパスを組み立てます。
	cachePath_ = cacheDir + L"/" + stem + L"_" + hashStr + L".dxil";

	// キャッシュバイナリが存在すれば読み込みます。
	if (!LoadBinary(cachePath_)) {
		// キャッシュがなければコンパイルを試みます。
		CompileIfNeeded();
	}

}

ShaderModule::~ShaderModule() {}


void ShaderModule::Initialize() {
	if (isInitialized) {
		return;
	}
	// dxCompilerを初期化
	// DXCはDirectX12とは別のものなので、別で初期化する
	HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&sDxcUtils));
	if (FAILED(hr)) {
		Log::DebugPrint("DxcUtiles Create failed", VerbosityLevel::kCritical);
		assert(false);
	}
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&sDxcCompiler));
	if (FAILED(hr)) {
		Log::DebugPrint("DxcCompiler Create failed", VerbosityLevel::kCritical);
		assert(false);
	}

	//現時点ではincludeしないが、includeに対応するための設定を行っておく
	hr = sDxcUtils->CreateDefaultIncludeHandler(&sIncludeHandler);
	if (FAILED(hr)) {
		Log::DebugPrint("IncludeHandler Create failed", VerbosityLevel::kCritical);
		assert(false);
	}

	isInitialized = true;
}

void ShaderModule::Shutdown() {
	sIncludeHandler.Reset();
	sDxcCompiler.Reset();
	sDxcUtils.Reset();
}


bool ShaderModule::CompileIfNeeded() {
	// ソースファイルの更新時刻を取得します。
	std::time_t newTimestamp{};
	{
		namespace fs = std::filesystem;
		auto ftime = fs::last_write_time(filePath_);
		auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
			ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
		);
		newTimestamp = std::chrono::system_clock::to_time_t(sctp);
		// ソースファイルが更新されていなければコンパイルは不要なのでリターンします。
		if (newTimestamp == sourceTimestamp_ && !bytecode_.empty()) {
			return false;
		}
	}

	// ソースへの内容とプロファイルからハッシュ化します。
	size_t newHash = ComputeCompileHash();

	// ハッシュが同じならキャッシュバイナリを使います。
	if (newHash == compileHash_ && LoadBinary(cachePath_)) {
		sourceTimestamp_ = newTimestamp;
		return true;
	}

	// DXCでコンパイルします。
	Microsoft::WRL::ComPtr<IDxcBlob> compiled;
	if (!CompileShader(compiled)) {
		assert(false);
	}

	// bytecodeをコピーします。
	// vectorのassign関数はリサイズと全ての要素を一つの値で埋める機能の他に、指定した範囲をコピーする機能があります。
	bytecode_.assign(
		(uint8_t*)compiled->GetBufferPointer(),
		(uint8_t*)compiled->GetBufferPointer() + compiled->GetBufferSize());

	// リフレクションを実行
	reflection_.ReflectShader(bytecode_);

	// キャッシュ保存
	SaveBinary(cachePath_);

	//タイムスタンプとハッシュを更新
	sourceTimestamp_ = newTimestamp;
	compileHash_ = newHash;


	return false;
}

void ShaderModule::SaveBinary(const std::wstring& path) const {
	// バイナリモードで開く
	std::ofstream ofs(path, std::ios::binary);
	if (!ofs)
		return;

	// 書き込み
	ofs.write((char*)bytecode_.data(), bytecode_.size());
}

bool ShaderModule::LoadBinary(const std::wstring& path) {
	std::ifstream ifs(path, std::ios::binary);
	if (!ifs) {
		return false;
	}

	// ファイルサイズ取得
	ifs.seekg(0, std::ios::end);
	size_t size = ifs.tellg();
	ifs.seekg(0, std::ios::beg);

	// バッファ確保
	bytecode_.resize(size);

	// 読み込み
	ifs.read((char*)bytecode_.data(), size);

	// リフレクションも再構築
	reflection_.ReflectShader(bytecode_);

	return true;

}

D3D12_SHADER_BYTECODE ShaderModule::GetBytecode() const {
	D3D12_SHADER_BYTECODE bc = {};
	bc.pShaderBytecode = bytecode_.data();
	bc.BytecodeLength = bytecode_.size();
	return bc;
}

const ShaderReflection& ShaderModule::GetReflection() const {
	return reflection_;
}

bool ShaderModule::CheckForSourceUpdate() {
	std::time_t newTimestamp{};

	namespace fs = std::filesystem;
	auto ftime = fs::last_write_time(filePath_);
	auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
		ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
	);
	newTimestamp = std::chrono::system_clock::to_time_t(sctp);
	// ソースファイルが更新されていなければコンパイル不要なのでリターンします。
	if (newTimestamp == sourceTimestamp_ && !bytecode_.empty()) {
		return false;
	}

	return true;
}


size_t ShaderModule::ComputeCompileHash() {
	// ハッシュ用の文字列を構築
	std::wstring data;

	// ソースファイルの内容を追加
	data += Utilities::LoadFileAsString(filePath_);

	// プロファイルを追加
	data += profile_;

	// 6. ハッシュ化（FNV-1a 64bit）
	size_t hash = 1469598103934665603ull;
	for (wchar_t c : data) {
		hash ^= (size_t)c;
		hash *= 1099511628211ull;
	}

	return hash;

}

bool ShaderModule::CompileShader(Microsoft::WRL::ComPtr<IDxcBlob>& outBlob) {
#pragma region 1.hlslファイルを読む
	// hlslファイルの内容ををDXCの機能を利用して読み、コンパイラに渡すための設定を行います。
	// これからシェーダーをコンパイルする旨をログに出します。
	NoEngine::Log::DebugPrint(NoEngine::ConvertString(std::format(L"Begin CompileShader, path:{}, profile:{}\n", filePath_, profile_)));
	// hlslファイルをロードしてバッファにします。
	Microsoft::WRL::ComPtr<IDxcBlobEncoding> shaderSource = nullptr;
	HRESULT hr = sDxcUtils->LoadFile(filePath_.c_str(), nullptr, &shaderSource);
	// 読めなかったら止めます。
	assert(SUCCEEDED(hr));

	// 読み込んだファイルの内容を設定します。
	DxcBuffer shaderSourceBuffer{};
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;	//UTF8の文字コードであることを通知
#pragma endregion

#pragma region 2.Compileする
	LPCWSTR arguments[] = {			// コンパイルオプション
		filePath_.c_str(),			// コンパイル対象のhlslファイル名
		L"-E", L"main",				// エントリーポイントの指定、基本的にmain以外にはしません。
		L"-T", profile_.c_str(),				// ShaderProfileの設定
		L"-Zi", L"-Qembed_debug",	// デバッグ用の情報を埋め込みます。
		L"-Od",						// 最適化を外しておきます。
		L"-Zpr",					// メモリレイアウトは行優先にします。
	};

	// 実際にShaderをコンパイルします。
	Microsoft::WRL::ComPtr<IDxcResult> shaderResult = nullptr;
	hr = sDxcCompiler->Compile(
		&shaderSourceBuffer,		// 読み込んだファイル
		arguments,					// コンパイルオプション
		_countof(arguments),		// コンパイルオプションの数
		sIncludeHandler.Get(),		// includeが含まれた諸々
		IID_PPV_ARGS(&shaderResult)	// コンパイル結果
	);

	// コンパイルエラーではなくdxcが起動できないなど致命的な状況
	assert(SUCCEEDED(hr));
#pragma endregion

#pragma region 3.警告・エラーがでていないか確認する
	// 警告・エラーが出ていたらログに出して止めます。
	Microsoft::WRL::ComPtr<IDxcBlobUtf8> shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		NoEngine::Log::DebugPrint(shaderError->GetStringPointer());
		assert(false);
	}
#pragma endregion

#pragma region 4.Compile結果を受け取り それが成功したかどうか返します
	// BLOBとはBinaryLargeOBjectの略で、大きなバイナリデータの塊を表します。
	// コンパイル結果から実行用のバイナリ部分を取得します。
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&outBlob), nullptr);
	assert(SUCCEEDED(hr));
	// 成功したログを出します。
	NoEngine::Log::DebugPrint(NoEngine::ConvertString(std::format(L"Compile Succeeded, path:{}, profile:{}\n", filePath_, profile_)));
	// もう使わないリソースを解放します。
	shaderSource.Reset();
	shaderResult.Reset();
	// コンパイル成功
	return true;
#pragma endregion
}

}
#pragma once
namespace NoEngine {
namespace Utilities {
// TypeID()はsize_tをstd::type_indexのように扱う目的の関数です。
// size_t型で返すのでarrayやvectorにも使用できます。
// TypeIndexはTypeID()内で使う関数を秘匿するクラスです。std::mapのKeyには使用できません。

/// <summary>
/// IDを発行するクラス。専用の関数以外から呼び出されないように発行する関数を秘匿します。
/// </summary>
class TypeIndex final {
public:
	TypeIndex(const TypeIndex& obj) = delete;
	TypeIndex& operator=(const TypeIndex& obj) = delete;
private:
	static size_t CreateNextTypeID() {
		static size_t i = 0;
		return i++;
	}

	template<typename T>
	friend size_t TypeID();

	TypeIndex() = default;
	~TypeIndex() = default;

};

/// <summary>
/// テンプレートで指定したタイプごとに一意なIDを発行します。
/// </summary>
/// <returns>ID</returns>
template<typename T>
size_t TypeID() {
    static size_t i = TypeIndex::CreateNextTypeID();
    return i;
}
}
}
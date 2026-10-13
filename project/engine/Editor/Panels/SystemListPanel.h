#pragma once

namespace NoEngine {
namespace ECS { class SystemManager; }

namespace Editor {

/// <summary>
/// 「Systems」ウィンドウ。システムの追加・削除・ドラッグ並べ替えを行う。
/// 実際の操作はSystemManagerの公開メソッドを呼ぶだけで、保存は呼び出し側が行う。
/// </summary>
class SystemListPanel {
public:
	/// <returns>構成が変更された場合true(保存のトリガーに使う)</returns>
	bool Draw(ECS::SystemManager& manager);

private:
	bool DrawAddCombo(ECS::SystemManager& manager);
	bool DrawSystemList(ECS::SystemManager& manager);
};

}
}
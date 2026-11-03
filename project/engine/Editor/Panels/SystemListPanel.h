#pragma once

namespace NoEngine {
namespace ECS { class SystemManager; }

namespace Editor {

/// <summary>
/// 「Systems」ウィンドウ。行選択・指定位置への追加・削除・並べ替え・履歴操作を行う。
/// 実際の操作はSystemManagerの公開メソッドを呼ぶだけで、保存は呼び出し側が行う。
/// </summary>
class SystemListPanel {
public:
	/// <returns>構成が変更された場合true(保存のトリガーに使う)</returns>
	bool Draw(ECS::SystemManager& manager);

	int GetSelectedIndex() const { return selectedIndex_; }
	void Select(int index) { selectedIndex_ = index; scrollToSelection_ = true; }
	void Reset() { selectedIndex_ = -1; scrollToSelection_ = false; }

private:
	bool DrawAddCombo(ECS::SystemManager& manager);
	bool DrawAddMenu(ECS::SystemManager& manager, size_t insertionIndex);
	bool DrawSystemList(ECS::SystemManager& manager);

	int selectedIndex_ = -1;
	bool scrollToSelection_ = false;
};

}
}

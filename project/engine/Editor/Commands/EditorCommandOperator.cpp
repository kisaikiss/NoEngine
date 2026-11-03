#include "stdafx.h"
#include "EditorCommandOperator.h"
#include "engine/Editor/Utilities/MultiEdit.h"
#include "engine/Functions/ECS/System/Editor/DrawManipulatorSystem.h"
#include "engine/Functions/Input/input.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/ImGuizmo.h"
#endif

namespace NoEngine {
namespace Editor {
namespace {
Command::CommandManager sEditorCommands;
float sTimeInterval = 0.0f;
}

void EditorCommandOperator::AddCommand(std::unique_ptr<Command::ICommand> cmd) {
	sEditorCommands.AddCommand(std::move(cmd));
}

void EditorCommandOperator::Update(float deltaTime) {
	if (sTimeInterval > 0.0f) {
		sTimeInterval -= deltaTime;
	}

#ifdef USE_IMGUI
	// 入力欄自身のUndoやギズモのドラッグ中に、シーンの履歴を操作しない。
	if (ImGui::IsAnyItemActive() || ImGuizmo::IsUsingAny()) return;
#endif
	static const float kIntervalTime = 0.3f;
	if (Input::Keyboard::IsPress(VK_LCONTROL) && Input::Keyboard::IsPress(VK_LSHIFT) && Input::Keyboard::IsPress('Z')) {
		if (sTimeInterval <= 0.0f) {
			sEditorCommands.Redo();
			sTimeInterval = kIntervalTime;
		}
	} else if (Input::Keyboard::IsPress(VK_LCONTROL) && Input::Keyboard::IsPress('Z')) {
		if (sTimeInterval <= 0.0f) {
			sEditorCommands.Undo();
			sTimeInterval = kIntervalTime;
		}
	} 
}

void EditorCommandOperator::Undo() {
	sEditorCommands.Undo();
}

void EditorCommandOperator::Redo() {
	sEditorCommands.Redo();
}

bool EditorCommandOperator::CanUndo() {
	return sEditorCommands.CanUndo();
}

bool EditorCommandOperator::CanRedo() {
	return sEditorCommands.CanRedo();
}

void EditorCommandOperator::Reset() {
	LogDebug("EditorCommand Reset");
	ResetPendingMultiEdit();
	ECS::DrawManipulatorSystem::ResetSelectionEdit();
	sEditorCommands.Clear();
}

void EditorCommandOperator::Shutdown() {
	ResetPendingMultiEdit();
	ECS::DrawManipulatorSystem::ResetSelectionEdit();
	sEditorCommands.Clear();
}

}
}

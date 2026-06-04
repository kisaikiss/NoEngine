#include "stdafx.h"
#include "EditorCommandOperator.h"
#include "engine/Functions/Input/input.h"

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

void EditorCommandOperator::Shutdown() {
	sEditorCommands.~CommandManager();
}

}
}
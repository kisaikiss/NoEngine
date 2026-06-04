#include "stdafx.h"
#include "CommandManager.h"

namespace NoEngine {
namespace Command {
void CommandManager::AddCommand(std::unique_ptr<ICommand> cmd) {
    // 新しい操作が行われたらRedoスタックはクリアする
    redoStack_.clear();
    LogInfo("AddCommand name : " + cmd->name_);
    undoStack_.push_back(std::move(cmd));

    static const size_t kMaxUndoSteps = 150;
    if (undoStack_.size() > kMaxUndoSteps) {
        undoStack_.pop_front();
    }
}

void CommandManager::Undo() {
    if (undoStack_.empty()) return;
    auto cmd = std::move(undoStack_.back());
    undoStack_.pop_back();

    cmd->Undo();
    LogInfo("Undo name : " + cmd->name_);
    redoStack_.push_back(std::move(cmd));
}

void CommandManager::Redo() {
    if (redoStack_.empty()) return;
    auto cmd = std::move(redoStack_.back());
    redoStack_.pop_back();

    cmd->Execute();
    LogInfo("Redo name : " + cmd->name_);
    undoStack_.push_back(std::move(cmd));
}
}
}
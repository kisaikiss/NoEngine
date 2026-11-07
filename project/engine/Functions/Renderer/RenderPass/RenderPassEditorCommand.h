#pragma once
#include "RenderPassEditorModel.h"
#include <deque>
#include <memory>
#include "engine/Functions/Command/CommandManager.h"

namespace NoEngine::Render::EditorModel {
class EditCommand final : public Command::ICommand {
public:
    EditCommand(std::shared_ptr<Document> document, Json before, Json after)
        : ICommand("描画パスの編集"), document_(std::move(document)), before_(std::move(before)), after_(std::move(after)) {}
    // 履歴は編集データだけを変更する。保存・描画への適用は明示的な操作に限定する。
    void Execute() override { document_->draft = after_; }
    void Undo() override { document_->draft = before_; }
private:
    std::shared_ptr<Document> document_;
    Json before_;
    Json after_;
};
}

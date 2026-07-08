#pragma once
#include "../CommandManager.h"

namespace NoEngine {
namespace Command {
class FunctionCommand : public ICommand {
public:
    using Func = std::function<void()>;

    FunctionCommand(Func doFunc, Func undoFunc, const std::string& name = "FunctionCommand")
        : ICommand(name), doFunc_(std::move(doFunc)), undoFunc_(std::move(undoFunc)) {}

    void Execute() override { doFunc_(); }
    void Undo() override { undoFunc_(); }

private:
    Func doFunc_;
    Func undoFunc_;
};
}
}
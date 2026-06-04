#pragma once
#include "../CommandManager.h"


namespace NoEngine {
namespace Command {
// プリミティブ型（Int, Float, Boolなど）用のコマンド
template<typename T>
class ChangeValueCommand : public ICommand {
public:
    ChangeValueCommand(T* target, const T& oldVal, const T& newVal)
        : ptr_(target), oldValue_(oldVal), newValue_(newVal), ICommand("ChangeValueCommand") {}

    void Execute() override { *ptr_ = newValue_; }
    void Undo() override { *ptr_ = oldValue_; }
private:
    T* ptr_;
    T oldValue_;
    T newValue_;
};
}
}
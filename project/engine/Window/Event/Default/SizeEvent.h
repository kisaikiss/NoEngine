#pragma once
#include "../IWindowEvent.h"

namespace NoEngine {
class SizeEvent :
    public IWindowEvent {
public:
    SizeEvent() : IWindowEvent(WM_SIZE) {}
    std::optional<LRESULT> OnEvent(UINT msg, WPARAM wparam, LPARAM lparam) override;
};
}


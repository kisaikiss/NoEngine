#pragma once
#include "../IWindowEvent.h"
namespace NoEngine {
class SizingEvent :
    public IWindowEvent {
public:
    SizingEvent() : IWindowEvent(WM_SIZING) {}
    std::optional<LRESULT> OnEvent(UINT msg, WPARAM wparam, LPARAM lparam) override;
};
}


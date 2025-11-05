#pragma once
#include "../IWindowEvent.h"
namespace NoEngine {
class GetMinMaxInfoEvent :
    public IWindowEvent {
public:
	GetMinMaxInfoEvent() : IWindowEvent(WM_GETMINMAXINFO) {}
	std::optional<LRESULT> OnEvent(UINT msg, WPARAM wparam, LPARAM lparam) override;
};
}

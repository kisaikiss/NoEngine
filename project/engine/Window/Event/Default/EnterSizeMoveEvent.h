#pragma once
#include "../IWindowEvent.h"
namespace NoEngine {
class EnterSizeMoveEvent :
	public IWindowEvent {
public:
	EnterSizeMoveEvent() : IWindowEvent(WM_ENTERSIZEMOVE) {}
	std::optional<LRESULT> OnEvent(UINT msg, WPARAM wparam, LPARAM lparam) override;
};
}
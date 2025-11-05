#pragma once
#include "../IWindowEvent.h"
namespace NoEngine
{
class CloseEvent :
    public IWindowEvent {
public:
	CloseEvent() : IWindowEvent(WM_CLOSE) {}
	std::optional<LRESULT> OnEvent(UINT msg, WPARAM wparam, LPARAM lparam) override;
};
}


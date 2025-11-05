#pragma once
#include "../IWindowEvent.h"
namespace NoEngine
{
class MainEditorWindowCloseEvent final : public IWindowEvent {
public:
	MainEditorWindowCloseEvent() : IWindowEvent(WM_CLOSE){}
	std::optional<LRESULT> OnEvent(UINT msg, WPARAM wparam, LPARAM lparam) override;
};
}


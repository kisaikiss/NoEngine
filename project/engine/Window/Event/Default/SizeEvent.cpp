#include "stdafx.h"
#include "SizeEvent.h"
#include "../../Window.h"

std::optional<LRESULT> NoEngine::SizeEvent::OnEvent(UINT msg, WPARAM wparam, LPARAM lparam) {
	static_cast<void>(msg);
	if (wparam == SIZE_MINIMIZED) {
		return 0;
	}
	UINT width  = LOWORD(lparam);
	UINT height = HIWORD(lparam);
	GetWindow()->OnResizeSignal(width, height);
	return 0;
}
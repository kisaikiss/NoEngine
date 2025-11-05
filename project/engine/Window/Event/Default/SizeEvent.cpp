#include "stdafx.h"
#include "SizeEvent.h"
#include "../../Window.h"

std::optional<LRESULT> NoEngine::SizeEvent::OnEvent(UINT /*msg*/, WPARAM /*wparam*/, LPARAM lparam) {
    UINT width = LOWORD(lparam);
    UINT height = HIWORD(lparam);
    WindowSize& size = GetWindow()->GetWindowSize();
    size.clientWidth = static_cast<int32_t>(width);
    size.clientHeight = static_cast<int32_t>(height);
    GetWindow()->CalculateAspectRatio();

    return std::nullopt;
}

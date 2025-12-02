#include "stdafx.h"
#include "SizeEvent.h"
#include "../../Window.h"

std::optional<LRESULT> NoEngine::SizeEvent::OnEvent(UINT /*msg*/, WPARAM /*wparam*/, LPARAM lparam) {
    UINT width = LOWORD(lparam);
    UINT height = HIWORD(lparam);
   
    GetWindow()->SetWindowSize(width, height);

    return std::nullopt;
}

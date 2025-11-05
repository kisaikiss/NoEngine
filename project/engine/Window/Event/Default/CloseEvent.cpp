#include "stdafx.h"
#include "CloseEvent.h"
#include "../../Window.h"

std::optional<LRESULT> NoEngine::CloseEvent::OnEvent(UINT /*msg*/, WPARAM /*wparam*/, LPARAM /*lparam*/) {
    GetWindow()->Destroy();
    return 0;
}

#include "stdafx.h"
#include "EnterSizeMoveEvent.h"

std::optional<LRESULT> NoEngine::EnterSizeMoveEvent::OnEvent(UINT msg, WPARAM /*wparam*/, LPARAM /*lparam*/) {
	if (msg != kTargetMessage_) return std::nullopt;
	return std::nullopt;
}

#pragma once

namespace NoEngine
{
class Window;
class IWindowEvent {
public:
	IWindowEvent(UINT targetMessage) : kTargetMessage_(targetMessage){}
	virtual ~IWindowEvent() = default;

	Window* GetWindow() { return window_; }
	UINT GetTargetMassage() { return kTargetMessage_; }
	void SetWindow(Window* window) { window_ = window; }
	virtual std::optional<LRESULT> OnEvent(UINT msg, WPARAM wparam, LPARAM lparam) = 0;
protected:
	const UINT kTargetMessage_ = 0;
private:
	Window* window_ = nullptr;
	
};
}


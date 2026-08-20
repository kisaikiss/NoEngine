#pragma once
struct TitleCursorComponent {
	enum class Options {
		kStart,
		kQuit
	};
	Options options;
	float positionDuration = 30.0f;
	bool selected = false;
};
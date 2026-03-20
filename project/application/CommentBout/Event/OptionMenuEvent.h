#pragma once

namespace CommentBout {
enum class OptionMenuOwner {
	Unknown = 0,
	Pause,
	Title,
};

struct OpenOptionMenuEvent {
	OptionMenuOwner owner = OptionMenuOwner::Unknown;
};

struct OptionMenuClosedEvent {
	OptionMenuOwner owner = OptionMenuOwner::Unknown;
};
}

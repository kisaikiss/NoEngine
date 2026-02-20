#pragma once
enum class Phase {
	ONE,
	TWO,
	END
};

struct PhaseComponent {
	//static化します
	static inline Phase phase = Phase::ONE;
};
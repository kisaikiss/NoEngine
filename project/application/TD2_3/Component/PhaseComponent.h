#pragma once
enum class Phase {
	ONE,
	TWO,
	END
};

struct PhaseComponent {
	Phase phase = Phase::ONE;
};
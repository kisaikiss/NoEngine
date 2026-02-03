#pragma once
enum class Phase {
	ONE,
	TWO,
};

struct PhaseComponent {
	Phase phase = Phase::ONE;
};
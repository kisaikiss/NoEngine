#include "Color.h"
namespace NoEngine {
const Color Color::WHITE(1.f, 1.f, 1.f, 1.f);
const Color Color::BLACK(0.f, 0.f, 0.f, 1.f);
const Color Color::RED(1.f, 0.f, 0.f, 1.f);

uint32_t Color::ToRGBA8() const {
	uint32_t R = static_cast<uint32_t>(std::clamp(r, 0.0f, 1.0f) * 255.0f + 0.5f);
	uint32_t G = static_cast<uint32_t>(std::clamp(g, 0.0f, 1.0f) * 255.0f + 0.5f);
	uint32_t B = static_cast<uint32_t>(std::clamp(b, 0.0f, 1.0f) * 255.0f + 0.5f);
	uint32_t A = static_cast<uint32_t>(std::clamp(a, 0.0f, 1.0f) * 255.0f + 0.5f);

	return (R << 24) | (G << 16) | (B << 8) | A;
}

}
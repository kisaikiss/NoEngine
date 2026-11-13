#include "NumberDisplayComponent.h"

REFLECT_STRUCT_BEGIN(NumberDisplayComponent, "Application")
REFLECT_FIELD(value),
REFLECT_FIELD(digitCount),
REFLECT_ARRAY_FIELD(digitEntities)
REFLECT_STRUCT_END(NumberDisplayComponent)

REFLECT_STRUCT_BEGIN(NumberDigitTag, "ApplicationTag")
REFLECT_FIELD(digitIndex)
REFLECT_STRUCT_END(NumberDigitTag)

REFLECT_STRUCT_BEGIN(GameResultNumberComponent, "Application")
REFLECT_ENUM_FIELD(source)
REFLECT_STRUCT_END(GameResultNumberComponent)

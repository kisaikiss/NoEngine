#include "DialogueComponent.h"

REFLECT_STRUCT_BEGIN(DialogueComponent, "Application")
REFLECT_ARRAY_FIELD(lines),
REFLECT_FIELD(fontName),
REFLECT_FIELD(portraitTexture),
REFLECT_FIELD(portraitFrameCount),
REFLECT_ARRAY_FIELD(voiceFiles),
REFLECT_FIELD(voiceVolume),
REFLECT_FIELD(interactDistance),
REFLECT_FIELD(interactable)
REFLECT_STRUCT_END(DialogueComponent)

REFLECT_STRUCT_BEGIN(DialogueAutoTriggerComponent, "Application")
REFLECT_FIELD(radius),
REFLECT_FIELD(once)
REFLECT_STRUCT_END(DialogueAutoTriggerComponent)

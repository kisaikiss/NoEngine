#include "TitleMenuConfigComponent.h"
#include "TitleMenuStateComponent.h"




REFLECT_STRUCT_BEGIN(TitleMenuConfigComponent)
REFLECT_FIELD(backgroundPosition),
REFLECT_FIELD(backgroundSize),
REFLECT_FIELD(backgroundRotation),
REFLECT_FIELD(backgroundColor),
REFLECT_FIELD(panelPosition),
REFLECT_FIELD(panelSize),
REFLECT_FIELD(panelRotation),
REFLECT_FIELD(panelColor),
REFLECT_FIELD(logoPosition),
REFLECT_FIELD(logoSize),
REFLECT_FIELD(logoBaseRotation),
REFLECT_FIELD(itemBasePosition),
REFLECT_FIELD(itemSize),
REFLECT_FIELD(itemSpacing),
REFLECT_FIELD(cursorOffset),
REFLECT_FIELD(cursorSize),
REFLECT_FIELD(itemColor),
REFLECT_FIELD(selectedItemColor),
REFLECT_FIELD(cursorColor),
REFLECT_FIELD(selectedScale),
REFLECT_FIELD(confirmScale),
REFLECT_FIELD(confirmDuration),
REFLECT_FIELD(logoMotionPosAmplitude),
REFLECT_FIELD(logoMotionPosSpeed),
REFLECT_FIELD(logoMotionScaleAmplitude),
REFLECT_FIELD(logoMotionScaleSpeed),
REFLECT_FIELD(logoMotionRotateAmplitude),
REFLECT_FIELD(logoMotionRotateSpeed)
REFLECT_STRUCT_END(TitleMenuConfigComponent)

REFLECT_STRUCT_BEGIN(TitleMenuStateComponent)
REFLECT_FIELD(selectedIndex),
REFLECT_FIELD(itemCount),
REFLECT_FIELD(isConfirmAnimating),
REFLECT_FIELD(confirmIndex),
REFLECT_FIELD(confirmAnimTime),
REFLECT_FIELD(requestedAction),
REFLECT_FIELD(logoMotionTime)
REFLECT_STRUCT_END(TitleMenuStateComponent)

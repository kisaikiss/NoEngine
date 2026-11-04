#include "GameResult.h"
#include "engine/Functions/ECS/System/PlayModeStaticState.h"

namespace GameResult {
namespace {
Data sData;
REGISTER_PLAY_MODE_STATIC(sData);
}

void Set(const Data& data) { sData = data; }
const Data& Get() { return sData; }
}

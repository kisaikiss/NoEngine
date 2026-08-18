#include "GameResult.h"

namespace GameResult {
namespace {
Data sData;
}

void Set(const Data& data) { sData = data; }
const Data& Get() { return sData; }
}
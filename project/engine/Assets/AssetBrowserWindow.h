#pragma once
#include "engine/Functions/ECS/Registry.h"

namespace NoEngine{
namespace Editor {

void RefreshAssetBrowser();

void DrawAssetBrowserWindow(ECS::Registry& registry);
}
}
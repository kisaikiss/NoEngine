#pragma once
#include "engine/Functions/ECS/Registry.h"

namespace NoEngine{
namespace Editor {

void RefreshAssetBrowser();
void DrawAssetBrowserFolderWindow();
void DrawAssetBrowserWindow(ECS::Registry& registry);
}
}
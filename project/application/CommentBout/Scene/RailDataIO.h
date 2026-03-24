#pragma once
#include <string>

struct RailCameraComponent;

std::string MakeRailFilePath(const std::string& stageName);
std::string MakeEventFilePath(const std::string& stageName);

void ResetEventRuntime(RailCameraComponent& rail);

bool SaveRailToJson(const RailCameraComponent& rail, const std::string& stageName);
bool SaveEventsToJson(const RailCameraComponent& rail, const std::string& stageName);

bool LoadRailToComponent(RailCameraComponent& rail, const std::string& stageName);
bool LoadEventsToComponent(RailCameraComponent& rail, const std::string& stageName);

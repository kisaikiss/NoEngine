#include "stdafx.h"
#include "StateObjectManager.h"
namespace NoEngine {
void StateObjectManager::Register(const std::string& name, const StateObject& entry) {
	entries_[name] = entry;
}
StateObject& StateObjectManager::Get(const std::string& name) {
	if (!entries_.contains(name)) assert(false);
	return entries_[name];
}
}
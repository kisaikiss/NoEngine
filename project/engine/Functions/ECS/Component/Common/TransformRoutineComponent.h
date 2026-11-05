#pragma once
#include "engine/Math/MathInclude.h"
#include "TransformComponent.h"
namespace NoEngine {
struct TransformKeyframe : public Component::TransformComponent {
    float      duration = 1.0f;           // このwaypointに到達するまでの秒数
    Easing::EasingType easing = Easing::EasingType::Linear; // 直前waypoint→このwaypointの速度カーブ
};

namespace Component {
struct TransformRoutineDefinition {
    std::string id = "default";
    std::vector<TransformKeyframe> keyframes;
    // trueなら全区間に共通設定を使い、falseなら各waypointのduration/easingを使う
    bool useGlobalTiming = false;
    float globalDuration = 1.0f;
    Easing::EasingType globalEasing = Easing::EasingType::Linear;
    InterpolationType interpolation = InterpolationType::Linear;
    bool loop = true;
};

struct TransformRoutineComponent {
    std::vector<TransformRoutineDefinition> routines{ TransformRoutineDefinition{} };
    std::string defaultRoutineId = "default";
    bool playing = true;
    float playbackSpeed = 1.0f;

    // ランタイム状態（エディタでは基本触らない想定）
    int   currentIndex = 0;
    float elapsed = 0.0f;
    std::string activeRoutineId;
    // 編集対象は再生対象と分離し、保存しない。
    int selectedRoutineIndex = 0;

    const TransformRoutineDefinition* FindRoutine(const std::string& id) const {
        const TransformRoutineDefinition* found = nullptr;
        if (id.empty()) return nullptr;
        for (const auto& routine : routines) {
            if (routine.id != id) continue;
            // 重複IDを曖昧なまま再生しない。
            if (found) return nullptr;
            found = &routine;
        }
        return found;
    }

    const TransformRoutineDefinition* GetActiveRoutine() const {
        return FindRoutine(activeRoutineId.empty() ? defaultRoutineId : activeRoutineId);
    }

    TransformRoutineDefinition* GetEditorRoutine() {
        if (routines.empty()) return nullptr;
        selectedRoutineIndex = std::clamp(selectedRoutineIndex, 0, static_cast<int>(routines.size()) - 1);
        return &routines[selectedRoutineIndex];
    }

    bool StartRoutine(const std::string& id = {}) {
        const auto* routine = FindRoutine(id.empty() ? defaultRoutineId : id);
        if (!routine) return false;
        activeRoutineId = routine->id;
        currentIndex = 0;
        elapsed = 0.0f;
        playing = routine->keyframes.size() >= 2;
        return true;
    }
};
}

}

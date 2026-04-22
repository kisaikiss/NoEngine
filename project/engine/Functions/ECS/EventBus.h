#pragma once
#include "engine/Utilities/TypeIndex.h"

namespace NoEngine {
namespace ECS {
class EventBus {
public:
    template<typename Event>
    void Emit(const Event& e) {
        queues_[Utilities::TypeID<Event>()].push_back(std::make_any<Event>(e));
    }

    template<typename Event>
    std::optional<Event> Poll() {
        auto& q = queues_[Utilities::TypeID<Event>()];
        if (q.empty()) return std::nullopt;

        Event e = std::any_cast<Event>(q.front());
        q.pop_front();
        return e;
    }

    template<typename Event>
    std::vector<Event> PollAll() {
        auto& q = queues_[Utilities::TypeID<Event>()];
        std::vector<Event> result;
        result.reserve(q.size());

        while (!q.empty()) {
            result.push_back(std::any_cast<Event>(q.front()));
            q.pop_front();
        }
        return result;
    }


private:
    std::unordered_map<size_t, std::deque<std::any>> queues_;
};
}
}


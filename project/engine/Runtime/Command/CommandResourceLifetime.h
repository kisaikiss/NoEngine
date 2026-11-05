#pragma once

namespace NoEngine {
// 記録中の参照は送信後もフェンス完了まで保持する。同じキュー内で使用する。
class CommandResourceLifetime {
public:
    void KeepAlive(std::shared_ptr<void> resource) {
        if (resource) recording_.push_back(std::move(resource));
    }

    void OnSubmitted(uint64_t fence) {
        if (recording_.empty()) return;
        submitted_.push_back({fence, std::move(recording_)});
        recording_.clear();
    }

    template<class IsComplete>
    void ReleaseCompleted(IsComplete&& isComplete) {
        while (!submitted_.empty() && isComplete(submitted_.front().fence)) {
            submitted_.pop_front();
        }
    }

    uint64_t LastSubmissionFence() const {
        return submitted_.empty() ? 0 : submitted_.back().fence;
    }

private:
    struct Submission {
        uint64_t fence;
        std::vector<std::shared_ptr<void>> resources;
    };
    std::vector<std::shared_ptr<void>> recording_;
    std::deque<Submission> submitted_;
};
}

#pragma once

#ifdef USE_IMGUI

namespace NoEngine {
class GraphicsContext;

namespace Editor {
class FrameProfiler {
public:
	using Clock = std::chrono::steady_clock;

	FrameProfiler();
	void BeginFrame();
	void EndFrame();
	void Draw();
	void ReleaseGpuResources();

	static FrameProfiler* Active() { return active_; }
	static float MillisecondsSince(Clock::time_point start) {
		return std::chrono::duration<float, std::milli>(Clock::now() - start).count();
	}
	void RecordSystem(const std::string& name, float cpuMs) {
		current_.systems.push_back({ name, cpuMs });
	}
	static void RecordDrawCall() {
		if (active_) ++active_->current_.drawCalls;
	}

	void PrepareRenderPasses(size_t passCount);
	void BeginRenderPass(GraphicsContext& gfx, const std::string& name);
	void EndRenderPass(GraphicsContext& gfx);
	void ResolveRenderPasses(GraphicsContext& gfx);

private:
	struct SystemTiming {
		std::string name;
		float cpuMs = 0.f;
	};
	struct PassTiming {
		std::string name;
		float cpuMs = 0.f;
		std::optional<double> gpuMs;
		size_t drawCalls = 0;
	};
	struct FrameSample {
		float frameMs = 0.f;
		size_t drawCalls = 0;
		std::vector<SystemTiming> systems;
		std::vector<PassTiming> passes;
		bool gpuResolved = false;
		bool gpuRead = false;
	};

	void ReadGpuTimings();
	void SampleVideoMemory();

	static inline thread_local FrameProfiler* active_ = nullptr;
	bool recording_ = true;
	Clock::time_point frameStart_;
	Clock::time_point passStart_;
	FrameSample current_;
	FrameSample latest_;
	std::array<float, 120> frameHistory_{};
	int historyCount_ = 0;
	int historyNext_ = 0;

	UINT64 timestampFrequency_ = 0;
	UINT queryCapacity_ = 0;
	bool captureGpu_ = false;
	Microsoft::WRL::ComPtr<ID3D12QueryHeap> timestampHeap_;
	Microsoft::WRL::ComPtr<ID3D12Resource> timestampReadback_;
	Clock::time_point nextMemorySample_{};
	std::optional<DXGI_QUERY_VIDEO_MEMORY_INFO> videoMemory_;
};
}
}
#endif

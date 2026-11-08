#include "FrameProfiler.h"

#ifdef USE_IMGUI
#include "engine/Runtime/Command/GraphicsContext.h"
#include "engine/Runtime/GraphicsCore.h"
#include "externals/imgui/imgui.h"

namespace NoEngine::Editor {
namespace {
constexpr ImGuiTableFlags kTimingTableFlags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV |
	ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY;
constexpr double kBytesPerMiB = 1024.0 * 1024.0;
}

FrameProfiler::FrameProfiler() {
	auto* queue = GraphicsCore::sCommandListManager.GetQueue().GetCommandQueue();
	if (FAILED(queue->GetTimestampFrequency(&timestampFrequency_))) timestampFrequency_ = 0;
}

void FrameProfiler::BeginFrame() {
	if (!recording_) return;
	current_.drawCalls = 0;
	current_.systems.clear();
	current_.passes.clear();
	current_.gpuResolved = false;
	current_.gpuRead = false;
	captureGpu_ = false;
	frameStart_ = Clock::now();
	active_ = this;
}

void FrameProfiler::PrepareRenderPasses(size_t passCount) {
	if (!timestampFrequency_ || passCount == 0 || passCount > (std::numeric_limits<UINT>::max)() / 2) return;
	const UINT queryCount = static_cast<UINT>(passCount * 2);
	if (queryCount > queryCapacity_) {
		auto* device = GraphicsCore::sGraphicsDevice->GetDevice();
		const D3D12_QUERY_HEAP_DESC queryDesc{ D3D12_QUERY_HEAP_TYPE_TIMESTAMP, queryCount, 0 };
		const CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_READBACK);
		const auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(static_cast<UINT64>(queryCount) * sizeof(UINT64));
		Microsoft::WRL::ComPtr<ID3D12QueryHeap> heap;
		Microsoft::WRL::ComPtr<ID3D12Resource> readback;
		if (FAILED(device->CreateQueryHeap(&queryDesc, IID_PPV_ARGS(heap.GetAddressOf()))) ||
			FAILED(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc,
				D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(readback.GetAddressOf())))) {
			return;
		}
		// 前フレームのクエリは EndFrame の既存の待機で完了している。
		timestampHeap_ = std::move(heap);
		timestampReadback_ = std::move(readback);
		queryCapacity_ = queryCount;
	}
	captureGpu_ = true;
}

void FrameProfiler::BeginRenderPass(GraphicsContext& gfx, const std::string& name) {
	const auto index = current_.passes.size();
	current_.passes.push_back({ name, 0.f, std::nullopt, current_.drawCalls });
	passStart_ = Clock::now();
	if (captureGpu_) {
		gfx.GetCommandList()->EndQuery(timestampHeap_.Get(), D3D12_QUERY_TYPE_TIMESTAMP, static_cast<UINT>(index * 2));
	}
}

void FrameProfiler::EndRenderPass(GraphicsContext& gfx) {
	auto& pass = current_.passes.back();
	pass.cpuMs = MillisecondsSince(passStart_);
	pass.drawCalls = current_.drawCalls - pass.drawCalls;
	if (captureGpu_) {
		const UINT endIndex = static_cast<UINT>(current_.passes.size() * 2 - 1);
		gfx.GetCommandList()->EndQuery(timestampHeap_.Get(), D3D12_QUERY_TYPE_TIMESTAMP, endIndex);
	}
}

void FrameProfiler::ResolveRenderPasses(GraphicsContext& gfx) {
	if (!captureGpu_ || current_.passes.empty()) return;
	const UINT queryCount = static_cast<UINT>(current_.passes.size() * 2);
	gfx.GetCommandList()->ResolveQueryData(timestampHeap_.Get(), D3D12_QUERY_TYPE_TIMESTAMP,
		0, queryCount, timestampReadback_.Get(), 0);
	current_.gpuResolved = true;
}

void FrameProfiler::ReadGpuTimings() {
	if (!current_.gpuResolved) return;
	const D3D12_RANGE readRange{ 0, current_.passes.size() * 2 * sizeof(UINT64) };
	void* data = nullptr;
	if (FAILED(timestampReadback_->Map(0, &readRange, &data))) return;
	const auto* timestamps = static_cast<const UINT64*>(data);
	const double msPerTick = 1000.0 / static_cast<double>(timestampFrequency_);
	for (size_t i = 0; i < current_.passes.size(); ++i) {
		const UINT64 start = timestamps[i * 2];
		const UINT64 end = timestamps[i * 2 + 1];
		if (end >= start) current_.passes[i].gpuMs = static_cast<double>(end - start) * msPerTick;
	}
	const D3D12_RANGE noWrites{};
	timestampReadback_->Unmap(0, &noWrites);
	current_.gpuRead = true;
}

void FrameProfiler::SampleVideoMemory() {
	const auto now = Clock::now();
	if (now < nextMemorySample_) return;
	nextMemorySample_ = now + std::chrono::milliseconds(500);
	DXGI_QUERY_VIDEO_MEMORY_INFO info{};
	auto* adapter = GraphicsCore::sGraphicsInfrastructures->GetDXGIAdapter();
	if (SUCCEEDED(adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &info))) {
		videoMemory_ = info;
	} else {
		videoMemory_.reset();
	}
}

void FrameProfiler::EndFrame() {
	if (active_ != this) return;
	active_ = nullptr;
	if (!recording_) return;

	current_.frameMs = MillisecondsSince(frameStart_);
	// GraphicsCore::EndFrame の送信と待機が済んだ後に読み戻す。
	ReadGpuTimings();
	SampleVideoMemory();
	std::stable_sort(current_.systems.begin(), current_.systems.end(), [](const auto& left, const auto& right) {
		return left.cpuMs > right.cpuMs;
	});
	std::stable_sort(current_.passes.begin(), current_.passes.end(), [](const auto& left, const auto& right) {
		if (left.gpuMs && right.gpuMs) return *left.gpuMs > *right.gpuMs;
		if (left.gpuMs || right.gpuMs) return left.gpuMs.has_value();
		return left.cpuMs > right.cpuMs;
	});
	std::swap(latest_, current_);
	const auto capacity = static_cast<int>(frameHistory_.size());
	frameHistory_[historyNext_] = latest_.frameMs;
	historyNext_ = (historyNext_ + 1) % capacity;
	historyCount_ = std::min(historyCount_ + 1, capacity);
}

void FrameProfiler::ReleaseGpuResources() {
	timestampReadback_.Reset();
	timestampHeap_.Reset();
}

void FrameProfiler::Draw() {
	if (ImGui::Begin("Performance")) {
		if (ImGui::Button(recording_ ? "Freeze capture" : "Resume capture")) recording_ = !recording_;
		ImGui::SameLine();
		ImGui::TextUnformatted(recording_ ? "Latest completed frame" : "Frozen frame");
		if (historyCount_ == 0) {
			ImGui::TextUnformatted("Waiting for the first completed frame.");
			ImGui::End();
			return;
		}

		ImGui::Text("FPS: %.1f  |  Frame: %.2f ms", latest_.frameMs > 0.f ? 1000.f / latest_.frameMs : 0.f, latest_.frameMs);
		ImGui::Text("Engine draw calls: %zu", latest_.drawCalls);
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("Direct draws through GraphicsContext. Excludes ImGui and dispatches.");
		if (videoMemory_) {
			ImGui::Text("Local GPU memory: %.1f / %.1f MiB",
				static_cast<double>(videoMemory_->CurrentUsage) / kBytesPerMiB,
				static_cast<double>(videoMemory_->Budget) / kBytesPerMiB);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Process usage / DXGI budget, sampled twice per second.");
		} else {
			ImGui::TextUnformatted("Local GPU memory: unavailable");
		}

		if (ImGui::BeginTabBar("TimingBreakdown")) {
			if (ImGui::BeginTabItem("ECS Systems")) {
				if (latest_.systems.empty()) ImGui::TextUnformatted("No systems ran in this frame.");
				else if (ImGui::BeginTable("Systems", 2, kTimingTableFlags, ImVec2(0.f, -1.f))) {
					ImGui::TableSetupColumn("System");
					ImGui::TableSetupColumn("CPU (ms)", ImGuiTableColumnFlags_WidthFixed, 85.f);
					ImGui::TableHeadersRow();
					for (const auto& system : latest_.systems) {
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::TextUnformatted(system.name.c_str());
						ImGui::TableSetColumnIndex(1);
						ImGui::Text("%.3f", system.cpuMs);
					}
					ImGui::EndTable();
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Render Passes")) {
				if (latest_.passes.empty()) ImGui::TextUnformatted("No render passes ran in this frame.");
				else {
					if (!latest_.gpuRead) ImGui::TextUnformatted("GPU timestamps unavailable for this frame.");
					if (ImGui::BeginTable("Passes", 4, kTimingTableFlags, ImVec2(0.f, -1.f))) {
						ImGui::TableSetupColumn("Render pass");
						ImGui::TableSetupColumn("CPU (ms)", ImGuiTableColumnFlags_WidthFixed, 85.f);
						ImGui::TableSetupColumn("GPU (ms)", ImGuiTableColumnFlags_WidthFixed, 85.f);
						ImGui::TableSetupColumn("Draws", ImGuiTableColumnFlags_WidthFixed, 55.f);
						ImGui::TableHeadersRow();
						for (const auto& pass : latest_.passes) {
							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::TextUnformatted(pass.name.c_str());
							ImGui::TableSetColumnIndex(1);
							ImGui::Text("%.3f", pass.cpuMs);
							ImGui::TableSetColumnIndex(2);
							if (pass.gpuMs) ImGui::Text("%.3f", *pass.gpuMs);
							else ImGui::TextUnformatted("-");
							ImGui::TableSetColumnIndex(3);
							ImGui::Text("%zu", pass.drawCalls);
						}
						ImGui::EndTable();
					}
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Frame History")) {
				const int offset = historyCount_ == static_cast<int>(frameHistory_.size()) ? historyNext_ : 0;
				ImGui::PlotLines("##FrameTime", frameHistory_.data(), historyCount_, offset, "Frame time (ms)",
					0.f, std::numeric_limits<float>::max(), ImVec2(-1.f, ImGui::GetContentRegionAvail().y));
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::End();
}
}
#endif

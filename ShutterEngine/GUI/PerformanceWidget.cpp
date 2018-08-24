#include "Widgets.h"

void PerformanceWidget::Draw()
{
	ImGui::SetNextWindowSize(ImVec2(120, 70));
	ImGui::SetNextWindowPos(ImVec2(_Margin, _Margin));
	ImGui::Begin("Performance", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
	ImGui::Text("%u FPS", static_cast<unsigned int>(1000.f / _LastFrameTime));
	ImGui::Text("%.2f ms", _LastFrameTime);
	ImGui::PushItemWidth(-1);
	ImGui::PlotLines("", _Buffer.data(), _Buffer.size(), 0, nullptr, 16.0f, 60.0f);
	ImGui::End();
}

void PerformanceWidget::AddValue(const float frameTime)
{
	_LastFrameTime = frameTime;
	_Buffer[_BufferOffset] = frameTime;
	_BufferOffset = (_BufferOffset + 1) % _Buffer.size();
}

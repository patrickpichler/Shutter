#include "Widgets.h"
#include "Engine/Object.h"

void ControlsWidget::Draw()
{
	if (_SceneTree->_SelectedObject != nullptr) {
		ImGui::SetNextWindowSize(ImVec2(250, 100));
		ImGui::SetNextWindowPos(ImVec2(_Margin, 768 - 100 - _Margin));
		ImGui::Begin(std::string("Controls: " + _SceneTree->_SelectedObject->_Name).c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::DragFloat3("Position", &_SceneTree->_SelectedObject->_Position[0], 0.1f);
		ImGui::DragFloat3("Rotation", &_SceneTree->_SelectedObject->_Rotation[0], 0.1f);
		ImGui::DragFloat3("Scale", &_SceneTree->_SelectedObject->_Scale[0], 0.1f);
		ImGui::End();
	}
}
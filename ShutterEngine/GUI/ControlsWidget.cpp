#include "Widgets.h"
#include "Engine/Object.h"
#include "Engine/Light.h"

void ControlsWidget::Draw()
{
	if (_SceneTree->_SelectedObject != nullptr) {
		ImGui::SetNextWindowSize(ImVec2(250, 100));
		ImGui::SetNextWindowPos(ImVec2(_Margin, _ViewportHeight - 100 - _Margin));
		ImGui::Begin(std::string("Controls: " + _SceneTree->_SelectedObject->_Name).c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::DragFloat3("Position", &_SceneTree->_SelectedObject->_Position[0], 0.1f);
		
		if (Light* light = dynamic_cast<Light*>(_SceneTree->_SelectedObject)) {
			ImGui::ColorEdit3("color", &light->_Colour[0]);
			ImGui::DragFloat("Range", &range, 0.1f);
			ImGui::SameLine();
			if (ImGui::Button("Apply!")) {
				light->SetRange(range);
			}
		}

		if (Object* object = dynamic_cast<Object*>(_SceneTree->_SelectedObject)) {
			ImGui::DragFloat3("Rotation", &object->_Rotation[0], 0.1f);
			ImGui::DragFloat3("Scale", &object->_Scale[0], 0.1f);
		}
		ImGui::End();
	}
}
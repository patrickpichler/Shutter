#include "Widgets.h"
#include "Engine/Scene.h"
#include "Engine/Object.h"

void SceneTreeWidget::Draw()
{
	ImGui::SetNextWindowSize(ImVec2(200, 300));
	ImGui::SetNextWindowPos(ImVec2(_ViewportWidth - 200 - _Margin, _Margin));

	ImGui::Begin("Scene Tree", nullptr, ImGuiWindowFlags_NoResize);

	for (auto &material : _Scene->_Objects) {

		for (auto &object : material.second) {
			if (ImGui::Selectable(object._Name.c_str(), object._Name == _Selected)) {
				if (_SelectedObject == &object) {
					_Selected = "";
					_SelectedObject = nullptr;

				}
				else {
					_Selected = object._Name;
					_SelectedObject = &object;
				}
			}
		}
	}

	ImGui::End();
}
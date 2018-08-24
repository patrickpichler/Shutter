#pragma once
#include <array>
#include <string>
#include "imgui.h"

class Widget {
public:
	virtual void Draw() = 0;

	bool IsVisible = true;

protected:
	const float _Margin = 15.0f;
};

class PerformanceWidget : public Widget {
public:
	void Draw() override;

	void AddValue(const float frameTime);

	float _LastFrameTime = .0f;
	std::array<float, 40> _Buffer = { .0f };
	int _BufferOffset = 0;
};

class Scene;
class Object;

class SceneTreeWidget : public Widget {
public:
	void Draw() override;

	Scene *_Scene;

	std::string _Selected;
	Object *_SelectedObject = nullptr;
};

class ControlsWidget : public Widget {
public:
	void Draw() override;

	SceneTreeWidget *_SceneTree;
};
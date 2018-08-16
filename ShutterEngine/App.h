#pragma once
#include <string>
#include <memory>
#include <GLFW\glfw3.h>
#include "Engine/Camera.h"
#include "Engine/Scene.h"

#include "Renderer/Renderer.h"

class Application {
public:
	Application(const std::string &appName);

	void Init();
	void Run();
	void Clean();

	void TriggerShaderReload();

	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
	void DrawFrame();

	const uint16_t _Width = 1024;
	const uint16_t _Height = 768;

	std::string ApplicationName;

	GLFWwindow* Window;
	Renderer render;

	enum KEY_BINDINGS {
		UP = GLFW_KEY_W,
		LEFT = GLFW_KEY_A,
		DOWN = GLFW_KEY_S,
		RIGHT = GLFW_KEY_D,
		RELOAD = GLFW_KEY_R
	};

	Camera *_Camera;
	Scene _Scene;

	double horizontalAngle;
	double verticalAngle;

	bool shaderReaload;
};
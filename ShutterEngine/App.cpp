#include "App.h"
#include <iostream>

Application::Application(const std::string &appName):
	ApplicationName(appName)
{
	horizontalAngle = 3.14f;
	verticalAngle = 0.0f;
	shaderReaload = false;
}

void Application::Init()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	Window = glfwCreateWindow(_Width, _Height, ApplicationName.c_str() , nullptr, nullptr);
	//glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetWindowUserPointer(Window, this);
	glfwSetFramebufferSizeCallback(Window, ResizeCallback);
	//glfwSetMouseButtonCallback(Window, Application::MouseCallback);

	_Scene = Scene();
	render.Init(Window, _Width, _Height, &_Scene);
	glfwSetWindowTitle(Window, _Scene._Name.c_str());
	_Camera = &_Scene._Camera;
	glfwSetKeyCallback(Window, Application::KeyCallback);
}

void Application::Run()
{

	double prevMouseX = 0, prevMouseY =0;
	while (!glfwWindowShouldClose(Window)) {
		glfwPollEvents();

		double mouseX, mouseY;
		glfwGetCursorPos(Window, &mouseX, &mouseY);

		if (broadcastCursor) {
			_Camera->Update(
				mouseX - prevMouseX,
				mouseY - prevMouseY,
				Direction{
					glfwGetKey(Window, UP) == GLFW_PRESS,
					glfwGetKey(Window, DOWN) == GLFW_PRESS,
					glfwGetKey(Window, LEFT) == GLFW_PRESS,
					glfwGetKey(Window, RIGHT) == GLFW_PRESS
				}
			);
		}

		prevMouseX = mouseX;
		prevMouseY = mouseY;
		
		DrawFrame();

		if (shaderReaload) {
			render.WaitIdle();
			render.ReloadShaders();
			shaderReaload = false;
		}
	}
	render.WaitIdle();
}

void Application::Clean()
{
	render.Clean();

	glfwDestroyWindow(Window);
	glfwTerminate();
}

void Application::TriggerShaderReload()
{
	std::cout << "Reloading shaders" << std::endl;
	shaderReaload = true;
}

void Application::TriggerResize()
{
	render.Resize();
}

void Application::KeyCallback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	// Reload the shaders
	if (key == KEY_BINDINGS::RELOAD && action == GLFW_PRESS)
	{
		static_cast<Application*>(glfwGetWindowUserPointer(window))->TriggerShaderReload();
	}
	else if(key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		static_cast<Application*>(glfwGetWindowUserPointer(window))->broadcastCursor = !static_cast<Application*>(glfwGetWindowUserPointer(window))->broadcastCursor;
	}
}

void Application::MouseCallback(GLFWwindow * window, int button, int action, int mods)
{

}

void Application::ResizeCallback(GLFWwindow * window, int width, int height)
{
	static_cast<Application*>(glfwGetWindowUserPointer(window))->TriggerResize();
}

void Application::DrawFrame()
{
	render.Draw();
}

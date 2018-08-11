#include "App.h"
#include <iostream>
#define TINYOBJLOADER_IMPLEMENTATION
#include "Ext/tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Ext/stb_image.h"

int main() {
	Application app("Project0");

	try
	{
		app.Init();

		app.Run();

		app.Clean();
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}

	return 0;
}
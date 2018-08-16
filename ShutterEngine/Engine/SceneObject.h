#pragma once
#include <string>
#include <glm/glm.hpp>

class SceneObject {
public:
	SceneObject(){}
	SceneObject(const std::string &name, const glm::vec3 &position = glm::vec3(0.0f, 0.0f, 0.0f)):
		_Name(name),
		_Position(position)
	{}

	std::string _Name;
	glm::vec3 _Position;
};
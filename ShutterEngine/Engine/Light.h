#pragma once
#include "SceneObject.h"


struct LightUniformData {
	glm::vec4 _Position;
	glm::vec4 _Colour;
	glm::vec4 _Parameters;
};

class Light : public SceneObject {
public:
	Light() {}
	Light(const std::string &name, const glm::vec3 &position = glm::vec3(0.0f, 0.0f, 0.0f));

	void SetRange(const double range);

	LightUniformData GetUniformData();

	glm::vec3 _Colour;
private:
	// Parameters used to set the range of a point light
	double _Constant;
	double _Linear;
	double _Quadratic;
};
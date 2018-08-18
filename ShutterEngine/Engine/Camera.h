#pragma once
#include "SceneObject.h"
#include <glm/glm.hpp>


struct CameraUniformData {
	glm::mat4 _View;
	glm::mat4 _Projection;
	glm::vec4 _Position;
};

struct Direction {
	bool Up;
	bool Down;
	bool Left;
	bool Right;
};

class Camera : public SceneObject {
public:
	Camera() {}
	Camera(const std::string &name, float fov, const uint16_t width, const uint16_t height, const glm::vec3 &position, const glm::vec3 direction, const glm::vec3 up);

	void Update(const double mouseX, const double mouseY, const Direction &dir);

	CameraUniformData GetUniformData();

	glm::mat4 GetProjection() const;
	glm::mat4 GetView() const;

private:
	float _FOV;

	uint16_t _Width;
	uint16_t _Height;

	glm::vec3 _Front;
	glm::vec3 _Up;
	glm::vec3 _Right;

	float _TranslationSpeed;
	float _RotationSpeed;

	double _HorizontalAngle;
	double _VerticalAngle;
};
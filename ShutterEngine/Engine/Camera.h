#pragma once

#include <glm/glm.hpp>

struct Direction {
	bool Up;
	bool Down;
	bool Left;
	bool Right;
};

class Camera {
public:
	Camera() {}
	Camera(const float fov, const uint16_t width, const uint16_t height, const glm::vec3 &position, const glm::vec3 direction, const glm::vec3 up);

	void Update(const double mouseX, const double mouseY, const Direction &dir);

	glm::mat4 GetProjection() const;
	glm::mat4 GetView() const;

private:
	float _FOV;

	uint16_t _Width;
	uint16_t _Height;

	glm::vec3 _Position;

	glm::vec3 _Front;
	glm::vec3 _Up;
	glm::vec3 _Right;

	float _TranslationSpeed;
	float _RotationSpeed;

	double _HorizontalAngle;
	double _VerticalAngle;
};
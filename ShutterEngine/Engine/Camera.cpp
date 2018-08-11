#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(const float fov, const uint16_t width, const uint16_t height, const glm::vec3 & position, const glm::vec3 direction, const glm::vec3 up) :
	_FOV(fov),
	_Width(width),
	_Height(height),
	_Position(position),
	_Front(direction),
	_Up(up)
{
	_Right = glm::cross(_Front, _Up);

	_TranslationSpeed = 0.13f;
	_RotationSpeed = 0.01f;

	_HorizontalAngle = 0.f;
	_VerticalAngle = 0.f;
}

void Camera::Update(const double mouseX, const double mouseY, const Direction &dir)
{
	_HorizontalAngle += _RotationSpeed * mouseX;
	_VerticalAngle += _RotationSpeed * mouseY;

	glm::vec3 direction(
		cos(_VerticalAngle) * sin(_HorizontalAngle),
		cos(_VerticalAngle) * cos(_HorizontalAngle),
		sin(_VerticalAngle)
	);
	_Right = glm::vec3(
		sin(_HorizontalAngle - 3.14f / 2.0f),
		cos(_HorizontalAngle - 3.14f / 2.0f),
		0
	);
	_Up = glm::cross(_Right, direction);


	_Front = glm::normalize(direction);

	glm::vec3 movement;

	if (dir.Up) {
		movement = _Front * _TranslationSpeed;
	}
	else if (dir.Down) {
		movement = _Front * -_TranslationSpeed;
	}
	if (dir.Left) {
		movement += _Right * -_TranslationSpeed;
	}
	else if (dir.Right) {
		movement += _Right * _TranslationSpeed;
	}

	_Position += movement;
}

glm::mat4 Camera::GetProjection() const
{
	return glm::perspective(glm::radians(_FOV), float(_Width) / float(_Height), 0.1f, 100.0f);
}

glm::mat4 Camera::GetView() const
{
	return glm::lookAt(_Position, _Position + _Front, _Up);
	//return glm::lookAt(_Position, glm::vec3(0,0,0), glm::vec3(0,0,1));
}

#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(const std::string &name, const float fov, const uint16_t width, const uint16_t height, const glm::vec3 & position, const glm::vec3 direction, const glm::vec3 up) :
	SceneObject(name, position),
	_FOV(fov),
	_Width(width),
	_Height(height),
	_Forward(direction),
	_Up(up)
{
	_Right = glm::cross(_Forward, _Up);

	_TranslationSpeed = 0.13f;
	_RotationSpeed = 0.01f;

	_HorizontalAngle = 0.f;
	_VerticalAngle = 0.f;

	_Frustrum.GenerateFrustrum(*this);
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


	_Forward = glm::normalize(direction);

	glm::vec3 movement = { 0.0f, 0.0f, 0.0f };

	if (dir.Up) {
		movement = _Forward * _TranslationSpeed;
	}
	else if (dir.Down) {
		movement = _Forward * -_TranslationSpeed;
	}
	if (dir.Left) {
		movement += _Right * -_TranslationSpeed;
	}
	else if (dir.Right) {
		movement += _Right * _TranslationSpeed;
	}

	_Position += movement;

	_Frustrum.GenerateFrustrum(*this);
}

CameraUniformData Camera::GetUniformData()
{
	CameraUniformData data;
	data._Position = glm::vec4(_Position, 0.0);
	data._Projection = GetProjection();
	data._View = GetView();

	return data;
}

glm::mat4 Camera::GetProjection() const
{
	glm::mat4 proj = glm::perspective(glm::radians(_FOV), float(_Width) / float(_Height), _Near, _Far);
	proj[1][1] *= -1;
	return proj;
}

glm::mat4 Camera::GetView() const
{
	return glm::lookAt(_Position, _Position + _Forward, _Up);
}

#include "Light.h"

Light::Light(const std::string & name, const glm::vec3 & position) : 
	SceneObject(name, position),
	_Colour(glm::vec3(0.0, 0.0, 0.0)),
	_Constant(1.0),
	_Linear(0.22),
	_Quadratic(0.20),
	_Strength(1.0)
{
}

void Light::SetRange(const double range)
{
	// https://wiki.ogre3d.org/Light+Attenuation+Shortcut
	_Constant = 1.0;
	_Linear = 4.5 / range;
	_Quadratic = 75.0f / (range * range);
}

LightUniformData Light::GetUniformData()
{
	LightUniformData data;
	data._Position = glm::vec4(_Position, 0.0);
	data._Colour = glm::vec4(_Colour, 0.0);
	data._Parameters = glm::vec4(_Constant, _Linear, _Quadratic, _Strength);

	return data;
}

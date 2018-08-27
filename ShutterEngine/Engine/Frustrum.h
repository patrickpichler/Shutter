#pragma once
#include <array>
#include <glm/glm.hpp>
#include "Mesh.h"

typedef std::array<glm::vec3, 4> Plane;

class Camera;

class Frustrum {
public:
	Frustrum(){}

	void GenerateFrustrum(const Camera &camera);
	bool TestFrustrum(const BoundingBox bbox);

private:
	std::array<Plane, 6>  _FrustrumPlanes;

};
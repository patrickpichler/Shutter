#include "Frustrum.h"
#include "Camera.h"

void Frustrum::GenerateFrustrum(const Camera &camera)
{
	/*
		Far plane:		Near plane: 
		f1 ---- f2		n1 ---- n2
		|		|		|		|
		|		|		|		|
		f3 ---- f4		n3 ---- n4
	*/

	glm::vec3 nearCenter = camera._Position + (camera._Forward * camera._Near);
	glm::vec3 farCenter = camera._Position + (camera._Forward * camera._Far);

	float nearHalfWidth = std::atanf(camera._FOV / 2.0f) * camera._Near;
	glm::vec3 nearHalfWidthVector = nearHalfWidth * camera._Right;
	float farHalfWidth = std::atanf(camera._FOV / 2.0f) * camera._Far;
	glm::vec3 farHalfWidthVector = farHalfWidth * camera._Right;

	float ratio = static_cast<float>(camera._Height) / static_cast<float>(camera._Width);

	float nearHalfHeight = nearHalfWidth * ratio;
	glm::vec3 nearHalfHeightVector = nearHalfHeight * camera._Up;
	float farHalfHeight = farHalfWidth * ratio;
	glm::vec3 farHalfHeightVector = farHalfHeight * camera._Up;


	glm::vec3 f1, f2, f3, f4, n1, n2, n3, n4;

	n1 = nearCenter + nearHalfHeightVector - nearHalfWidthVector;
	n2 = nearCenter + nearHalfHeightVector + nearHalfWidthVector;
	n3 = nearCenter - nearHalfHeightVector - nearHalfWidthVector;
	n4 = nearCenter - nearHalfHeightVector + nearHalfWidthVector;

	f1 = farCenter + farHalfHeightVector - farHalfWidthVector;
	f2 = farCenter + farHalfHeightVector + farHalfWidthVector;
	f3 = farCenter - farHalfHeightVector - farHalfWidthVector;
	f4 = farCenter - farHalfHeightVector + farHalfWidthVector;

	Plane front{ n1, n2, n3, n4 };
	Plane left{ n1, f1, f3, n3 };
	Plane top{ f1, f2, n2, n1 };
	Plane right{ n2, f2, f4, n4 };
	Plane bottom{ f4, f3, n3, n4 };
	Plane back{ f2, f1, f4, f3 };

	_FrustrumPlanes = { front, front, top, top, bottom, back };
}

bool Frustrum::TestFrustrum(const BoundingBox bbox)
{
	uint8_t isIn = 0;


	for (uint8_t i = 0; i < 6; ++i) {
		bool isOut = false;
		glm::vec3 normal = glm::cross((_FrustrumPlanes[i][1] - _FrustrumPlanes[i][0]), (_FrustrumPlanes[i][2] - _FrustrumPlanes[i][0]));


		glm::vec3 toMax = bbox._Max - _FrustrumPlanes[i][0];
		glm::vec3 toMin = bbox._Min - _FrustrumPlanes[i][0];

		if (glm::dot(toMax, normal) > 0) {
			isIn++;
		}
		else {
			isOut = true;
		}

		if (glm::dot(toMin, normal) > 0) {
			isIn++;
		}
		else if (isOut) {
			return false;
		}
	}
	return isIn > 0;
}

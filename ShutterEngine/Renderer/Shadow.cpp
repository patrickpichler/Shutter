#include "Shadow.h"
#include "Engine/Mesh.h"
#include "Helpers.h"

Shadow::Shadow(Device * device, Scene *scene, const uint16_t width, const uint16_t height, const uint32_t poolSize) :
	Material(device, scene, width, height, poolSize)
{
	// Investigate impact
	PopulateInfo(width, height);
}
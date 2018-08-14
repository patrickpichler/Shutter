#include "Layers.h"
#include <algorithm>
#include <iostream>

// Debug callback for the validation layers
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char* layerPrefix,
	const char* msg,
	void* userData) {

	std::cerr << "validation layer: " << msg << std::endl;

	return VK_FALSE;
}

void Layer::Init(const LayerRequestInfo& info)
{
	_AvailableLayers = vk::enumerateInstanceLayerProperties();

	// Check the required, missing one will throw an exception
	for (const auto& layerName : info.RequiredLayers) {
		if (!CheckLayer(layerName)) {
			throw std::runtime_error("Required layer " + std::string(layerName) + " is unavailable.");
		}

		_EnabledLayers.push_back(layerName);
	}

	// Check the optional, missing one will add it to the disabled
	for (const auto& layerName : info.OptionalLayers) {
		if (!CheckLayer(layerName)) {
			_DisabledLayers.push_back(layerName);
		}

		_EnabledLayers.push_back(layerName);
	}
}

void Layer::Clean(const vk::Instance &instance)
{
	//instance.destroyDebugReportCallbackEXT(_Callback);
}

void Layer::AttachDebugCallback(const vk::Instance &instance)
{
	//instance.createDebugReportCallbackEXT(vk::DebugReportCallbackCreateInfoEXT(
	//	vk::DebugReportFlagBitsEXT::eWarning | vk::DebugReportFlagBitsEXT::eError,
	//	(PFN_vkDebugReportCallbackEXT)_Callback
	//));
}

bool Layer::CheckLayer(const tLayerName & layerName)
{
	auto it = std::find_if(
		_AvailableLayers.begin(),
		_AvailableLayers.end(),
		[&layerName](const vk::LayerProperties&  availableExtenion) {
			return std::strcmp(availableExtenion.layerName, layerName) == 0;
		}
	);
	return it != _AvailableLayers.end();
}

#include "Layers.h"
#include <vulkan/vulkan.h>
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
	uint32_t LayerCount = 0;
	vkEnumerateInstanceLayerProperties(&LayerCount, nullptr);
	AvailableLayers.resize(LayerCount);
	vkEnumerateInstanceLayerProperties(&LayerCount, AvailableLayers.data());

	// Check the required, missing one will throw an exception
	for (const auto& layerName : info.RequiredLayers) {
		if (!CheckLayer(layerName)) {
			throw std::runtime_error("Required layer " + std::string(layerName) + " is unavailable.");
		}

		EnabledLayers.push_back(layerName);
	}

	// Check the optional, missing one will add it to the disabled
	for (const auto& layerName : info.OptionalLayers) {
		if (!CheckLayer(layerName)) {
			DisabledLayers.push_back(layerName);
		}

		EnabledLayers.push_back(layerName);
	}
}

void Layer::Clean(const VkInstance & instance)
{
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr) {
		func(instance, Callback, nullptr);
	}
}

const tLayerNameList& Layer::GetEnabledLayers() const
{
	return EnabledLayers;
}

const tLayerNameList& Layer::GetDisabledLayers() const
{
	return DisabledLayers;
}

VkResult Layer::AttachDebugCallback(const VkInstance &instance)
{
	VkDebugReportCallbackCreateInfoEXT debugCallbackInfo = { VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT };
	debugCallbackInfo.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT;
	debugCallbackInfo.pfnCallback = debugCallback;

	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	return func(instance, &debugCallbackInfo, nullptr, &Callback);
}

bool Layer::CheckLayer(const tLayerName & layerName)
{
	auto it = std::find_if(
		AvailableLayers.begin(),
		AvailableLayers.end(),
		[&layerName](const VkLayerProperties&  availableExtenion) {
			return std::strcmp(availableExtenion.layerName, layerName) == 0;
		}
	);
	return it != AvailableLayers.end();
}

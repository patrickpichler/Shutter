#include "Extensions.h"
#include <vulkan/vulkan.h>
#include <algorithm>

void Extension::Init(const ExtensionRequestInfo& info)
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	AvailableExtensions.resize(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, AvailableExtensions.data());

	// Check the required, missing one will throw an exception
	for (const auto& extensionName : info.RequiredExtensions) {
		if (!CheckExtension(extensionName)) {
			throw std::runtime_error("Required extension " + std::string(extensionName) + " is unavailable.");
		}

		EnabledExtensions.push_back(extensionName);
	}

	// Check the optional, missing one will add it to the disabled
	for (const auto& extensionName : info.OptionalExtensions) {
		if (!CheckExtension(extensionName)) {
			DisabledExtensions.push_back(extensionName);
		}

		EnabledExtensions.push_back(extensionName);
	}
}

const tExtensionNameList& Extension::GetEnabledExtensions() const
{
	return EnabledExtensions;
}

const tExtensionNameList& Extension::GetDisabledExtensions() const
{
	return DisabledExtensions;
}

bool Extension::CheckExtension(const tExtensionName & extensionName)
{
	auto it = std::find_if(
		AvailableExtensions.begin(),
		AvailableExtensions.end(),
		[&extensionName](const VkExtensionProperties&  availableExtenion) {
			return std::strcmp(availableExtenion.extensionName, extensionName) == 0;
		}
	);
	return it != AvailableExtensions.end();
}

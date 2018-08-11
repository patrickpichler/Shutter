#pragma once
#include <vector>
#include <vulkan/vulkan.h>

typedef const char* tExtensionName;
typedef std::vector<tExtensionName> tExtensionNameList;

struct ExtensionRequestInfo {
	tExtensionNameList RequiredExtensions;
	tExtensionNameList OptionalExtensions;
};

class Extension {
public:
	Extension() {}

	void Init(const ExtensionRequestInfo& info);

	const tExtensionNameList& GetEnabledExtensions() const;
	const tExtensionNameList& GetDisabledExtensions() const;

private:
	bool CheckExtension(const tExtensionName& extensionName);
private:
	std::vector<VkExtensionProperties> AvailableExtensions;
	tExtensionNameList EnabledExtensions;
	tExtensionNameList DisabledExtensions;
};
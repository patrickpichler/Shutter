#pragma once
#include <vector>
#include <vulkan/vulkan.h>

typedef const char* tLayerName;
typedef std::vector<tLayerName> tLayerNameList;

struct LayerRequestInfo {
	tLayerNameList RequiredLayers;
	tLayerNameList OptionalLayers;
};

class Layer {
public:
	Layer() {}

	void Init(const LayerRequestInfo& info);
	void Clean(const VkInstance &instance);

	const tLayerNameList& GetEnabledLayers() const;
	const tLayerNameList& GetDisabledLayers() const;

	VkResult AttachDebugCallback(const VkInstance &instance);

private:
	bool CheckLayer(const tLayerName& layerName);
private:
	std::vector<VkLayerProperties> AvailableLayers;
	tLayerNameList EnabledLayers;
	tLayerNameList DisabledLayers;

	VkDebugReportCallbackEXT Callback;
};
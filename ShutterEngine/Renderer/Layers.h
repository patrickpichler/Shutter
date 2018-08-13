#pragma once
#include <vector>
#include <vulkan/vulkan.hpp>

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
	void Clean(const vk::Instance &instance);

	const tLayerNameList& GetEnabledLayers() const {
		return _EnabledLayers;
	}
	const tLayerNameList& GetDisabledLayers() const {
		return _DisabledLayers;
	}

	void AttachDebugCallback(const vk::Instance &instance);

private:
	bool CheckLayer(const tLayerName& layerName);
private:
	std::vector<vk::LayerProperties> _AvailableLayers;
	tLayerNameList _EnabledLayers;
	tLayerNameList _DisabledLayers;

	VkDebugReportCallbackEXT _Callback;
};
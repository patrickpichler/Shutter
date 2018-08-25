#include "Scene.h"
#include "yaml-cpp/yaml.h"

namespace YAML {
	template<>
	struct convert<glm::vec3> {
		static Node encode(const glm::vec3& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs) {
			if (!node.IsSequence() || node.size() != 3) {
				return false;
			}

			rhs.x = node[0].as<double>();
			rhs.y = node[1].as<double>();
			rhs.z = node[2].as<double>();
			return true;
		}
	};
}

void Scene::Load(const std::string &name, Device *device, const vk::CommandPool &cmdPool, const vk::RenderPass &renderPass, const vk::RenderPass &shadowPass, const Texture &shadow) {
	CreateDynamic(device);
	std::string root = "Data/" + name + "/";

	YAML::Node config = YAML::LoadFile(root + "info.yaml");

	// Set the scene name
	_Name = config["name"].as<std::string>();
	
	// Load the lights
	_Lights.resize(config["lights"].size());
	for (int i = 0; i < _Lights.size() ; ++i) {
		std::string name = config["lights"][i]["name"].as<std::string>();
		glm::vec3 position = config["lights"][i]["position"].as<glm::vec3>();

		_Lights[i] = Light(name, position);
		_Lights[i]._Colour = config["lights"][i]["colour"].as<glm::vec3>();
		_Lights[i].SetRange(config["lights"][i]["range"].as<double>());
	}

	// Load the camera
	{
		std::string name = config["cameras"][0]["name"].as<std::string>();
		glm::vec3 position = config["cameras"][0]["position"].as<glm::vec3>();
		float fov = config["cameras"][0]["fov"].as<float>();

		_Camera = Camera(name, fov, 1024, 768, position, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
	}

	// Create the camera for the light source
	_ShadowCamera = Camera(name, 45, 1024, 768, glm::vec3(10, 0, 35), glm::vec3(-10, 0, -35), glm::vec3(0.0, 0.0, 1.0));

	// Load the materials
	for (int i = 0; i < config["materials"].size(); ++i) {
		// Load the shaders
		std::string vertex = config["materials"][i]["shaders"]["vertex"].as<std::string>();
		Shader vert(device, vertex, root + "shaders/" + vertex, vk::ShaderStageFlagBits::eVertex);

		std::string fragment = config["materials"][i]["shaders"]["fragment"].as<std::string>();
		Shader frag(device, fragment, root + "shaders/" + fragment, vk::ShaderStageFlagBits::eFragment);

		std::string name = config["materials"][i]["name"].as<std::string>();

		// Find the type
		std::string pipeline = config["materials"][i]["pipeline"].as<std::string>();
		if (pipeline == "basic") {
			// Create the material
			Material *mat = new Material(device, this, 1024, 768, 1024);
			mat->BindShader(vert);
			mat->BindShader(frag);
			mat->CreatePipeline(renderPass);
			_Materials.insert(std::pair<std::string, Material*>(name, mat));
		}
		else if (pipeline == "cubemap") {
			// Create the material
			Cubemap *mat = new Cubemap(device, this, 1024, 768, 1024);
			mat->BindShader(vert);
			mat->BindShader(frag);
			mat->CreatePipeline(renderPass);
			_Materials.insert(std::pair<std::string, Material*>(name, mat));
		}
		else if (pipeline == "shadow") {
			// Create the material
			Shadow *mat = new Shadow(device, this, 1024, 768, 1024);
			mat->BindShader(vert);
			mat->BindShader(frag);
			mat->CreatePipeline(shadowPass);
			_Materials.insert(std::pair<std::string, Material*>(name, mat));
		}
		_Objects.insert(std::pair<std::string, std::vector<Object>>(name, std::vector<Object>()));
	}

	// Load the models
	//_Models.resize(config["models"].size());
	for (int i = 0; i < config["models"].size(); ++i) {
		std::string filename = config["models"][i]["filename"].as<std::string>();
		std::unordered_map<std::string, Mesh> temp = Mesh::Load(device, root + "models/" + filename, root);
		_Models.insert(temp.begin(), temp.end());
	}

	// Load the textures
	for (int i = 0; i < config["textures"].size(); ++i) {
		std::string filename = config["textures"][i]["name"].as<std::string>();
		std::string type = config["textures"][i]["type"].as<std::string>();
		bool mipmap = config["textures"][i]["mipmap"].as<bool>();

		if (type == "normal") {
			Texture temp(device);
			temp.Load(root + "textures/" + filename, mipmap);
			temp.TransferBufferToImage(cmdPool);

			_Textures.insert(std::pair<std::string, Texture>(filename, temp));
		}
		else if (type == "cube") {
			CubeTexture temp(device);
			temp.Load({
				root + "textures/" + filename + "/posx.jpg",
				root + "textures/" + filename + "/negx.jpg",
				root + "textures/" + filename + "/posy.jpg",
				root + "textures/" + filename + "/negy.jpg",
				root + "textures/" + filename + "/posz.jpg",
				root + "textures/" + filename + "/negz.jpg"
			});
			temp.TransferBufferToImage(cmdPool);

			_Textures.insert(std::pair<std::string, Texture>(filename, temp));
		}
	}


	YAML::Node scene = YAML::LoadFile(root + "scene.yaml");

	// Load the objects
	//_Objects.resize(scene["scene"].size());
	for (int i = 0; i < scene["scene"].size(); ++i) {
		std::string model = scene["scene"][i]["model"].as<std::string>();
		std::string name = scene["scene"][i]["name"].as<std::string>();

		std::string material = scene["scene"][i]["material"].as<std::string>();
		Material *materialM = _Materials.at(material);

		glm::vec3 position = scene["scene"][i]["position"].as<glm::vec3>();
		glm::vec3 rotation = scene["scene"][i]["rotation"].as<glm::vec3>();
		glm::vec3 scale = scene["scene"][i]["scale"].as<glm::vec3>();

		_Objects[material].push_back(Object(device, _Models.at(model), materialM, 2));
		_Objects[material].back()._Position = position;
		_Objects[material].back()._Rotation = rotation;
		_Objects[material].back()._Scale = scale;

		for (int j = 0; j < scene["scene"][i]["textures"].size(); ++j) {
			if (scene["scene"][i]["textures"]) {
				int slot = scene["scene"][i]["textures"][j]["slot"].as<int>();
				std::string name = scene["scene"][i]["textures"][j]["texture"].as<std::string>();
				Texture texture = _Textures.at(name);
				_Objects[material].back().AddTexture(slot, texture);
			}
		}

		if (material == "basic" || material == "bump" || material == "transparent") {
			_Objects[material].back().AddTexture(2, shadow);
		}

		_Objects[material].back()._DynamicIndex = AddToDynamic(_Objects[material].back());
		_Objects[material].back().CreateDescriptorSet();
		_Objects[material].back()._Name = name;
	}

	UploadDynamic();
}

void Scene::Resize(const vk::RenderPass &renderPass, const vk::RenderPass &renderShadow, const vk::Extent2D & dimension)
{
	for (auto &material : _Materials) {
		if (material.first == "shadow") {

			material.second->ReloadPipeline(renderShadow, dimension.width, dimension.height);
		}
		else {
			material.second->ReloadPipeline(renderPass, dimension.width, dimension.height);
		}
	}

	_Camera._Width = dimension.width;
	_Camera._Height = dimension.height;
}

void Scene::CreateDynamic(Device *device)
{
	uint32_t minAlignement = device->GetProperties().limits.minUniformBufferOffsetAlignment;
	Object::dynamicAlignement = sizeof(glm::mat4);

	if (minAlignement > 0) {
		Object::dynamicAlignement = (Object::dynamicAlignement + minAlignement - 1) & ~(minAlignement - 1);
	}

	uint32_t bufferSize = 1024 * Object::dynamicAlignement;
	Object::uboDynamic.model = (glm::mat4*)_aligned_malloc(bufferSize, Object::dynamicAlignement);

	Object::DynamicBuffer = Buffer(device, vk::BufferUsageFlagBits::eUniformBuffer, bufferSize);
}

uint32_t Scene::AddToDynamic(const Object & object)
{
	uint32_t oldIndex = dynamicIndex;
	glm::mat4* modelPtr = (glm::mat4*)(((uint64_t)Object::uboDynamic.model + (oldIndex * Object::dynamicAlignement)));
	*modelPtr = object.GetModelMatrix();

	dynamicIndex++;
	return oldIndex;
}

void Scene::UploadDynamic()
{
	uint32_t bufferSize = 1024 * Object::dynamicAlignement;
	Object::DynamicBuffer.Copy(Object::uboDynamic.model, bufferSize);
}

void Scene::ReloadShader(const vk::RenderPass &renderPass, const vk::Extent2D &screenSize)
{
	// Reload the basic Material
	std::vector<Shader> shaders =  _Materials.at("basic")->GetShaderList();
	_Materials.at("basic")->ClearShaders();

	for (auto &shader : shaders) {
		Shader newShader(_Device, shader._Name, shader._Filename, shader._Stage, shader._EntryPoint);
		shader.Clean();
		_Materials.at("basic")->BindShader(newShader);
	}
	_Materials.at("basic")->ReloadPipeline(renderPass, screenSize.width, screenSize.height);

	shaders = _Materials.at("bump")->GetShaderList();
	_Materials.at("bump")->ClearShaders();

	for (auto &shader : shaders) {
		Shader newShader(_Device, shader._Name, shader._Filename, shader._Stage, shader._EntryPoint);
		shader.Clean();
		_Materials.at("bump")->BindShader(newShader);
	}
	_Materials.at("bump")->ReloadPipeline(renderPass, screenSize.width, screenSize.height);
}

void Scene::CreateDescriptorSets(Device *device, const uint32_t nbImages)
{
	_Device = device;

	CreateDescriptorSetLayout(nbImages);

	std::vector<vk::DescriptorSetLayout> layouts(nbImages, _DescriptorSetLayout);

	_SceneDescriptorSets = _Device->GetDevice().allocateDescriptorSets(vk::DescriptorSetAllocateInfo(
		_DescriptorPool,
		nbImages,
		layouts.data()
	));

	_SceneDescriptorSets.resize(nbImages);
	_SceneDataBuffers.resize(nbImages);
	_SceneDataObjects.resize(nbImages);

	for (size_t i = 0; i < nbImages; ++i)
	{
		_SceneDataBuffers.at(i) = Buffer(_Device, vk::BufferUsageFlagBits::eUniformBuffer, sizeof(SceneDataObject::Data) * 3);

		vk::WriteDescriptorSet cameraDescriptor(
			_SceneDescriptorSets.at(i),
			0, 0, 1,
			vk::DescriptorType::eUniformBuffer,
			nullptr,
			&vk::DescriptorBufferInfo(
				_SceneDataBuffers[i].GetBuffer(),
				0,
				sizeof(CameraUniformData)
			),
			nullptr
		);

		vk::WriteDescriptorSet shadowCameraDescriptor(
			_SceneDescriptorSets.at(i),
			1, 0, 1,
			vk::DescriptorType::eUniformBuffer,
			nullptr,
			&vk::DescriptorBufferInfo(
				_SceneDataBuffers[i].GetBuffer(),
				alignof(SceneDataObject::Data),
				sizeof(CameraUniformData)
			),
			nullptr
		);

		vk::DescriptorBufferInfo lightInfo = {
			_SceneDataBuffers.at(i).GetBuffer(),
			alignof(SceneDataObject::Data) * 2,
			sizeof(LightUniformData)
		};

		vk::WriteDescriptorSet lightDescriptor(
			_SceneDescriptorSets.at(i),
			2, 0, 1,
			vk::DescriptorType::eUniformBuffer,
			nullptr,
			&lightInfo
		);

		_Device->GetDevice().updateDescriptorSets({ cameraDescriptor, shadowCameraDescriptor, lightDescriptor }, nullptr);
	}
}

void Scene::Update(const uint32_t image)
{
	// Prepare the camera
	_SceneDataObjects.at(image)._Data[0]._CameraData = _Camera.GetUniformData();
	_SceneDataObjects.at(image)._Data[1]._CameraData = _ShadowCamera.GetUniformData();

	_SceneDataObjects.at(image)._Data[2]._LightData[0] = _Lights[0].GetUniformData();
	_SceneDataObjects.at(image)._Data[2]._LightData[1] = _Lights[1].GetUniformData();

	_SceneDataBuffers.at(image).Copy(&_SceneDataObjects.at(image)._Data, sizeof(SceneDataObject::Data) * 3);

	for (auto &mat : _Objects) {
		for (auto &obj : mat.second) {
			glm::mat4* modelPtr = (glm::mat4*)(((uint64_t)Object::uboDynamic.model + (obj._DynamicIndex * Object::dynamicAlignement)));
			*modelPtr = obj.GetModelMatrix();
		}
	}

	UploadDynamic();
}

void Scene::CreateDescriptorSetLayout(const uint32_t nbImages)
{
	vk::DescriptorSetLayoutBinding cameraInfo(0, vk::DescriptorType::eUniformBuffer, 1,  vk::ShaderStageFlagBits::eVertex);
	vk::DescriptorSetLayoutBinding shadowCameraInfo(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex);
	vk::DescriptorSetLayoutBinding lightInfo(2, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment);

	std::vector<vk::DescriptorSetLayoutBinding> bindings{ cameraInfo, shadowCameraInfo, lightInfo };


	_DescriptorSetLayout = _Device->GetDevice().createDescriptorSetLayout(
		vk::DescriptorSetLayoutCreateInfo(
			{},
			bindings.size(),
			bindings.data()
		)
	);

	std::vector<vk::DescriptorPoolSize> poolSizes;
	poolSizes.resize(bindings.size());

	for (size_t i = 0; i < bindings.size(); ++i) {
		poolSizes[i].type = bindings[i].descriptorType;
		poolSizes[i].descriptorCount = nbImages;
	}

	_DescriptorPool = _Device->GetDevice().createDescriptorPool(vk::DescriptorPoolCreateInfo({}, nbImages, poolSizes.size(), poolSizes.data()));
}
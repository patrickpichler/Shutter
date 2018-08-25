#include <string>
#include <iostream>
#include <assert.h>
   #define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <fstream>

int main(int argc, char **argv) {
	// Generate a scene file from the given obj
	assert(argc > 1);

	std::cout << "Generating scene file for: " << argv[1] << std::endl;

	std::ofstream file("scene.yaml");

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, argv[1], std::string(std::string(argv[2]) + "\\").c_str());

	for (size_t i = 0; i < shapes.size(); ++i) {

		file << "  - name: " << shapes[i].name << "\n";
		file << "    model: " << shapes[i].name << "\n";
		file << "    position: [0.0,0.0,0.0]\n";
		file << "    rotation: [90.0,90.0,0.0]\n";
		file << "    scale: [0.01,0.01,0.01]\n";
		if (materials.at(shapes.at(i).mesh.material_ids.front()).ambient_texname != "") {
			bool isUsingBump = false;
			bool isUsingAlpha = false;
			file << "    textures:\n";

			// Add the diffuse texture if there are some provided
			if (materials.at(shapes.at(i).mesh.material_ids.front()).ambient_texname != "") {
				file << "      - slot: 1\n";
				std::string ambient = materials.at(shapes.at(i).mesh.material_ids.front()).ambient_texname;
				file << "        texture: " << ambient.erase(0, 9) << "\n";
			}

			if (materials.at(shapes.at(i).mesh.material_ids.front()).alpha_texname != "") {
				file << "      - slot: 3\n";
				isUsingAlpha = true;
				std::string alpha = materials.at(shapes.at(i).mesh.material_ids.front()).alpha_texname;
				file << "        texture: " << alpha.erase(0, 9) << "\n";
			}

			if (materials.at(shapes.at(i).mesh.material_ids.front()).bump_texname != "") {
				isUsingBump = true;
				file << "      - slot: 4\n";
				std::string bump = materials.at(shapes.at(i).mesh.material_ids.front()).bump_texname;
				file << "        texture: " << bump.erase(0, 9) << "\n";
			}

			if (isUsingAlpha) {
				file << "    material: transparent\n";
			}
			else if (isUsingBump) {
				file << "    material: bump\n";
			}
			else {
				file << "    material: basic\n";
			}
		}
		else {
			file << "    material: flat\n";
		}

	}

	file.close();

	system("PAUSE");
}
#pragma once

#include <unordered_map>
#include <string.h>

#include "Model.h"
#include "TextureHDR.h"

struct HDRTextureSet {

	// For the skydome
	TextureHDR* m_SkyTexture;

	// Associated irradiance map for image-based lighting
	TextureHDR* m_IrradianceMap;

	HDRTextureSet(TextureHDR* sky, TextureHDR* irrMap)
		: m_SkyTexture(sky), m_IrradianceMap(irrMap) {}
};

class ResourceManager {

public:
	ResourceManager() {
		std::ifstream input;
		input.open("../res/ModelList.txt");

		if (!input.is_open()) {
			std::cout << "ERROR: Failed to read file" << '\n';
		}

		while (input) {
			// Get name of scene and add to the list of scene names
			std::string modelFile; // abc.blend, abc.obj etc.
			std::getline(input, modelFile, '\n');
			if (modelFile == "") break;

			AddModel(modelFile);
		}

		std::cout << "Loaded all models";

		// TODO: Later - deserialize from a text file
		AddHDRTextureSet("Newport_Loft_Ref");
	};

	// Add an HDR image with a desired name - if already present - Don't add
	void AddHDRTextureSet(std::string name) {
		if (m_HDRTextureSets.find(name) == m_HDRTextureSets.end()) {
			std::string pathPrefix = "../res/IBL/" + name;
			m_HDRTextureSets[name] = new HDRTextureSet(
				new TextureHDR(pathPrefix + ".hdr"),
				new TextureHDR(pathPrefix + "_Irr.hdr")
			);
		}
	}

	// Add a model with a desired name - if already present - Don't add
	void AddModel(std::string fileName) {
		std::string modelName = fileName.substr(0, fileName.find_first_of('.'));
		if (m_Models.find(modelName) == m_Models.end()) {
			m_ModelNames.push_back(modelName);
			std::string path = "../res/" + modelName + '/' + fileName;
			// abc ---> ../res/abc/abc.obj or whatever
			m_Models[modelName] = new Model(path);
		}
	}

	// Get the model of the given name
	Model* GetModel(std::string name) {
		if (m_Models.find(name) != m_Models.end()) {
			return m_Models[name];
		}
		else return nullptr;
	}

	HDRTextureSet* GetHDRTextureSet(std::string name) {
		if (m_HDRTextureSets.find(name) != m_HDRTextureSets.end()) {
			return m_HDRTextureSets[name];
		}
		else return nullptr;
	}

	std::vector<std::string> m_ModelNames;
private:
	std::unordered_map<std::string, Model*> m_Models;
	std::unordered_map<std::string, HDRTextureSet*> m_HDRTextureSets;
};

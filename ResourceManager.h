#pragma once

#include <unordered_map>

#include "Model.h"

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
			std::string modelName;
			std::getline(input, modelName, '\n');
			if (modelName == "") break;

			m_ModelNames.push_back(modelName);
			AddModel(modelName, "../res/" + modelName + ".obj");
		}
	};

	// Add a model with a desired name - if already present - Don't add
	void AddModel(std::string name, std::string path) {
		if (m_Models.find(name) == m_Models.end()) {
			m_Models[name] = new Model(path);
		}
	}

	// Get the model of the given name
	Model* GetModel(std::string name) {
		if (m_Models.find(name) != m_Models.end()) {
			return m_Models[name];
		}
		else return nullptr;
	}

	std::vector<std::string> m_ModelNames;
	// All the models
	//TODO: Preferably make this DS private and only give access to names of models;
private:
	std::unordered_map<std::string, Model*> m_Models;
	//std::string m_Model
};

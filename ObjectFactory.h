#pragma once

#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

#include "ObjectManager.h"
#include "ResourceManager.h"
#include "ComponentCreators.h"


class ObjectFactory {

public:
	ObjectFactory() {
		m_ComponentCreators["Transform"] = new TransformCreator();
		m_ComponentCreators["Model"] = new ModelCompCreator();
		m_ComponentCreators["Collider"] = new ColliderCreator();
		m_ComponentCreators["Material"] = new MaterialCreator();
	}

	void CreateSceneObjects(std::string path, 
		ObjectManager* p_ObjManager, ResourceManager* p_ResourceManager) {
		std::ifstream sceneFile(path);
		nlohmann::json allData = nlohmann::json::parse(sceneFile);
		auto& data = allData["Objects"];

		for (auto it = data.begin(); it != data.end(); it++) {
			
			// Get object JSON data and create the new object
			auto& objData = *it;
			Object* obj = new Object(it.key());

			for (auto itComp = objData.begin(); itComp != objData.end(); itComp++) {
				// Create each comp, deserialize, and add to obj
				std::string compCreatorType = (itComp).key();
				Component* comp = m_ComponentCreators[compCreatorType]->Create();
				comp->Deserialize(*itComp, p_ResourceManager);
				obj->AddComponent(comp);
			}

			// Add the object after adding all components
			p_ObjManager->AddObject(obj);
		}

		// Initialize all objects and their components.
		p_ObjManager->Initialize();
	}

	// TODO: Allow functionality to add and remove components from the editor
	//void GenerateComponentForObject(std::string componentName, Object* object) {
	//	object->AddComponent(m_ComponentCreators[componentName]->Create());
	//}

	void CreateObject(ObjectManager* p_ObjectManager, ResourceManager* p_ResourceManager) {
		// Create a new object
		Object* object = new Object("New_object" + std::to_string(obj_num));

		// Create all components and add in the beginning
		for (const auto& [compName, compCreator] : m_ComponentCreators) {
			Component* comp = compCreator->Create();
			// Set default values in components
			comp->SetDefaults(p_ResourceManager);
			object->AddComponent(comp);

		}
		object->Initialize();
		p_ObjectManager->AddObject(object);
		//p_ObjectManager->Initialize();
	}

private:
	int obj_num = 0;
	std::unordered_map<std::string,
		ComponentCreator*> m_ComponentCreators;
};


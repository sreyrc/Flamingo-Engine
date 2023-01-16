#pragma once

#include "ObjectFactory.h"
#include "CollisionWorld.h"

class SceneManager
{
public:
	SceneManager(ObjectManager* p_ObjectManager,
		ObjectFactory* p_ObjectFactory, ResourceManager* p_ResourceManager,
		CollisionWorld* p_CollisionWorld, Renderer* p_Renderer) {

		ReadSceneList();

		// First scene is the default scene
		m_SelectedScene = m_SceneNames[0];

		// Load first scene - default
		LoadScene(m_SceneNames[0], p_ObjectManager,
			p_ObjectFactory, p_ResourceManager, p_CollisionWorld, p_Renderer);
	}

	void ReadSceneList() {

		std::ifstream input;
		input.open("../scenes/SceneList.txt");

		if (!input.is_open()) {
			std::cout << "ERROR: Failed to read file" << '\n';
		}

		while (input) {
			// Get name of scene and add to the list of scene names
			std::string sceneName;
			std::getline(input, sceneName, '\n');

			if (std::find(m_SceneNames.begin(), m_SceneNames.end(),
				sceneName) == m_SceneNames.end()) {
				m_SceneNames.push_back(sceneName);
			}
		}

		// Last element is "". Remove that
		m_SceneNames.pop_back();
	}

	void LoadScene(std::string& sceneName, ObjectManager* p_ObjectManager,
		ObjectFactory* p_ObjectFactory, ResourceManager* p_ResourceManager,
		CollisionWorld* p_CollisionWorld, Renderer* p_Renderer) {

		// Destroy all objects in the scene - this runs near the end of 
		// the update loop so it's safe
		p_ObjectManager->DeleteAllObjects();

		// Load the scene using object factory
		p_ObjectFactory->CreateSceneObjects("../scenes/" + sceneName + ".json",
			p_ObjectManager, p_ResourceManager);

		// Re-add colliders now that a new scene is loaded in
		p_CollisionWorld->Configure(p_ObjectManager->m_ObjectList);

		// Configure scene lights
		p_Renderer->Deserialize("../scenes/" + sceneName + ".json");
	}

	void SaveScene(std::string newSceneName,
		ObjectManager* p_GameObjectManager, Renderer* p_Renderer) {
		// Save the json
		nlohmann::json sceneData;
		sceneData["Objects"] = p_GameObjectManager->Serialize();
		sceneData["Lights"] = p_Renderer->Serialize();
		//std::string newSceneName = "Scene" + std::to_string(++m_SceneNum);
		std::ofstream savedScene("../scenes/" + newSceneName + ".json");
		savedScene << std::setw(4) << sceneData;

		// Write this new scene name to the file only if not already present
		if (std::find(m_SceneNames.begin(), m_SceneNames.end(),
			newSceneName) == m_SceneNames.end()) {

			std::ofstream sceneList;
			sceneList.open("../scenes/SceneList.txt", std::ios::app);
			sceneList << '\n' + newSceneName;
			sceneList.close();
		}

		// Read the list again
		ReadSceneList();
	}

	//unsigned int m_SceneNum = 0;
	char m_SceneNameBuf[255];
	std::string m_SceneName = "";

	std::string m_SelectedScene;

	// List of sceneNames - to be used in the editor
	std::vector<std::string> m_SceneNames;
};


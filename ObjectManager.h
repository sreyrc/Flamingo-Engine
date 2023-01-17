#pragma once

#include "Object.h"

class ObjectManager {

public:
	ObjectManager() {};

	void AddObject(Object* obj) {
		m_ObjectList.push_back(obj);
	}

	void Initialize() {
		for (auto& obj : m_ObjectList) {
			obj->Initialize();
		}
	}

	void DestroyDeletedObjects() {
		for (int i = 0; i < m_ObjectList.size(); i++) {
			if (m_ObjectList[i]->isDeleted()) {
				delete m_ObjectList[i];
			}
		}
	}

	void DeleteObject(std::string name) {
		for (auto& obj : m_ObjectList) {
			if (obj->GetName() == name) {
				obj->MarkAsDeleted(true);
			}
		}
	}

	void DeleteAllObjects() {
		for (auto it = m_ObjectList.begin(); it != m_ObjectList.end(); it++) {
			delete *it;
		}
		m_ObjectList.clear();
	}

	void Update() {
		for (auto& obj : m_ObjectList) {
			obj->Update();
		}
	}

	~ObjectManager() {
		/*for (auto obj : m_ObjectList) {
			delete obj;
		}*/
		DeleteAllObjects();
	}

	nlohmann::json::value_type Serialize() {
		nlohmann::json sceneData;

		for (auto obj : m_ObjectList) {
			sceneData[obj->GetName()] = obj->Serialize();
		}

		return sceneData;

		//std::ofstream savedScene("Scene" + std::to_string(++m_SceneNum) + ".json");
		//savedScene << sceneData.dump(4) << std::endl;
	}

	// TODO: Create an object list getter
	std::vector<Object*> m_ObjectList;
};

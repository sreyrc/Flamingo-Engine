#pragma once

#include <glm/glm.hpp>

#include "Components.h"

class Object
{
public:
	Object() : m_Name(""), m_IsDeleted(false) {};
	Object(std::string name) : m_Name(name), m_IsDeleted(false) {}

	void AddComponent(Component* comp) {
		comp->SetParent(this);
		m_Components.push_back(comp);
	}

	template<typename T>
	T GetComponent() {
		for (auto comp : m_Components) {
			if (dynamic_cast<T>(comp)) {
				return dynamic_cast<T>(comp);
			}
		}
		return nullptr;
	}

	void Initialize() {
		for (auto& comp : m_Components) {
			comp->Initialize();
		}
	}

	void Update() {
		for(auto comp : m_Components) {
			comp->Update();
		}
	}

	inline std::string GetName() { return m_Name; }
	inline bool isDeleted() { return m_IsDeleted; }
	inline void MarkAsDeleted(bool deleted) { m_IsDeleted = deleted; }

	inline std::vector<Component*> GetComponents() {
		return m_Components;
	}

	nlohmann::json::value_type Serialize() {
		nlohmann::json jsonObject;
		for (auto comp : m_Components) {
			jsonObject[comp->GetName()] = comp->Serialize();
		}
		return jsonObject;
	}

	~Object() {
		for (auto it = m_Components.begin(); it != m_Components.end(); it++) {
			delete *it;
		}
		m_Components.clear();
	}

	inline void SetName(std::string name) { m_Name = name; }

	char m_ObjectNameBuff[255];
private:
	std::string m_Name = ""; bool m_IsDeleted;
	std::vector<Component*> m_Components;
};


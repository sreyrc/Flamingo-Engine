#pragma once

#include <glm/glm.hpp>

#include "Components.h"

class Object
{
public:
	Object() : m_Name("") {};
	Object(std::string name) : m_Name(name) {}

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
		for (auto comp : m_Components) {
			delete comp;
		}
		m_Components.clear();
	}

private:
	std::string m_Name = ""; bool m_IsDeleted = false;
	std::vector<Component*> m_Components;
};


#pragma once

#include "Model.h"
#include "BoundingVolume.h"
#include "ResourceManager.h"
#include "AABB.h"

#include <nlohmann/json.hpp>

class Object;

// Component Base class
class Component {
public:
	//virtual ~Component() = 0;
	virtual void Update() = 0;
	virtual std::string GetName() = 0;
	virtual void Initialize() = 0;

	virtual void Deserialize(nlohmann::json::value_type& jsonObj,
		ResourceManager* p_ResourceManager) = 0;

	virtual nlohmann::json::value_type Serialize () = 0;

	inline void SetParent(Object* parent) { m_Parent = parent; }
	Object* GetParent() { return m_Parent; }

protected:
	Object* m_Parent;
};

// Transform component
class Transform : public Component {
public:
	//Transform() {};
	Transform() : m_Position(0),
		m_Rotation(0),
		m_Scale(0),
		m_WorldTransform(1.0f) {};

	//virtual ~Transform() {};

	virtual std::string GetName() { return "Transform"; }

	virtual void Initialize() {}

	virtual void Deserialize(nlohmann::json::value_type& jsonObj,
		ResourceManager* p_ResourceManager = nullptr) {

		m_Position = glm::vec3(jsonObj["Position"][0],
			jsonObj["Position"][1], jsonObj["Position"][2]);

		m_Scale = glm::vec3(jsonObj["Scale"][0],
			jsonObj["Scale"][1], jsonObj["Scale"][2]);

		m_Rotation = glm::vec3(jsonObj["Rotation"][0],
			jsonObj["Rotation"][1], jsonObj["Rotation"][2]);

		glm::mat4 translationMat = glm::translate(glm::mat4(1.0f), m_Position);
		glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), m_Scale);
		glm::mat4 rotateMat = glm::rotate(glm::mat4(1.0f),
			glm::radians(m_Rotation.y), glm::vec3(0, 1, 0));
		m_WorldTransform = translationMat * rotateMat * scaleMat;
	};

	nlohmann::json::value_type Serialize() {

		nlohmann::json jsonObject;

		jsonObject["Position"] =
			nlohmann::json::array({ m_Position.x, m_Position.y, m_Position.z });
		jsonObject["Scale"] =
			nlohmann::json::array({ m_Scale.x, m_Scale.y, m_Scale.z });
		jsonObject["Rotation"] =
			nlohmann::json::array({ m_Rotation.x, m_Rotation.y, m_Rotation.z });

		return jsonObject;
	}

	virtual void Update() {
		// TODO: Factor in rotation properly
		glm::mat4 translationMat = glm::translate(glm::mat4(1.0f), m_Position);
		glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), m_Scale);
		glm::mat4 rotateMat = glm::rotate(glm::mat4(1.0f),
			glm::radians(m_Rotation.y), glm::vec3(0, 1, 0));
		//glm::mat4 rotateMat = glm::mat4_cast(m_Rotation);
		m_WorldTransform = translationMat * rotateMat * scaleMat;
	}

	//inline glm::vec3& GetPosition() { return m_Position; }
	//inline glm::quat& GetRotation() { return m_Rotation; }
	//inline glm::vec3& GetScale() { return m_Scale; }

	inline glm::mat4 GetWorldTransform() { return m_WorldTransform; }

	//inline void SetPosition(glm::vec3 pos) { m_Position = pos; }
	//inline void SetRotation(glm::quat rot) { m_Rotation = rot; }
	//inline void SetScale(glm::vec3 scale) { m_Scale = scale; }

	glm::vec3 m_Position, m_Scale;
	glm::vec3 m_Rotation;
	glm::mat4 m_WorldTransform;
};

// Collider Component
class Collider : public Component {	
public:
	Collider() : m_BVLevel1(nullptr), m_BVLevel2(nullptr) {}

	//virtual ~Collider() {}

	virtual void Initialize() {
		m_BVLevel1->Initialize();
		m_BVLevel2->Initialize();
	}

	virtual std::string GetName() { return "Collider"; }

	void Draw() {
		m_BVLevel1->Draw();

		//  TODO: Activate this when ready
		//m_BVLevel2->UpdateVBDataAndDraw();
	}

	void Deserialize(nlohmann::json::value_type& jsonObj,
		ResourceManager* p_ResourceManager) {

		if (jsonObj["BV_Level_1"] == "AABB") {
			m_BVLevel1 = new AABB();
		}
		if (jsonObj["BV_Level_2"] == "AABB") {
			m_BVLevel2 = new AABB();
		}

		m_BVLevel1->SetParentCollider(this);
		m_BVLevel2->SetParentCollider(this);
	}

	// Passing in sth["Collider"]
	nlohmann::json::value_type Serialize() {
		nlohmann::json jsonObject;

		if (m_BVLevel1Type == BVType::AABB)
			jsonObject["BV_Level_1"] = "AABB";
		if (m_BVLevel2Type == BVType::AABB)
			jsonObject["BV_Level_2"] = "AABB";

		return jsonObject;
	}

	virtual void Update() {
		m_BVLevel1->Update();

		// TODO: Activate this when ready
		//m_BVLevel2->Update();
	};

	inline void IsInCollision(bool inCol) { m_inCollision = inCol; }
	inline bool inCollision() { return m_inCollision; }

	BoundingVolume* m_BVLevel1, * m_BVLevel2;

private:
	BVType m_BVLevel1Type, m_BVLevel2Type;
	bool m_inCollision = false;
};


// Model Component
class ModelComp : public Component {

public:
	ModelComp() : m_Model(nullptr) {};
	ModelComp(Model* model) : m_Model(model) {}
	//virtual ~ModelComp() {}
	virtual void Update() {}
	virtual void Initialize() {}

	virtual std::string GetName() { return "Model"; }

	virtual void Deserialize(nlohmann::json::value_type& jsonObj,
		ResourceManager* p_ResourceManager) {

		m_ModelName = jsonObj;
		m_ModelSelected = jsonObj;
		m_Model = p_ResourceManager->GetModel(m_ModelName);
	}

	// value of sth["Model"]
	nlohmann::json::value_type Serialize() {
		return m_ModelName;
	}

	inline Model* GetModel() { return m_Model; }
	void SetModel(ResourceManager* p_ResourceManager) {
		m_ModelName = m_ModelSelected;
		m_Model = p_ResourceManager->GetModel(m_ModelSelected);
	}

	inline void Draw(Shader& shader) { 
		m_Model->Draw(shader); 
	}

	std::string m_ModelSelected;

private:
	Model* m_Model;
	std::string m_ModelName;
};



class Material : public Component {
public:
	Material() : m_Albedo(0), m_Metallic(0), m_Roughness(0), m_AO(0) {}
	//virtual ~Material() {}
	virtual void Update() {}
	virtual std::string GetName() { return "Material"; }
	virtual void Initialize() {}

	virtual void Deserialize(nlohmann::json::value_type& jsonObj,
		ResourceManager* p_ResourceManager = nullptr) {

		m_Albedo = glm::vec3(jsonObj["Albedo"][0],
			jsonObj["Albedo"][1], jsonObj["Albedo"][2]);

		m_Metallic = jsonObj["Metalness"];
		m_Roughness = jsonObj["Roughness"];
		m_AO = jsonObj["AO"];
	};

	// Value of json["Material"]
	nlohmann::json::value_type Serialize() {

		nlohmann::json jsonObject;
		jsonObject["Albedo"] = nlohmann::json::array(
			{m_Albedo.x, m_Albedo.y, m_Albedo.z});
		jsonObject["Metalness"] = m_Metallic;
		jsonObject["Roughness"] = m_Roughness;
		jsonObject["AO"] = m_AO;
		return jsonObject;
	}

	glm::vec3 m_Albedo;
	float m_Metallic, m_Roughness, m_AO;
};


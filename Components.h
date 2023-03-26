#pragma once

#include "Model.h"
#include "BoundingVolume.h"
#include "ResourceManager.h"
#include "AABB.h"
#include "OBB.h"

#include <glm/gtx/quaternion.hpp>

#include <nlohmann/json.hpp>

class Object;
class Shader;

// Component Base class
class Component {
public:
	virtual ~Component() {};
	virtual void Update() = 0;
	virtual std::string GetName() = 0;
	virtual void Initialize() = 0;

	virtual void Deserialize(nlohmann::json::value_type& jsonObj,
		ResourceManager* p_ResourceManager) = 0;

	virtual void SetDefaults(ResourceManager* p_ResourceManager) = 0;

	virtual nlohmann::json::value_type Serialize () = 0;

	inline void SetParent(Object* parent) { m_Parent = parent; }
	Object* GetParent() { return m_Parent; }

protected:
	Object* m_Parent = nullptr;
};

// Transform component
class Transform : public Component {
public:
	//Transform() {};
	Transform() : m_Position(0.0f),
		m_Rotation(1.0f, 0.0f, 0.0f, 0.0f),
		m_RotationEuler(0.0f),
		m_Scale(1.0f),
		m_WorldTransform(1.0f) {};

	virtual ~Transform() {};

	virtual std::string GetName() { return "Transform"; }

	void Initialize() {}
	void SetDefaults(ResourceManager* p_ResourceManager) {}

	void Deserialize(nlohmann::json::value_type& jsonObj,
		ResourceManager* p_ResourceManager = nullptr) {

		m_Position = glm::vec3(jsonObj["Position"][0],
			jsonObj["Position"][1], jsonObj["Position"][2]);

		m_Scale = glm::vec3(jsonObj["Scale"][0],
			jsonObj["Scale"][1], jsonObj["Scale"][2]);

		m_RotationEuler = glm::vec3(jsonObj["Rotation"][0],
			jsonObj["Rotation"][1], jsonObj["Rotation"][2]);

		m_RotationEulerRad = glm::vec3(
			glm::radians(m_RotationEuler.x),
			glm::radians(m_RotationEuler.y),
			glm::radians(m_RotationEuler.z)
		);

		m_Rotation = glm::quat(m_RotationEulerRad);

		glm::mat4 translationMat = glm::translate(glm::mat4(1.0f), m_Position);
		glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), m_Scale);
		glm::mat4 rotateMat = glm::toMat4(m_Rotation);
		//(1.0f);
		//rotateMat = glm::rotate(rotateMat, glm::radians(m_Rotation.y), glm::vec3(0, 1, 0));
		//rotateMat = glm::rotate(rotateMat, glm::radians(m_Rotation.x), glm::vec3(1, 0, 0));
		//rotateMat = glm::rotate(rotateMat, glm::radians(m_Rotation.z), glm::vec3(0, 0, 1));
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

	void Update() {
		// TODO: Factor in rotation properly
		glm::mat4 translationMat = glm::translate(glm::mat4(1.0f), m_Position);
		glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), m_Scale);
		glm::mat4 rotateMat(1.0f);// = glm::rotate(glm::mat4(1.0f),
		m_RotationEulerRad = glm::vec3(
			glm::radians(m_RotationEuler.x),
			glm::radians(m_RotationEuler.y),
			glm::radians(m_RotationEuler.z)
		);
		m_Rotation = glm::quat(m_RotationEulerRad);
		rotateMat = glm::toMat4(m_Rotation);
		m_WorldTransform = translationMat * rotateMat * scaleMat;
	}

	inline glm::mat4 GetWorldTransform() { return m_WorldTransform; }

	glm::vec3 m_Position, m_Scale;
	glm::quat m_Rotation;
	glm::vec3 m_RotationEuler, m_RotationEulerRad;
	glm::mat4 m_WorldTransform;
};

// Collider Component
class Collider : public Component {	
public:
	Collider() : m_BVLevel1(nullptr), m_BVLevel2(nullptr) {}

	virtual ~Collider() {
		delete m_BVLevel1;
		delete m_BVLevel2;
	}

	void SetDefaults(ResourceManager* p_ResourceManager) {
		m_BVLevel1 = new AABB();
		m_BVLevel2 = new OBB();
		m_BVLevel1->SetParentCollider(this);
		m_BVLevel2->SetParentCollider(this);
	}

	void Initialize() {
		m_BVLevel1->Initialize();
		m_BVLevel2->Initialize();
	}

	std::string GetName() { return "Collider"; }

	void Draw(Shader* shader) {
		m_BVLevel1->Draw(shader);
		m_BVLevel2->Draw(shader);
	}

	// hashmap[str comp_name] = CollCreator->create();

	void Deserialize(nlohmann::json::value_type& jsonObj,
		ResourceManager* p_ResourceManager) {

		// TODO: This yucky "if" - approach isn't scalable. 
		// Replace with an alternative eventually
		if (jsonObj["BV_Level_1"] == "AABB") { m_BVLevel1 = new AABB(); m_BVLevel1Type = BVType::AABB; }
		if (jsonObj["BV_Level_1"] == "OBB") { m_BVLevel1 = new OBB(); m_BVLevel1Type = BVType::OBB; } 

		if (jsonObj["BV_Level_2"] == "AABB") { m_BVLevel2 = new AABB(); m_BVLevel2Type = BVType::AABB; }
		if (jsonObj["BV_Level_2"] == "OBB") { m_BVLevel2 = new OBB(); m_BVLevel2Type = BVType::OBB; }

		m_BVLevel1->SetParentCollider(this);
		m_BVLevel2->SetParentCollider(this);
	}

	// Passing in sth["Collider"]
	nlohmann::json::value_type Serialize() {
		nlohmann::json jsonObject;

		if (m_BVLevel1Type == BVType::AABB)
			jsonObject["BV_Level_1"] = "AABB";
		if (m_BVLevel1Type == BVType::OBB)
			jsonObject["BV_Level_1"] = "OBB";
		if (m_BVLevel2Type == BVType::AABB)
			jsonObject["BV_Level_2"] = "AABB";
		if (m_BVLevel2Type == BVType::OBB)
			jsonObject["BV_Level_2"] = "OBB";

		return jsonObject;
	}

	void Update() {
		m_BVLevel1->Update();

		// TODO: Activate this when ready
		m_BVLevel2->Update();
	};

	inline void IsInCollision(bool inCol) {
		m_BVLevel1->IsInCollision(inCol);
		m_BVLevel2->IsInCollision(inCol);
	}
	//inline bool inCollision() { return m_inCollision; }

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
	~ModelComp() {}
	void Update() {}
	void Initialize() {}

	std::string GetName() { return "Model"; }

	void SetDefaults(ResourceManager* p_ResourceManager) {
		// Set the first model as default
		m_ModelName = p_ResourceManager->m_ModelNames[0];
		m_Model = p_ResourceManager->GetModel(p_ResourceManager->m_ModelNames[0]);
	}

	void Deserialize(nlohmann::json::value_type& jsonObj,
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
	Material() : m_Albedo(1.0f), m_Metalness(0.5f), m_Roughness(0.5f), m_AO(0.0f) {}
	~Material() {}
	void Update() {}
	std::string GetName() { return "Material"; }
	void Initialize() {}

	void SetDefaults(ResourceManager* p_ResourceManager) {}

	void Deserialize(nlohmann::json::value_type& jsonObj,
		ResourceManager* p_ResourceManager = nullptr) {

		m_Albedo = glm::vec3(jsonObj["Albedo"][0],
			jsonObj["Albedo"][1], jsonObj["Albedo"][2]);

		m_Metalness = jsonObj["Metalness"];
		m_Roughness = jsonObj["Roughness"];
		m_AO = jsonObj["AO"];
	};

	// Value of json["Material"]
	nlohmann::json::value_type Serialize() {

		nlohmann::json jsonObject;
		jsonObject["Albedo"] = nlohmann::json::array(
			{m_Albedo.x, m_Albedo.y, m_Albedo.z});
		jsonObject["Metalness"] = m_Metalness;
		jsonObject["Roughness"] = m_Roughness;
		jsonObject["AO"] = m_AO;
		return jsonObject;
	}

	glm::vec3 m_Albedo;
	float m_Metalness, m_Roughness, m_AO;
};


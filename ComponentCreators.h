#pragma once

#include "Components.h"

class ComponentCreator {
public:
	virtual Component* Create() = 0;
};

class TransformCreator : public ComponentCreator {
public:
	Transform* Create() { 
		return new Transform();
	}
};

class ModelCompCreator : public ComponentCreator {
public:
	ModelComp* Create() {
		return new ModelComp();
	}
};

class ColliderCreator : public ComponentCreator {
public:
	Collider* Create() {
		return new Collider();
	}
};

class MaterialCreator : public ComponentCreator {
public:
	Material* Create() {
		return new Material();
	}
};


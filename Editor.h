#pragma once

#include "Renderer.h"
#include "SceneManager.h"
#include "CollisionWorld.h"

#include <glfw/glfw3.h>

// Handling changing values in the Renderer with Dear Imgui
class Editor 
{
public:
	Editor(GLFWwindow* window);

	void Update( 
		ObjectManager* p_GetObjectManager,
		ObjectFactory* p_ObjectFactory,
		SceneManager* p_SceneManager,
		ResourceManager* p_ResourceManager,
		CollisionWorld* p_CollisionWorld,
		Renderer* p_Renderer);
};
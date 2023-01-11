/*
**********************************************************
Framework by Sreyash(Srey) Raychaudhuri
Collision Detection / Fried Engine
**********************************************************
* */

#include <glad/glad.h>

#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <iomanip>
#include <memory>
#include <algorithm>
#include <iostream>
#include <cmath>

#include "Shader.h"
#include "Camera.h"
#include "Renderer.h"
#include "Editor.h"
#include "CollisionWorld.h"
#include "ResourceManager.h"

#include "Object.h"
#include "ObjectFactory.h"
#include "ObjectManager.h"
#include "SceneManager.h"

#include "AABB.h"

#include <glm/gtx/matrix_decompose.hpp>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, Camera* camera);

const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;


void PrintVec3(glm::vec3 vec3) {
    std::cout << vec3.x << ", " << vec3.y << ", " << vec3.z << '\n';
}

// TODO: Add manager creation logic here
void CreateManagers() {

}

int main() {

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Fried Engine", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    CreateManagers();

    // Create a camera
    Camera* p_Camera = new Camera(glm::vec3(0.0f, 0.0f, 3.0f));

    // Soft body
    //SoftBody* softBody = new SoftBody(4, 4, 4);

    // Create renderer for drawing stuff
    Renderer* p_Renderer = new Renderer(p_Camera, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Create an editor (for changing params with Dear Imgui)
    Editor* p_Editor = new Editor(window);

    // To check for collisions
    CollisionWorld* p_CollisionWorld = new CollisionWorld();

    // To store resources - models and textures
    ResourceManager* p_ResourceManager = new ResourceManager();

    // Contains all objects and manages them (addition, deletion etc)
    ObjectManager* p_ObjectManager = new ObjectManager();

    // For creating objects (from scenes)
    ObjectFactory* p_ObjectFactory = new ObjectFactory();

    // To manage scenes - loading, saving and unloading scenes
    SceneManager* p_SceneManager = new SceneManager(p_ObjectManager,
        p_ObjectFactory, p_ResourceManager, p_CollisionWorld, p_Renderer);

    lastFrame = glfwGetTime();

    // Update loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // Process all keyboard input
        processInput(window, p_Camera);
       
        p_ObjectManager->Update();

        // Check for collisions
        p_CollisionWorld->Update();

        // Draw call
        p_Renderer->Draw(p_ObjectManager->m_ObjectList);

        // If changes made with Imgui interface, update
        p_Editor->Update(p_ObjectManager, p_ObjectFactory,
            p_SceneManager, p_ResourceManager, p_CollisionWorld, p_Renderer);

        p_CollisionWorld->ClearCollisionQueues();

        // Delete necessary objects
        p_ObjectManager->DestroyDeletedObjects();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //ImPlot::DestroyContext();

    // Delete the rest of the stuff
    delete p_Camera;
    delete p_Renderer;
    delete p_Editor;
    delete p_CollisionWorld;
    delete p_ObjectManager;
    delete p_ObjectFactory;
    delete p_ResourceManager;

    glfwTerminate();
    return 0;
}

// To make sure the viewport matches the new window dimensions; 
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// For (keyboard) input processing
void processInput(GLFWwindow* window, Camera* camera)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // For camera translation
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera->ProcessKeyboard(MOVE_FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera->ProcessKeyboard(MOVE_BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera->ProcessKeyboard(MOVE_LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera->ProcessKeyboard(MOVE_RIGHT, deltaTime);

    // For Camera Rotation
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera->ProcessKeyboard(TURN_UPWARDS, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera->ProcessKeyboard(TURN_DOWNWARDS, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        camera->ProcessKeyboard(TURN_LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        camera->ProcessKeyboard(TURN_RIGHT, deltaTime);

    // For Zooming
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        camera->ProcessKeyboard(ZOOM_IN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        camera->ProcessKeyboard(ZOOM_OUT, deltaTime);

    // To Reset to initial Camera positions
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        camera->ProcessKeyboard(RESET, deltaTime);
}

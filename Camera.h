#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// Defines several possible actions for camera manipulation
enum Camera_Actions {
    MOVE_FORWARD,
    MOVE_BACKWARD,
    MOVE_LEFT,
    MOVE_RIGHT,
    TURN_UPWARDS,
    TURN_DOWNWARDS,
    TURN_LEFT,
    TURN_RIGHT,
    ZOOM_IN,
    ZOOM_OUT,
    RESET
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float MOVESPEED = 20.0f;
const float ROTSPEED = 50.0f;
const float ZOOMSPEED = 20.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;


class Camera
{
public:
    glm::vec3 m_InitPos;
    glm::vec3 m_Position;
    glm::vec3 m_Front;
    glm::vec3 m_Up;
    glm::vec3 m_Right;
    glm::vec3 m_WorldUp;
    
    float m_Yaw;
    float m_Pitch;

    float m_MovementSpeed;
    float m_RotationSpeed;
    float m_ZoomSpeed;
    float m_Zoom;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = YAW,
        float pitch = PITCH);

    Camera(float posX, float posY,
        float posZ, float upX,
        float upY, float upZ,
        float yaw, float pitch);


    // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix() {
        return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
    }

    // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Actions action, float deltaTime);

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void UpdateCameraVectors();
};

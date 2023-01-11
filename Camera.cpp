#include "Camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : m_Front(glm::vec3(0.0f, 0.0f, -1.0f)),
    m_MovementSpeed(MOVESPEED),
    m_RotationSpeed(ROTSPEED),
    m_Zoom(ZOOM)
{
    // Intialize variables
    m_InitPos = position;
    m_Position = position;
    m_WorldUp = up;
    m_Yaw = yaw;
    m_Pitch = pitch;
    UpdateCameraVectors();
}

Camera::Camera(float posX, float posY, 
    float posZ, float upX, 
    float upY, float upZ, 
    float yaw, float pitch) 
    :
    m_Front(glm::vec3(0.0f, 0.0f, -1.0f)),
    m_MovementSpeed(MOVESPEED),
    m_RotationSpeed(ROTSPEED),
    m_Zoom(ZOOM)
{
    // Initialize camera params
    m_Position = glm::vec3(posX, posY, posZ);
    m_WorldUp = glm::vec3(upX, upY, upZ);
    m_Yaw = yaw;
    m_Pitch = pitch;
    UpdateCameraVectors();
}

// Process keyboard input according to action
void Camera::ProcessKeyboard(Camera_Actions action, float deltaTime)
{
    float moveSpeed = m_MovementSpeed * deltaTime;
    float rotSpeed = m_RotationSpeed * deltaTime;
    float zoomSpeed = m_ZoomSpeed * deltaTime;

    // For movement
    if (action == MOVE_FORWARD)
        m_Position += m_Front * moveSpeed;
    if (action == MOVE_BACKWARD)
        m_Position -= m_Front * moveSpeed;
    if (action == MOVE_LEFT)
        m_Position -= m_Right * moveSpeed;
    if (action == MOVE_RIGHT)
        m_Position += m_Right * moveSpeed;

    // For camera dir
    if (action == TURN_UPWARDS)
        m_Pitch += rotSpeed;
    if (action == TURN_DOWNWARDS)
        m_Pitch -= rotSpeed;
    if (action == TURN_LEFT)
        m_Yaw -= rotSpeed;
    if (action == TURN_RIGHT)
        m_Yaw += rotSpeed;


    // For zoom
    if (action == ZOOM_IN)
        m_Zoom -= ZOOMSPEED * deltaTime;
    if (action == ZOOM_OUT)
        m_Zoom += ZOOMSPEED * deltaTime;

    // Clamp zoom between 1.0f and 90.0f
    if (m_Zoom < 1.0f)
        m_Zoom = 1.0f;
    if (m_Zoom > 90.0f)
        m_Zoom = 90.0f;

    // Reset camera params
    if (action == RESET) {
        m_Zoom = ZOOM;
        m_Position = m_InitPos;
        m_WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        m_Yaw = YAW;
        m_Pitch = PITCH;
    }

    UpdateCameraVectors();
}

void Camera::UpdateCameraVectors()
{
    // Calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front.y = sin(glm::radians(m_Pitch));
    front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    m_Front = glm::normalize(front);
    
    // Also re-calculate the Right and Up vector
    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}
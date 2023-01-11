#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <vector>

#include "Shader.h"

#define MAX_BONE_INFLUENCE 4

// Vertex attributes
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
};

class Mesh {
public:
    // Mesh Data
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    unsigned int VAO;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);

    //Render the mesh
    void Draw(Shader& shader);

private:
    // Render data 
    unsigned int VBO, EBO;

    // Initializes all the buffer objects/arrays
    void SetupMesh();
};
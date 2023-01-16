#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <vector>

#include "Shader.h"
#include "Texture.h"

// Vertex attributes
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

class Mesh {
public:
    // Mesh Data
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture*>     textures;

    unsigned int VAO;

    Mesh(std::vector<Vertex> vertices, 
        std::vector<unsigned int> indices, 
        std::vector<Texture*> textures);

    //Render the mesh
    void Draw(Shader& shader);

    void BindTextures(Shader& shader);

private:
    // Render data 
    unsigned int VBO, EBO;

    // Initializes all the buffer objects/arrays
    void SetupMesh();
};
#include "Mesh.h"

// Load the information for each mesh
Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, 
    std::vector<Texture*> textures)
{
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    // now that we have all the required data, set the vertex buffers and its attribute pointers.
    SetupMesh();
}

// Draw the mesh
void Mesh::Draw(Shader& shader)
{
    // TODO: Reactivate textures once shadows work
    //Bind textures
    int unit = 0;
    for (int i = 0; i < textures.size(); i++) {
        if (textures[i]->GetType() == "Normal") {
            textures[i]->BindTexture(0, shader.GetID(), "normalMap");
        }
        if (textures[i]->GetType() == "Diffuse") {
            textures[i]->BindTexture(1, shader.GetID(), "diffuseMap");
        }
        if (textures[i]->GetType() == "Roughness") {
            textures[i]->BindTexture(2, shader.GetID(), "roughnessMap");
        }
        if (textures[i]->GetType() == "Metalness") {
            textures[i]->BindTexture(3, shader.GetID(), "metallicMap");
        }
    }

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::BindTextures(Shader& shader)
{
    // TODO: Add texure binding logic
}

void Mesh::SetupMesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    
    // Load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // -----
    // Set the vertex attribute pointers
    // Vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    
    // Vertex Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    
    // Vertex Texture Coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    
    // Vertex Tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
    
    // Vertex Bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
}

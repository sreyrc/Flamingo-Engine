#pragma once

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stb/stb_image.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

#include "Shader.h"
#include "Mesh.h"
#include <unordered_map>

#include "Texture.h"

#define MAX_BONE_INFLUENCE 4

struct Node 
{
    // the meshes of this node;
    std::vector<Mesh> m_Meshes;

    // All children nodes
    std::vector<Node*> m_Children;

    void DrawMeshes(Shader& shader) {
        
        // First Draw all meshes
        for (auto mesh : m_Meshes) {
            mesh.Draw(shader);
        }

        // Draw all children
        for (auto child : m_Children) {
            child->DrawMeshes(shader);
        }
    };
};

class Model
{
public:
    // constructor, expects a filepath to a 3D model.
    Model(std::string const& path)
    {
        LoadModel(path);
    }

    // Draws the model i.e. all meshes that make it up
    void Draw(Shader& shader);
    std::vector<Vertex> GetAllVertices() { return m_AllVertices; }


    bool m_UseTextures = false;

private:
    // All textures loaded for this model
    std::unordered_map<std::string, Texture*> m_TexturesLoaded;

    // All vertices that make up the meshes of this model
    std::vector<Vertex> m_AllVertices;

    // Root of the scene heirarchy tree
    Node* m_Root;

    // Loads model from given path
    void LoadModel(std::string const& path);

    // Processes nodes recursively
    Node* ProcessNode(aiNode* node, const aiScene* scene);

    // Processes each mesh. Obtains mesh data from ASSIMP data structures and populates the VBOs
    Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);

    Texture* LoadMaterialTexture(std::string typeName);

    std::string m_Directory;
};
#include "Model.h"

void Model::Draw(Shader& shader)
{
    m_Root->DrawMeshes(shader);
}

void Model::LoadModel(std::string const& path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, 
        aiProcess_Triangulate | 
        aiProcess_GenSmoothNormals | 
        aiProcess_CalcTangentSpace
    );
    
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR! " << importer.GetErrorString() << std::endl;
        return;
    }

    // process ASSIMP's root node recursively
    m_Root = ProcessNode(scene->mRootNode, scene);
}

Node* Model::ProcessNode(aiNode* node, const aiScene* scene)
{
    // Create a new node
    Node* newNode = new Node();

    // Process each mesh located at the current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        newNode->m_Meshes.push_back(ProcessMesh(mesh, scene));
    }

    // After we've processed all of the meshes (if any) we then 
    // recursively process each of the children nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        newNode->m_Children.push_back(ProcessNode(node->mChildren[i], scene));
    }

    return newNode;
}

Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
    // Data to fill
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // Go through each of the mesh's vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector; 
        
        // Positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;

        // Normals
        if (mesh->HasNormals())
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        }

        vertices.push_back(vertex);
        m_AllVertices.push_back(vertex);
    }

    // Go through the mesh's faces (a face is a mesh its triangle) 
    // and retrieve the corresponding vertex indices.
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        // Retrieve all indices of the face and store them in the indices vector
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    return Mesh(vertices, indices);
}

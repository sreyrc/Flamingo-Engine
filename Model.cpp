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

    // path = "../res/abc/abc.extension"
    // need: "../res/abc/"
    m_Directory = path.substr(0, path.find_last_of('/'));

    // Process ASSIMP's root node recursively
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
    std::vector<Texture*> textures;

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

        // Texture coordinates
        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
            
            // Tangent
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector;

            // Bitangent
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.Bitangent = vector;
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

    // process materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];


   // aiString fileBaseColor, fileMetallic, fileRoughness;
    if (material) {
        Texture* diffuseMap = LoadMaterialTexture("Diffuse");
        if (diffuseMap) textures.push_back(diffuseMap);

        Texture* normalMap = LoadMaterialTexture("Normal");
        if (normalMap) textures.push_back(normalMap);

        Texture* roughnessMap = LoadMaterialTexture("Roughness");
        if (roughnessMap) textures.push_back(roughnessMap);

        Texture* metalnessMap = LoadMaterialTexture("Metalness");
        if (metalnessMap) textures.push_back(metalnessMap);
    }

    
    //// 1. Diffuse maps
    //std::vector<Texture*> diffuseMaps = LoadMaterialTextures("Diffuse");
    //textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    //
    //// 3. Normal maps
    //std::vector<Texture*> normalMaps = LoadMaterialTextures("Normal");
    //textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
  
    //// 2. Roughness and Metalnesss maps
    //std::vector<Texture*> roughnessMaps = LoadMaterialTextures("Roughness");
    //textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());

    return Mesh(vertices, indices, textures);
}


// checks all material textures of a given type and loads the textures if they're not loaded yet.
// the required info is returned as a Texture struct.
Texture* Model::LoadMaterialTexture(std::string typeName)
{
    std::string texPath = m_Directory + "/textures/" + typeName + ".png";
    Texture* texture = new Texture(texPath, typeName);
    if (!texture->isLoaded()) {
        delete texture; return nullptr;
    }
    // If texture at this path is not found - add this tex
    if (m_TexturesLoaded.find(texPath) == m_TexturesLoaded.end()) {
        m_TexturesLoaded[texPath] = texture;
    }
    return m_TexturesLoaded.at(texPath);
};


#include <glad/glad.h>
#include <stb/stb_image.h>

#include "Renderer.h"

unsigned int LoadCubemap(std::vector<std::string> faces);

Renderer::Renderer(Camera* cam, int SCREEN_WIDTH, int SCREEN_HEIGHT)
{
    // Setting initial params for scene and lighting
    m_BgColor = glm::vec3(0.0f, 0.0f, 0.0f);
    m_ViewPos = glm::vec3(0.0f, 0.1f, 0.6f);
    m_DiffuseColor = glm::vec3(1.0f, 0.0f, 1.0f);
    m_SpecularColor = glm::vec3(1.0f, 1.0f, 1.0f);
    m_LightPos = glm::vec3(2.0f, 2.5f, 2.5f);
    m_LightColor = glm::vec3(1.0f, 1.0f, 1.0f);

    m_LineWidth = 1.0f;
    m_LineColor = glm::vec3(1.0f, 1.0f, 1.0f);

    // Loading shaders: one for the model and the other for line drawing
    m_ModelShader = new Shader("ModelShader.vert", "ModelShader.frag");
    m_ModelShaderPBR = new Shader("ModelShaderPBR.vert", "ModelShaderPBR.frag");
    m_LineShader = new Shader("LineShader.vert", "LineShader.frag"); 
    //m_SkyBoxShader = new Shader("Skybox.vert", "Skybox.frag");

    //m_SkyBoxShader->Use();
    //m_SkyBoxShader->SetInt("skybox", 0);

    // Set up camera
    m_Camera = cam;

    // Initialize matrices
    m_ModelMat = glm::mat4(1.0f);
    m_ViewMat = glm::mat4(1.0f);

    m_ModelPos = glm::vec3(0.0f);

    m_RotMat = glm::mat4(1.0f);

    m_SphereMesh = new SphereMesh();

    m_ProjMat = glm::perspective(glm::radians(m_Camera->m_Zoom),
        (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 1000.0f);

    m_SkyBoxVAO = 0; m_SkyBoxVBO = 0;

    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    // first, configure the cube's VAO (and VBO)
    glGenVertexArrays(1, &m_SkyBoxVAO);
    glGenBuffers(1, &m_SkyBoxVBO);

    glBindBuffer(GL_ARRAY_BUFFER, m_SkyBoxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    glBindVertexArray(m_SkyBoxVAO);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    std::vector<std::string> faces
    {
        "../skybox/right.jpg",
        "../skybox/left.jpg",
        "../skybox/top.jpg",
        "../skybox/bottom.jpg",
        "../skybox/front.jpg",
        "../skybox/back.jpg"
    };
    m_CubeMapTexID = LoadCubemap(faces);
}


// Draw skeleton and model if opted for
void Renderer::Draw(std::vector<Object*>& objects)
{
    glEnable(GL_DEPTH_TEST);
 
    // Set BG color and clear buffers
    //glClearColor(m_BgColor.x, m_BgColor.y, m_BgColor.z, 1.0f);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
    //m_SkyBoxShader->Use();

    m_ViewMat = m_Camera->GetViewMatrix();
    m_ViewPos = m_Camera->m_Position;

    //glm::mat4 view = glm::mat4(glm::mat3(m_ViewMat));
    //m_SkyBoxShader->SetMat4("view", view);
    //m_SkyBoxShader->SetMat4("proj", m_ProjMat);

    //// Skybox cube
    //glBindVertexArray(m_SkyBoxVAO);
    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMapTexID);
    //glDrawArrays(GL_TRIANGLES, 0, 36);
    //glBindVertexArray(0);
    //glDepthFunc(GL_LESS);

    //SetupMatrices(glm::vec3(0, 0, 0));
    //m_SphereMesh->BindVAO();
    //glDrawElements(GL_TRIANGLE_STRIP,
    //    m_SphereMesh->GetIndexCount(), GL_UNSIGNED_INT, 0);

    // Draw the models
    m_ModelShaderPBR->Use();
    m_ModelShaderPBR->SetMat4("proj", m_ProjMat);
    m_ModelShaderPBR->SetMat4("view", m_ViewMat);

    SetModelShaderVars();

    // Draw the models/meshes
    for (auto obj : objects) {
        m_ModelShaderPBR->SetMat4("model", 
            obj->GetComponent<Transform*>()->GetWorldTransform());

        // Set material properties
        auto material = obj->GetComponent<Material*>();
        if (material) {
            m_ModelShaderPBR->SetVec3("material.albedo", material->m_Albedo);
            m_ModelShaderPBR->SetFloat("material.metallic", material->m_Metallic);
            m_ModelShaderPBR->SetFloat("material.roughness", material->m_Roughness);
            m_ModelShaderPBR->SetFloat("material.ao", material->m_AO);
        }

        // Draw call
        obj->GetComponent<ModelComp*>()->Draw(*m_ModelShaderPBR);
    }

    SetupLineShaderVars();

    // Draw the Bounding Volumes of Colliders
    for (auto obj : objects) {
        auto col = obj->GetComponent<Collider*>();
        if (col->inCollision() ) m_LineShader->SetVec3("lineColor", glm::vec3(1.0f, 0.0f, 0.0f));
        else m_LineShader->SetVec3("lineColor", glm::vec3(1.0f, 1.0f, 1.0f));
        col->Draw();
    }
}

void Renderer::Deserialize(std::string path) {
    m_Lights.clear();
    std::ifstream sceneFile(path);
    nlohmann::json allData = nlohmann::json::parse(sceneFile);
    auto& data = allData["Lights"];

    for (auto it = data.begin(); it != data.end(); it++) {
        auto lightObj = *it;
        Light light;

        light.position = glm::vec3(lightObj["Position"][0],
            lightObj["Position"][1], lightObj["Position"][2]);
        
        light.color = glm::vec3(lightObj["Color"][0],
            lightObj["Color"][1], lightObj["Color"][2]);

        m_Lights.push_back(light);
    }

    std::cout << "Configured";
    // Initialize all objects and their components.
    //p_ObjManager->Initialize();

}

nlohmann::json::value_type Renderer::Serialize()
{
    nlohmann::json lightsData;
    for (int i = 0; i < m_Lights.size(); i++) {
        
        nlohmann::json currLightData;
        currLightData["Position"] = nlohmann::json::array ({ 
                m_Lights[i].position.x, 
                m_Lights[i].position.y,
                m_Lights[i].position.z
            });

        currLightData["Color"] = nlohmann::json::array({
                m_Lights[i].color.x,
                m_Lights[i].color.y,
                m_Lights[i].color.z
            });

        lightsData["Light_" + std::to_string(i)] = currLightData;
    }

    return lightsData;
}

// Set variables in the shader for the model
void Renderer::SetModelShaderVars() {

    for (int i = 0; i < m_Lights.size(); i++) {
        // Set light position and color (for now, just one light source)
        m_ModelShaderPBR->SetVec3("lights[" + std::to_string(i) + "].position", m_Lights[i].position);
        m_ModelShaderPBR->SetVec3("lights[" + std::to_string(i) + "].color", m_Lights[i].color);
    }

    // TODO: Uncommment soon
    //// Set light position and color (for now, just one light source)
    //m_ModelShaderPBR->SetVec3("lights[0].position", m_LightPos);
    //m_ModelShaderPBR->SetVec3("lights[0].color", m_LightColor);

    // Set other vars
    m_ModelShaderPBR->SetInt("numberOfLights", m_Lights.size());
    m_ModelShaderPBR->SetVec3("camPos", m_ViewPos);
}


// Pass variables to the line shader
void Renderer::SetupLineShaderVars() {
    // Set variables for line drawing shader
    m_LineShader->Use();
    m_LineShader->SetMat4("view", m_ViewMat);
    m_LineShader->SetMat4("model", glm::mat4(1.0f));
    m_LineShader->SetMat4("proj", m_ProjMat);
}

//// Draw the grid lines
//void Renderer::DrawGrid() {
//
//    glEnable(GL_DEPTH_TEST);
//    glBegin(GL_LINES);
//
//    for (int i = -5; i <= 5; i++) {
//        glVertex3f(i, 0, -5);
//        glVertex3f(i, 0, 5);
//        glVertex3f(-5, 0, i);
//        glVertex3f(5, 0, i);
//    };
//    glEnd();
//
//    glDisable(GL_LINE_SMOOTH);
//}

unsigned int LoadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
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
    m_GlobalLight.m_Position = glm::vec3(2.0f, 2.5f, 2.5f);
    m_GlobalLight.m_Color = glm::vec3(1.0f, 1.0f, 1.0f);

    m_LineWidth = 1.0f;
    //m_LineColor = glm::vec3(1.0f, 1.0f, 1.0f);

    // Creating shaders
    m_ModelShader = new Shader("ModelShader.vert", "ModelShader.frag");
    m_ModelShaderPBR = new Shader("ModelShaderPBR.vert", "ModelShaderPBR.frag");
    m_DefShaderGBuffer = new Shader("DefShaderGBufPass.vert", "DefShaderGBufPass.frag");
    m_DefShaderGBufTex = new Shader("DefShaderGBufTex.vert", "DefShaderGBufTex.frag");
    m_DefShaderLighting = new Shader("DefShaderLightingPass.vert", "DefShaderLightingPass.frag");
    m_LineShader = new Shader("LineShader.vert", "LineShader.frag"); 
    m_MultLocalLightsShader = new Shader("MultLocalLightsShader.vert", "MultLocalLightsShader.frag");
    //m_SkyBoxShader = new Shader("Skybox.vert", "Skybox.frag");

    // Set samplers for textures which will be used to fill in G-Buffer
    m_DefShaderGBufTex->Use();
    m_DefShaderGBufTex->SetInt("normalMap", 0);
    m_DefShaderGBufTex->SetInt("diffuseMap", 1);
    m_DefShaderGBufTex->SetInt("roughnessMap", 2);
    m_DefShaderGBufTex->SetInt("metallicMap", 3);
    m_DefShaderGBufTex->Unuse();

    // Set samplers. Values will be read from these
    m_DefShaderLighting->Use();
    m_DefShaderLighting->SetInt("g_Position", 0);
    m_DefShaderLighting->SetInt("g_Normal", 1);
    m_DefShaderLighting->SetInt("g_Diffuse", 2);
    m_DefShaderLighting->SetInt("g_RoughMetal", 3);
    m_DefShaderLighting->SetFloat("width", (float)SCREEN_WIDTH);
    m_DefShaderLighting->SetFloat("height", (float)SCREEN_HEIGHT);
    m_DefShaderLighting->Unuse();

    //Preferably get rid of this shader later. And try doing all with one shader
    m_MultLocalLightsShader->Use();
    m_MultLocalLightsShader->SetInt("g_Position", 0);
    m_MultLocalLightsShader->SetInt("g_Normal", 1);
    m_MultLocalLightsShader->SetInt("g_Diffuse", 2);
    m_MultLocalLightsShader->SetInt("g_RoughMetal", 3);
    m_MultLocalLightsShader->SetFloat("width", (float)SCREEN_WIDTH);
    m_MultLocalLightsShader->SetFloat("height", (float)SCREEN_HEIGHT);
    m_MultLocalLightsShader->Unuse();


    //m_SkyBoxShader->Use();
    //m_SkyBoxShader->SetInt("skybox", 0);

    // Set up camera
    m_Camera = cam;

    // Initialize matrices
    m_ModelMat = glm::mat4(1.0f);
    m_ViewMat = glm::mat4(1.0f);

    m_ModelPos = glm::vec3(0.0f);

    m_RotMat = glm::mat4(1.0f);

    //m_SphereMesh = new SphereMesh();

    m_ProjMat = glm::perspective(glm::radians(m_Camera->m_Zoom),
        (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 1000.0f);

    m_SkyBoxVAO = 0; m_SkyBoxVBO = 0;

    // Create an FBO with 4 color attachments/buffers
    FBOForDefShading.CreateFBO(SCREEN_WIDTH, SCREEN_HEIGHT, 4);

    SphereMesh sphereMesh;
    m_SphereMesh = sphereMesh;

    //float skyboxVertices[] = {
    //    // positions          
    //    -1.0f,  1.0f, -1.0f,
    //    -1.0f, -1.0f, -1.0f,
    //     1.0f, -1.0f, -1.0f,
    //     1.0f, -1.0f, -1.0f,
    //     1.0f,  1.0f, -1.0f,
    //    -1.0f,  1.0f, -1.0f,

    //    -1.0f, -1.0f,  1.0f,
    //    -1.0f, -1.0f, -1.0f,
    //    -1.0f,  1.0f, -1.0f,
    //    -1.0f,  1.0f, -1.0f,
    //    -1.0f,  1.0f,  1.0f,
    //    -1.0f, -1.0f,  1.0f,

    //     1.0f, -1.0f, -1.0f,
    //     1.0f, -1.0f,  1.0f,
    //     1.0f,  1.0f,  1.0f,
    //     1.0f,  1.0f,  1.0f,
    //     1.0f,  1.0f, -1.0f,
    //     1.0f, -1.0f, -1.0f,

    //    -1.0f, -1.0f,  1.0f,
    //    -1.0f,  1.0f,  1.0f,
    //     1.0f,  1.0f,  1.0f,
    //     1.0f,  1.0f,  1.0f,
    //     1.0f, -1.0f,  1.0f,
    //    -1.0f, -1.0f,  1.0f,

    //    -1.0f,  1.0f, -1.0f,
    //     1.0f,  1.0f, -1.0f,
    //     1.0f,  1.0f,  1.0f,
    //     1.0f,  1.0f,  1.0f,
    //    -1.0f,  1.0f,  1.0f,
    //    -1.0f,  1.0f, -1.0f,

    //    -1.0f, -1.0f, -1.0f,
    //    -1.0f, -1.0f,  1.0f,
    //     1.0f, -1.0f, -1.0f,
    //     1.0f, -1.0f, -1.0f,
    //    -1.0f, -1.0f,  1.0f,
    //     1.0f, -1.0f,  1.0f
    //};
    //// first, configure the cube's VAO (and VBO)
    //glGenVertexArrays(1, &m_SkyBoxVAO);
    //glGenBuffers(1, &m_SkyBoxVBO);

    //glBindBuffer(GL_ARRAY_BUFFER, m_SkyBoxVBO);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    //glBindVertexArray(m_SkyBoxVAO);

    //// position attribute
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    //glEnableVertexAttribArray(0);

    //std::vector<std::string> faces
    //{
    //    "../skybox/right.jpg",
    //    "../skybox/left.jpg",
    //    "../skybox/top.jpg",
    //    "../skybox/bottom.jpg",
    //    "../skybox/front.jpg",
    //    "../skybox/back.jpg"
    //};
    //m_CubeMapTexID = LoadCubemap(faces);
}


// Draw skeleton and model if opted for
void Renderer::Draw(std::vector<Object*>& objects, 
    int SCREEN_WIDTH, int SCREEN_HEIGHT)
{
    m_ViewMat = m_Camera->GetViewMatrix();
    m_ViewPos = m_Camera->m_Position;

    glEnable(GL_DEPTH_TEST);
 
    // Set BG color and clear buffers
    //glClearColor(m_BgColor.x, m_BgColor.y, m_BgColor.z, 1.0f);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
    //m_SkyBoxShader->Use()

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


    // ---- DEFERRED SHADING ---
    //  
    // PASS 1 - G-BUFFER PASS

    // Bind this FBO first. So that all output is to this 
    FBOForDefShading.Bind();

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);


    // Load data of all objects into the G-Buffers
    for (auto obj : objects) {

        ModelComp* modelComp = obj->GetComponent<ModelComp*>();

        // If we are to use the textures
        if (modelComp->GetModel()->m_UseTextures) {

            // Use the shader that takes in textures
            m_DefShaderGBufTex->Use();
            // TODO: Set proj just once
            m_DefShaderGBufTex->SetMat4("proj", m_ProjMat);
            m_DefShaderGBufTex->SetMat4("view", m_ViewMat);
            m_DefShaderGBufTex->SetMat4("model",
                obj->GetComponent<Transform*>()->GetWorldTransform());

            // Draw the model with this shader. Txtures are bound within the draw call
            obj->GetComponent<ModelComp*>()->Draw(*m_DefShaderGBufTex);
        }
        else {
            // Use material component parameters
            m_DefShaderGBuffer->Use();
            m_DefShaderGBuffer->SetMat4("proj", m_ProjMat);
            m_DefShaderGBuffer->SetMat4("view", m_ViewMat);
            m_DefShaderGBuffer->SetMat4("model",
                obj->GetComponent<Transform*>()->GetWorldTransform());

            // Set material properties
            auto material = obj->GetComponent<Material*>();
            if (material) {
                m_DefShaderGBuffer->SetVec3("material.kD", material->m_Albedo);
                m_DefShaderGBuffer->SetFloat("material.metalness", material->m_Metalness);
                m_DefShaderGBuffer->SetFloat("material.alpha", material->m_Roughness);
            }
            // Draw the model with this shader
            obj->GetComponent<ModelComp*>()->Draw(*m_DefShaderGBuffer);
        }
    }

    // PASS 2 - LIGHTING PASS

    // Now output is to the screen
    FBOForDefShading.Unbind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_DefShaderLighting->Use();

    // Bind all G-Buffer textures
    // TODO: Replace with i < numColorAttachments or sth
    for (int i = 0; i < 4; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, FBOForDefShading.m_GBuffers[i]);
    }

    // Set vars for the global light source
    m_DefShaderLighting->SetVec3("globalLight.position", m_GlobalLight.m_Position);
    m_DefShaderLighting->SetVec3("globalLight.color", m_GlobalLight.m_Color);
    m_DefShaderLighting->SetVec3("camPos", m_ViewPos);

    m_QuadDefShadingOutput.BindVAO();
    glDrawArrays(GL_TRIANGLES, 0, 6);


    // MULTIPLE LOCAL LIGHTS PASS

    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    m_MultLocalLightsShader->Use();
    m_MultLocalLightsShader->SetVec3("camPos", m_ViewPos);
    m_MultLocalLightsShader->SetMat4("view", m_ViewMat);
    m_MultLocalLightsShader->SetMat4("proj", m_ProjMat);

    // Bind all G-Buffer textures
    for (int i = 0; i < 4; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, FBOForDefShading.m_GBuffers[i]);
    }

    for (const auto& localLight : m_LocalLights) {
        m_MultLocalLightsShader->SetVec3("localLight.position", localLight.m_Position);
        m_MultLocalLightsShader->SetVec3("localLight.color", localLight.m_Color);
        m_MultLocalLightsShader->SetFloat("localLight.r", localLight.m_Range);

        // Create transformation for this light sphere
        m_ModelMat = glm::translate(glm::mat4(1.0f), localLight.m_Position);
        m_ModelMat = glm::scale(m_ModelMat, glm::vec3(localLight.m_Range, 
            localLight.m_Range, localLight.m_Range));
        m_MultLocalLightsShader->SetMat4("model", m_ModelMat);

        m_SphereMesh.BindVAO();
        glDrawElements(GL_TRIANGLE_STRIP, 
            m_SphereMesh.GetIndexCount(), GL_UNSIGNED_INT, 0);
    }

    // Remember to disable blending before rendering next frame
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);

    // ------- FORWARD SHADING -------
    //// Draw the models
    //m_ModelShaderPBR->Use();
    //m_ModelShaderPBR->SetMat4("proj", m_ProjMat);
    //m_ModelShaderPBR->SetMat4("view", m_ViewMat);

    //SetModelShaderVars();

    //// Draw the models/meshes
    //for (auto obj : objects) {
    //    m_ModelShaderPBR->SetMat4("model", 
    //        obj->GetComponent<Transform*>()->GetWorldTransform());

    //    // Set material properties
    //    auto material = obj->GetComponent<Material*>();
    //    if (material) {
    //        m_ModelShaderPBR->SetVec3("material.albedo", material->m_Albedo);
    //        m_ModelShaderPBR->SetFloat("material.metallic", material->m_Metallic);
    //        m_ModelShaderPBR->SetFloat("material.roughness", material->m_Roughness);
    //        m_ModelShaderPBR->SetFloat("material.ao", material->m_AO);
    //    }

    //    // Draw call
    //    obj->GetComponent<ModelComp*>()->Draw(*m_ModelShaderPBR);
    //}


    // Draw the Bounding Volumes of Colliders

    SetupLineShaderVars();

    for (auto obj : objects) {
        auto col = obj->GetComponent<Collider*>();
        col->Draw(m_LineShader);
    }
}

void Renderer::Deserialize(std::string path) {
    m_LocalLights.clear();
    std::ifstream sceneFile(path);
    nlohmann::json allData = nlohmann::json::parse(sceneFile);
    auto& data = allData["Lights"];
    
    m_GlobalLight.m_Position = glm::vec3(data["Global_Light"]["Position"][0],
        data["Global_Light"]["Position"][1], data["Global_Light"]["Position"][2]);

    m_GlobalLight.m_Color = glm::vec3(data["Global_Light"]["Color"][0],
        data["Global_Light"]["Color"][1], data["Global_Light"]["Color"][2]);

    m_GlobalLight.m_Range = FLT_MAX;

    auto localLightsData = data["Local_Lights"];

    // begin + 1 since first one was the sun - rest all local lights
    for (auto it = localLightsData.begin(); it != localLightsData.end(); it++) {
        auto lightObj = *it;
        Light light;

        light.m_Position = glm::vec3(lightObj["Position"][0],
            lightObj["Position"][1], lightObj["Position"][2]);
        
        light.m_Color = glm::vec3(lightObj["Color"][0],
            lightObj["Color"][1], lightObj["Color"][2]);

        light.m_Range = lightObj["Range"];

        m_LocalLights.push_back(light);
    }

    std::cout << "Configured";
    // Initialize all objects and their components.
    //p_ObjManager->Initialize();

}

nlohmann::json::value_type Renderer::Serialize()
{
    nlohmann::json lightsData;

    // Save global light / sun
    nlohmann::json currSunData;
    currSunData["Position"] = nlohmann::json::array({
            m_GlobalLight.m_Position.x, m_GlobalLight.m_Position.y, m_GlobalLight.m_Position.z
        });

    currSunData["Color"] = nlohmann::json::array({
            m_GlobalLight.m_Color.x, m_GlobalLight.m_Color.y, m_GlobalLight.m_Color.z
        });

    lightsData["Global_Light"] = currSunData;

    nlohmann::json localLightsData;

    // For the local lights
    for (int i = 0; i < m_LocalLights.size(); i++) {
        
        nlohmann::json currLightData;
        currLightData["Position"] = nlohmann::json::array ({ 
                m_LocalLights[i].m_Position.x, 
                m_LocalLights[i].m_Position.y,
                m_LocalLights[i].m_Position.z
            });

        currLightData["Color"] = nlohmann::json::array({
                m_LocalLights[i].m_Color.x,
                m_LocalLights[i].m_Color.y,
                m_LocalLights[i].m_Color.z
            });

        currLightData["Range"] = m_LocalLights[i].m_Range;

        localLightsData["Light_" + std::to_string(i)] = currLightData;
    }

    lightsData["Local_Lights"] = localLightsData;

    return lightsData;
}

// Pass variables to the line shader
void Renderer::SetupLineShaderVars() {
    // Set variables for line drawing shader
    m_LineShader->Use();
    m_LineShader->SetMat4("view", m_ViewMat);
    m_LineShader->SetMat4("model", glm::mat4(1.0f));
    m_LineShader->SetMat4("proj", m_ProjMat);
}

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
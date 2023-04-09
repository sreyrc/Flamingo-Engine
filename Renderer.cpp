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

    // Creating shaders
    // TODO: Use smart ptrs
    m_ModelShader = new Shader("ModelShader.vert", "ModelShader.frag");
    m_ModelShaderPBR = new Shader("ModelShaderPBR.vert", "ModelShaderPBR.frag");
    m_DefShaderGBuffer = new Shader("DefShaderGBufPass.vert", "DefShaderGBufPass.frag");
    m_DefShaderGBufTex = new Shader("DefShaderGBufTex.vert", "DefShaderGBufTex.frag");
    m_DefShaderLighting = new Shader("DefShaderLightingPass.vert", "DefShaderLightingPass.frag");
    m_LineShader = new Shader("LineShader.vert", "LineShader.frag"); 
    m_MultLocalLightsShader = new Shader("MultLocalLightsShader.vert", "MultLocalLightsShader.frag");
    m_ShadowShader = new Shader("Shadow.vert", "Shadow.frag");
    m_SkyDomeShader = new Shader("SkyDome.vert", "SkyDome.frag");
    m_AmbientOcclusionShader = new Shader("AmbientBufferPass.vert", "AmbientBufferPass.frag");
    
    m_HorizontalBlur = new ComputeShader("HorizontalBlur.comp");
    m_VerticalBlur = new ComputeShader("VerticalBlur.comp");

    m_HorizontalAOBlur = new ComputeShader("HorizontalBlurAO.comp");

    // TODO: Abstract this away
    glUniform1i(glGetUniformLocation(m_HorizontalAOBlur->GetID(), "g_Position"), 0);
    glUniform1i(glGetUniformLocation(m_HorizontalAOBlur->GetID(), "g_Normal"), 1);

    m_HorizontalAOBlur = new ComputeShader("HorizontalAOBlur.comp");

    m_DefShaderGBuffer->Use();
    m_DefShaderGBuffer->SetInt("skyBoxTexture", 0);

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
    m_DefShaderLighting->SetInt("shadowMap", 4);
    m_DefShaderLighting->SetInt("irradianceMap", 5);
    m_DefShaderLighting->SetInt("skyDomeMap", 6);
    m_DefShaderLighting->SetFloat("width", (float)SCREEN_WIDTH);
    m_DefShaderLighting->SetFloat("height", (float)SCREEN_HEIGHT);
    m_DefShaderLighting->Unuse();

    // Set samplers and other vars
    m_MultLocalLightsShader->Use();
    m_MultLocalLightsShader->SetInt("g_Position", 0);
    m_MultLocalLightsShader->SetInt("g_Normal", 1);
    m_MultLocalLightsShader->SetInt("g_Diffuse", 2);
    m_MultLocalLightsShader->SetInt("g_RoughMetal", 3);
    m_MultLocalLightsShader->SetFloat("width", (float)SCREEN_WIDTH);
    m_MultLocalLightsShader->SetFloat("height", (float)SCREEN_HEIGHT);
    m_MultLocalLightsShader->Unuse();

    m_AmbientOcclusionShader->Use();
    m_AmbientOcclusionShader->SetInt("g_Position", 0);
    m_AmbientOcclusionShader->SetInt("g_Normal", 1);
    m_AmbientOcclusionShader->SetInt("width", SCREEN_WIDTH);
    m_AmbientOcclusionShader->SetInt("height", SCREEN_HEIGHT);

    m_ShadowProj = glm::perspective(
        glm::radians(60.0f), 1.0f, 0.1f, 1000.0f);

    m_BMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f));
    m_BMat = glm::scale(m_BMat, glm::vec3(0.5f));

    m_ShadowShader->Use();
    m_ShadowShader->SetMat4("proj", m_ShadowProj);
    m_ShadowShader->Unuse();

    m_SkyDomeShader->Use();
    m_SkyDomeShader->SetInt("tex", 0);

    // Set up camera
    m_Camera = cam;

    // Initialize matrices
    m_ModelMat = glm::mat4(1.0f);
    m_ViewMat = glm::mat4(1.0f);

    m_ModelPos = glm::vec3(0.0f);

    m_RotMat = glm::mat4(1.0f);

    //m_SphereMesh = new SphereMesh();

    m_SkyBoxVAO = 0; m_SkyBoxVBO = 0;

    // Create an FBO with 4 color attachments/buffers
    m_FBOForDefShading.CreateFBO(SCREEN_WIDTH, SCREEN_HEIGHT, 4);

    // For storing depth values. Needed for shadow-mapping
    m_FBOLightDepth.CreateFBO(1024, 1024, 1);

    // For storing depth values. Needed for shadow-mapping
    m_FBOLightDepthBlurred.CreateFBO(1024, 1024, 1);

    // For storing ambient occlusion values
    m_FBOAmbientOcclusion.CreateFBO(SCREEN_WIDTH, SCREEN_HEIGHT, 1);

    glGenBuffers(1, &m_Block); // Generates block
    glGenBuffers(1, &m_Block1); // Generates block

    HammersleyBlockSetup();
}


// Draw the scene
void Renderer::Draw(std::vector<Object*>& objects, ResourceManager* p_ResourceManager,
    int SCREEN_WIDTH, int SCREEN_HEIGHT)
{

    // Set up transforms
    m_ProjMat = glm::perspective(glm::radians(m_Camera->m_Zoom),
        (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 1000.0f);

    m_ViewMat = m_Camera->GetViewMatrix();
    m_ViewPos = m_Camera->m_Position;

    m_ShadowView = glm::lookAt(m_GlobalLight.m_Position,
        glm::vec3(1), glm::vec3(0, 1, 0));

    //glDisable(GL_DEPTH_TEST);    
    glEnable(GL_DEPTH_TEST);
    
    
    // ---- SHADOW PASS ----
    //
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_FRONT);

    m_FBOLightDepth.Bind();
    
    glViewport(0, 0, 1024, 1024);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    float lightDistance =  glm::length(m_GlobalLight.m_Position - glm::vec3(1));
    float minDepth = lightDistance - m_CenterRadius;
    float maxDepth = lightDistance + m_CenterRadius;

    m_ShadowShader->Use();
    m_ShadowShader->SetFloat("minDepth", minDepth);
    m_ShadowShader->SetFloat("maxDepth", maxDepth);
    m_ShadowShader->SetMat4("view", m_ShadowView);

    // Draw objects from the global light's POV
    for (auto obj : objects) {
        m_ShadowShader->SetMat4("model", obj->GetComponent<Transform*>()->GetWorldTransform());
        obj->GetComponent<ModelComp*>()->Draw(*m_ShadowShader);
    }

    // Unbind the FBO and unuse the shader
    m_FBOLightDepth.Unbind();
    m_ShadowShader->Unuse();


    glDisable(GL_CULL_FACE);


    // --- GAUSSIAN BLUR USING CONVOLUTION BLUR FILTER ---

    // Fill in and send weights
    int kernelSize = 2 * m_KernelHalfWidth + 1;
    float sum = 0;
    float s = m_KernelHalfWidth / 2.0f;
    if (m_KernelHalfWidth == 0) s = 1;
    std::vector<float> weights(kernelSize);
    for (int i = 0; i < kernelSize; i++) {
        weights[i] = pow(glm::e<float>(), -0.5f * pow((i - m_KernelHalfWidth) / s, 2));
        sum += weights[i];
    }

    for (int i = 0; i < kernelSize; i++) {
        weights[i] = weights[i] / sum;
    }

    // Horizontal Blur
    m_HorizontalBlur->Use();
    
    // TODO: Have all this abstracted
    // 
    // Input image
    unsigned imageUnit = 0 ; // Perhaps 0 for input image and 1 for output image
    unsigned loc = glGetUniformLocation(m_HorizontalBlur->GetID(), "src"); // Perhaps “src” and “dst”.
    glBindImageTexture(imageUnit, m_FBOLightDepth.m_GBuffers[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glUniform1i(loc, imageUnit);

    // Output image
    imageUnit = 1; // Perhaps 0 for input image and 1 for output image
    loc = glGetUniformLocation(m_HorizontalBlur->GetID(), "dst"); // Perhaps “src” and “dst”.
    glBindImageTexture(imageUnit, m_FBOLightDepthBlurred.m_GBuffers[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glUniform1i(loc, imageUnit);
    // Change GL_READ_ONLY to GL_WRITE_ONLY for output image
    // Note: GL_RGBA32F means 4 channels (RGBA) of 32 bit floats.


    unsigned bindpoint = 0; // Start at zero, increment for other blocks
    loc = glGetUniformBlockIndex(m_HorizontalBlur->GetID(), "blurKernel");
    glUniformBlockBinding(m_HorizontalBlur->GetID(), loc, bindpoint);
    glBindBuffer(GL_UNIFORM_BUFFER, m_Block);
    glBindBufferBase(GL_UNIFORM_BUFFER, bindpoint, m_Block);
    glBufferData(GL_UNIFORM_BUFFER, weights.size() * sizeof(float), &weights[0], GL_STATIC_DRAW);

    loc = glGetUniformLocation(m_HorizontalBlur->GetID(), "w");
    glUniform1i(loc, m_KernelHalfWidth);

    glDispatchCompute(1024/128, 1024, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    m_HorizontalBlur->Unuse();


    // Vertical blur
    m_VerticalBlur->Use();

    // TODO: Have all this abstracted
    // 
    // Input image
    imageUnit = 0; // 0 for input image
    loc = glGetUniformLocation(m_VerticalBlur->GetID(), "src"); // Perhaps “src” and “dst”.
    glBindImageTexture(imageUnit, m_FBOLightDepthBlurred.m_GBuffers[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glUniform1i(loc, imageUnit);

    // Output image
    imageUnit = 1; // 1 for output image
    loc = glGetUniformLocation(m_VerticalBlur->GetID(), "dst"); // Perhaps “src” and “dst”.
    glBindImageTexture(imageUnit, m_FBOLightDepthBlurred.m_GBuffers[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glUniform1i(loc, imageUnit);
    // Change GL_READ_ONLY to GL_WRITE_ONLY for output image
    // Note: GL_RGBA32F means 4 channels (RGBA) of 32 bit floats.

    bindpoint = 0; // Start at zero, increment for other blocks
    loc = glGetUniformBlockIndex(m_VerticalBlur->GetID(), "blurKernel");
    glUniformBlockBinding(m_VerticalBlur->GetID(), loc, bindpoint);
    glBindBuffer(GL_UNIFORM_BUFFER, m_Block);
    glBindBufferBase(GL_UNIFORM_BUFFER, bindpoint, m_Block);
    glBufferData(GL_UNIFORM_BUFFER, weights.size() * sizeof(float), &weights[0], GL_STATIC_DRAW);
        
    loc = glGetUniformLocation(m_VerticalBlur->GetID(), "w");
    glUniform1i(loc, m_KernelHalfWidth);

    glDispatchCompute(1024, 1024/128, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    m_VerticalBlur->Unuse();


    // ---- DEFERRED SHADING ----
    //  
    // PASS 1 - G-BUFFER PASS

    // Bind this FBO first. So that all output is to this 
    m_FBOForDefShading.Bind();

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
            m_DefShaderGBuffer->SetInt("isSkyDome", false);

            // Set material properties
            auto material = obj->GetComponent<Material*>();
            if (material) {
                m_DefShaderGBuffer->SetVec3("material.kD", material->m_Albedo);
                m_DefShaderGBuffer->SetFloat("material.metalness", material->m_Metalness);
                float roughness = sqrt(2.0/(material->m_Shininess + 2.0));
                m_DefShaderGBuffer->SetFloat("material.alpha", roughness);
            }
            // Draw the model with this shader
            obj->GetComponent<ModelComp*>()->Draw(*m_DefShaderGBuffer);
        }
    }

    // Skydome - Onlyif IBL is on

    // TODO: Make this a member var
    if (m_IBLon) {

        m_HDRTexSet = p_ResourceManager->GetHDRTextureSet("Newport_Loft_Ref");

        m_DefShaderGBuffer->Use();

        glm::mat4 skyModelTr = glm::mat4(1.0f);
        skyModelTr = glm::scale(skyModelTr, glm::vec3(100, 100, 100));
        m_DefShaderGBuffer->SetMat4("model", skyModelTr);
        glm::mat4 skyView = glm::mat4(glm::mat3(m_ViewMat));
        m_DefShaderGBuffer->SetMat4("view", skyView);
        m_DefShaderGBuffer->SetMat4("proj", m_ProjMat);
        m_DefShaderGBuffer->SetFloat("expControl", m_ExposureControl);
        m_DefShaderGBuffer->SetInt("isSkyDome", true);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_HDRTexSet->m_SkyTexture->GetID());

        m_SkyDomeMesh.BindVAO();
        m_SkyDomeMesh.Draw();

        m_DefShaderGBuffer->Unuse();
    }


    // --- IN BETWEEN: AMBIENT OCCLUSION PASS ---

    // --- AO BUFFER PASS ---

    m_FBOAmbientOcclusion.Bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_AmbientOcclusionShader->Use();
    
    // Set Shader vars
    m_AmbientOcclusionShader->SetVec3("eyePos", m_ViewPos);
    m_AmbientOcclusionShader->SetFloat("s", m_s);
    m_AmbientOcclusionShader->SetFloat("k", m_k);
    m_AmbientOcclusionShader->SetFloat("R", m_R);

    // World Pos buffer
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_FBOForDefShading.m_GBuffers[0]);

    // Normal Buffer
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_FBOForDefShading.m_GBuffers[1]);

    m_Quad.BindVAO();
    glDrawArrays(GL_TRIANGLES, 0, 6);

    m_FBOAmbientOcclusion.Unbind();
    m_AmbientOcclusionShader->Unuse();

    // --- AO MAP BLUR PASS --

    m_HorizontalAOBlur->Use();

    imageUnit = 0; // 0 for input image
    loc = glGetUniformLocation(m_HorizontalAOBlur->GetID(), "src"); // Perhaps “src” and “dst”.
    glBindImageTexture(imageUnit, m_FBOAmbientOcclusion.m_GBuffers[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glUniform1i(loc, imageUnit);

    // Output image
    imageUnit = 1; // 1 for output image
    loc = glGetUniformLocation(m_HorizontalAOBlur->GetID(), "dst"); // Perhaps “src” and “dst”.
    glBindImageTexture(imageUnit, m_FBOAmbientOcclusion.m_GBuffers[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glUniform1i(loc, imageUnit);

    bindpoint = 0; // Start at zero, increment for other blocks
    loc = glGetUniformBlockIndex(m_HorizontalAOBlur->GetID(), "blurKernel");
    glUniformBlockBinding(m_HorizontalAOBlur->GetID(), loc, bindpoint);
    glBindBuffer(GL_UNIFORM_BUFFER, m_Block);
    glBindBufferBase(GL_UNIFORM_BUFFER, bindpoint, m_Block);
    glBufferData(GL_UNIFORM_BUFFER, weights.size() * sizeof(float), &weights[0], GL_STATIC_DRAW);

    loc = glGetUniformLocation(m_HorizontalAOBlur->GetID(), "w");
    glUniform1i(loc, m_KernelHalfWidth);

    loc = glGetUniformLocation(m_HorizontalAOBlur->GetID(), "eyePos");
    glUniform3f(loc, m_ViewPos.x, m_ViewPos.y, m_ViewPos.z);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_FBOForDefShading.m_GBuffers[0]);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_FBOForDefShading.m_GBuffers[1]);

    glDispatchCompute(1024/128, 1024, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    m_VerticalBlur->Unuse();


    // PASS 2 - LIGHTING PASS

    // Now output is to the screen
    //m_FBOForDefShading.Unbind();


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_DefShaderLighting->Use();

    // Bind all G-Buffer textures
    // TODO: Replace with i < numColorAttachments or sth
    for (int i = 0; i < 4; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_FBOForDefShading.m_GBuffers[i]);
    }

    // Set vars for the global light source
    m_DefShaderLighting->SetVec3("globalLight.position", m_GlobalLight.m_Position);
    m_DefShaderLighting->SetVec3("globalLight.color", m_GlobalLight.m_Color);
    m_DefShaderLighting->SetVec3("viewPos", m_ViewPos);
    m_DefShaderLighting->SetFloat("minDepth", minDepth);
    m_DefShaderLighting->SetFloat("maxDepth", maxDepth);
    m_DefShaderLighting->SetInt("iblOn", m_IBLon);

    // Shadow matrix
    glm::mat4 shadowMat = m_BMat * m_ShadowProj * m_ShadowView;
    m_DefShaderLighting->SetMat4("shadowMat", shadowMat);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, m_FBOLightDepthBlurred.m_GBuffers[0]);

    if (m_IBLon) {
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, m_HDRTexSet->m_IrradianceMap->GetID());

        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, m_HDRTexSet->m_SkyTexture->GetID());

        bindpoint = 1;
        glBindBuffer(GL_UNIFORM_BUFFER, m_HammersleyBlock);
        glBindBufferBase(GL_UNIFORM_BUFFER, bindpoint, m_HammersleyBlock);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(float) + sizeof(float) * 
            m_HammersleyData.values.size(), &m_HammersleyData, GL_STATIC_DRAW);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float), sizeof(float) * m_HammersleyData.values.size(),
            &(m_HammersleyData.values[0]));

        int loc = glGetUniformBlockIndex(m_DefShaderLighting->GetID(), "HammersleyBlock");
        glUniformBlockBinding(m_DefShaderLighting->GetID(), loc, bindpoint);

        m_DefShaderLighting->SetFloat("expControl", m_ExposureControl);
    }

    m_Quad.BindVAO();
    glDrawArrays(GL_TRIANGLES, 0, 6);


    //// --- MULTIPLE LOCAL LIGHTS PASS --- 

    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glEnable(GL_CULL_FACE);
    //glDisable(GL_DEPTH_TEST);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_ONE, GL_ONE);

    //m_MultLocalLightsShader->Use();
    //m_MultLocalLightsShader->SetVec3("viewPos", m_ViewPos);
    //m_MultLocalLightsShader->SetMat4("view", m_ViewMat);
    //m_MultLocalLightsShader->SetMat4("proj", m_ProjMat);

    //// Bind all G-Buffer textures
    //for (int i = 0; i < 4; i++) {
    //    glActiveTexture(GL_TEXTURE0 + i);
    //    glBindTexture(GL_TEXTURE_2D, m_FBOForDefShading.m_GBuffers[i]);
    //}

    //for (const auto& localLight : m_LocalLights) {
    //    m_MultLocalLightsShader->SetVec3("localLight.position", localLight.m_Position);
    //    m_MultLocalLightsShader->SetVec3("localLight.color", localLight.m_Color);
    //    m_MultLocalLightsShader->SetFloat("localLight.r", localLight.m_Range);

    //    // Create transformation for this light sphere
    //    m_ModelMat = glm::translate(glm::mat4(1.0f), localLight.m_Position);
    //    m_ModelMat = glm::scale(m_ModelMat, glm::vec3(localLight.m_Range, 
    //        localLight.m_Range, localLight.m_Range));
    //    m_MultLocalLightsShader->SetMat4("model", m_ModelMat);

    //    m_SphereMesh.Draw();
    //}

    //// Remember to disable blending before rendering next frame
    //glDisable(GL_BLEND);
    //glDisable(GL_CULL_FACE);

    //// ------- FORWARD SHADING -------
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

    // Enable colliders later
    if (m_DebugCollidersOn) {
        for (auto obj : objects) {
            auto col = obj->GetComponent<Collider*>();
            if (col) col->Draw(m_LineShader);
        }
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

void Renderer::HammersleyBlockSetup()
{
    m_HammersleyData.N = 40; // N=20 ... 40 or whatever
    m_HammersleyData.values.resize(m_HammersleyData.N * 2);
    //m_HammersleyData.values = new float[m_HammersleyData.N * 2];

    int kk;
    int pos = 0;
    for (int k = 0; k < m_HammersleyData.N; k++) {
        kk = k;
        float u = 0.0f;
        for (float p = 0.5f; kk; p *= 0.5f, kk >>= 1)
        {
            if (kk & 1)
                u += p;
        }
        float v = (k + 0.5) / m_HammersleyData.N;
        m_HammersleyData.values[pos++] = u;
        m_HammersleyData.values[pos++] = v;
    }

    glGenBuffers(1, &m_HammersleyBlock);
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
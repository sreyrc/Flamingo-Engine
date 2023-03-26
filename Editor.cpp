#include "Editor.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"

#include <glm/glm.hpp>

#include <random>

Editor::Editor(GLFWwindow* window) {

    // Dear Imgui initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsClassic();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void Editor::Update( 
    Camera* p_Camera,
    ObjectManager* p_ObjectManager, 
    ObjectFactory* p_ObjectFactory,
    SceneManager* p_SceneManager,
    ResourceManager* p_ResourceManager,
    CollisionWorld* p_CollisionWorld,
    Renderer* p_Renderer) {

    // Imgui new frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Camera 
    ImGui::Begin("Camera Instructions"); {
        ImGui::Text("Use WASD to move around");
        ImGui::Text("Use the Arrow Keys to rotate the camera");
        ImGui::Text("Press I to zoom in and O to zoom out");
        ImGui::Text("Press R to reset to camera defaults");
        ImGui::SliderFloat("Move Speed", &p_Camera->m_MovementSpeed, 2.5, 20.0f);
        ImGui::SliderFloat("Rotation Speed", &p_Camera->m_RotationSpeed, 50.0f, 100.0f);
    }
    ImGui::End();

    ImGui::Begin("G-Buffers"); {
        int vecSize = p_Renderer->m_FBOForDefShading.m_GBuffers.size();
        for (int i = 0; i < vecSize; i++) {
            ImGui::Image((ImTextureID)p_Renderer->m_FBOForDefShading.m_GBuffers[i],
                ImVec2(384, 216), ImVec2(0, 1), ImVec2(1, 0));
        }
    }
    ImGui::End();

    //// Scene lights
    //ImGui::Begin("Lights");
    //ImGui::End();

    // Object editor
    ImGui::Begin("Scene Editor"); {
        ImGui::Text("Scenes: ");

        if (ImGui::BeginListBox("##SceneList")) {
            for (auto& sceneName : p_SceneManager->m_SceneNames) {
                const bool is_selected =
                    (p_SceneManager->m_SelectedScene == sceneName);

                if (ImGui::Selectable(sceneName.c_str(), is_selected)) {
                    p_SceneManager->m_SelectedScene = sceneName;
                }

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndListBox();
        }

        if (ImGui::Button("Load Scene")) {
            p_SceneManager->LoadScene(p_SceneManager->m_SelectedScene,
                p_ObjectManager, p_ObjectFactory,
                p_ResourceManager, p_CollisionWorld, p_Renderer);
        }

        ImGui::Dummy(ImVec2(20, 20));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(20, 20));

        // Shadow settings
        ImGui::Text("Shadows:");
        ImGui::SliderInt("Kernel Half-Width", &p_Renderer->m_KernelHalfWidth, 0, 50);
        ImGui::DragFloat("Center Half-Radius", &p_Renderer->m_CenterRadius, 5, 20);

        ImGui::Dummy(ImVec2(20, 20));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(20, 20));

        // Lights
        ImGui::Text("Lights:");
        ImGui::Dummy(ImVec2(20, 20));

        // Global light
        ImGui::DragFloat3("Global Light Position",
            &p_Renderer->m_GlobalLight.m_Position.x);
        ImGui::DragFloat3("Global Light Color",
            &p_Renderer->m_GlobalLight.m_Color.x);

        ImGui::Dummy(ImVec2(20, 20));

        ImGui::Text("Local Lights:");
        if (ImGui::BeginTabBar("#LightTabs")) {
            // Local lights
            int numLights = static_cast<int>(p_Renderer->m_LocalLights.size());
            for (int i = 0; i < numLights; i++) {

                std::string lightNumText = "Light " + std::to_string(i);

                if (ImGui::BeginTabItem(lightNumText.c_str())) {
                    std::string lightNumText = "Light " + std::to_string(i);
                    ImGui::Text(lightNumText.c_str());
                    std::string posLabel = "Position ##" + lightNumText;
                    std::string colorLabel = "Color ##" + lightNumText;
                    std::string rangeLabel = "Range ##" + lightNumText;
                    ImGui::DragFloat3(posLabel.c_str(),
                        &p_Renderer->m_LocalLights[i].m_Position.x);
                    ImGui::DragFloat3(colorLabel.c_str(),
                        &p_Renderer->m_LocalLights[i].m_Color.x);
                    ImGui::DragFloat(rangeLabel.c_str(),
                        &p_Renderer->m_LocalLights[i].m_Range, 1.0f, 200.0f);

                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }

        
        // TODO: This might be temporary
        if (ImGui::Button("Add a LOT of lights")) {

            unsigned seedColor = std::chrono::steady_clock::now().time_since_epoch().count();
            std::default_random_engine eColor(seedColor);

            unsigned seedRange = std::chrono::steady_clock::now().time_since_epoch().count();
            std::default_random_engine eRange(seedRange);

            glm::vec3 pos = glm::vec3(0), color = glm::vec3(50);
            float range = 5.0f;
            int k = 0;
            for (int i = -40; i < 40; i += 4) {
                for (int j = -40; j < 40; j += 4) {
                    pos = glm::vec3(i, 3.0f, j);
                    // TODO: Assign random values here
                    range = 7.0f;
                    color = glm::vec3(20 + (eColor() % 50),
                        20 + (eColor() % 50), 20 + (eColor() % 50));
                    //range = 2;
                    p_Renderer->AddLight(pos, color, range);
                }
            }
        }
        
        ImGui::Dummy(ImVec2(20, 20));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(20, 20));

        ImGui::Text("Objects:");

        auto& objects = p_ObjectManager->m_ObjectList;
        
        if (ImGui::BeginTabBar("#Tabs")) {

            for (int i = 0; i < objects.size(); i++) {

                //std::string str = "Obj " + std::to_string(i);
                if (ImGui::BeginTabItem(objects[i]->GetName().c_str())) {
                    
                    ImGui::InputText("Object name", objects[i]->m_ObjectNameBuff,
                        sizeof(objects[i]->m_ObjectNameBuff));

                     if (ImGui::Button("Set Name")) {
                         if (std::string(objects[i]->m_ObjectNameBuff) != "") {
                             objects[i]->SetName(objects[i]->m_ObjectNameBuff);
                         }
                     }

                    // Transform component
                    ImGui::Dummy(ImVec2(20, 20));
                    ImGui::Text("Transform: ");
                    Transform* tr = objects[i]->GetComponent<Transform*>();
                    if (tr) {
                        ImGui::DragFloat3("Position", &tr->m_Position.x, -10.0f, 10.0f);
                        ImGui::DragFloat3("Scale", &tr->m_Scale.x, 0.1f, 2.0f);
                        ImGui::SliderFloat3("Rotation", &tr->m_RotationEuler.x, -180.0f, 180.0f);
                    }

                    ImGui::Dummy(ImVec2(20, 20));

                    // Model component
                    ImGui::Text("Model: ");
                    ModelComp* md = objects[i]->GetComponent<ModelComp*>();
                    if (md) {
                        if (ImGui::BeginListBox("##ModelList")) {
                            for (auto& modelName : p_ResourceManager->m_ModelNames) {
                                const bool is_selected =
                                    (md->m_ModelSelected == modelName);

                                if (ImGui::Selectable(modelName.c_str(), is_selected)) {
                                    md->m_ModelSelected = modelName;
                                }

                                if (is_selected)
                                    ImGui::SetItemDefaultFocus();
                            }
                            ImGui::EndListBox();
                        }

                        if (ImGui::Button("Select Model")) {
                            md->SetModel(p_ResourceManager);
                            Collider* col = objects[i]->GetComponent<Collider*>();
                            if (col) col->Initialize();
                        }

                        ImGui::Checkbox("Use textures", 
                            &md->GetModel()->m_UseTextures);
                    }

                    ImGui::Dummy(ImVec2(20, 20));

                    // Material component
                    ImGui::Text("Material: ");
                    Material* mt = objects[i]->GetComponent<Material*>();
                    if (mt) {
                        ImGui::ColorEdit3("Albedo", &mt->m_Albedo.x);
                        ImGui::SliderFloat("Metalness", &mt->m_Metalness, 0.0f, 1.0f);
                        ImGui::SliderFloat("Roughness", &mt->m_Roughness, 0.0f, 1.0f);
                    }
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }
        ImGui::Dummy(ImVec2(50, 50));

        if (ImGui::Button("Add New Object")) {
            p_ObjectFactory->CreateObject(p_ObjectManager, p_ResourceManager, p_CollisionWorld);
        }

        // TODO: Super hacky and most probably temporary - refactor later or just remove
        if (ImGui::Button("Add a LOT of Objects")) {

            // For material properties. any value between 0 and 1 for both hence same 
            // rangome engine can be used for both
            unsigned seeObjProp = std::chrono::steady_clock::now().time_since_epoch().count();
            std::default_random_engine eObjProp(seeObjProp);

            int obj_num = 0;
            for (int i = -40; i < 40; i += 16) {
                for (int j = -40; j < 40; j += 16) {

                    Object* object = new Object("Newobj" + std::to_string(obj_num++));
                    // Create all components and add in the beginning
                    Transform* tr = new Transform();
                    ModelComp* md = new ModelComp();
                    Material* mat = new Material();

                    // Set random mat properties
                    mat->m_Albedo = glm::vec3(
                        eObjProp() / static_cast<float>(eObjProp.max()),
                        eObjProp() / static_cast<float>(eObjProp.max()),
                        eObjProp() / static_cast<float>(eObjProp.max()));
                    mat->m_Metalness = eObjProp() / static_cast<float>(eObjProp.max());
                    mat->m_Roughness = eObjProp() / static_cast<float>(eObjProp.max());

                    md->SetDefaults(p_ResourceManager);
                    //mat->SetDefaults(p_ResourceManager);

                    tr->m_Position = glm::vec3(i, 0, j);
                    tr->m_Scale = glm::vec3(1 +
                        (eObjProp() / static_cast<float>(eObjProp.max())));

                    // Add all the components
                    object->AddComponent(tr);
                    object->AddComponent(md);
                    object->AddComponent(mat);

                    object->Initialize();
                    p_ObjectManager->AddObject(object);
                }
            }
            //p_ObjectFactory->CreateObject(p_ObjectManager, p_ResourceManager);
        }
        ImGui::Dummy(ImVec2(50, 50));
        ImGui::InputText("Scene name", p_SceneManager->m_SceneNameBuf,
            sizeof(p_SceneManager->m_SceneNameBuf));

        if (ImGui::Button("Save New Scene")) {
            if (std::string(p_SceneManager->m_SceneNameBuf) != "") {
                p_SceneManager->m_SceneName = p_SceneManager->m_SceneNameBuf;
                p_SceneManager->SaveScene(p_SceneManager->m_SceneName,
                    p_ObjectManager, p_Renderer);
            }
            else {
                ImGui::Text("Please name your scene");
            }
        }

        if (ImGui::Button("Overwrite Current Scene")) {
            p_SceneManager->SaveScene(p_SceneManager->m_SelectedScene,
                p_ObjectManager, p_Renderer);
        }
        ImGui::End();
    }


    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
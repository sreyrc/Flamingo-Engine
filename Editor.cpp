#include "Editor.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"

#include <glm/glm.hpp>

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

    // Camera controls instructions
    ImGui::Begin("Camera Instructions"); {
        ImGui::Text("Use WASD to move around");
        ImGui::Text("Use the Arrow Keys to rotate the camera");
        ImGui::Text("Press I to zoom in and O to zoom out");
        ImGui::Text("Press R to reset to camera defaults");
    }
    ImGui::End();

    // Scene lights
    ImGui::Begin("Lights"); {
        int numLights = p_Renderer->m_Lights.size();
        for (int i = 0; i < numLights; i++) {

            std::string lightNumText = "Light " + std::to_string(i);
            ImGui::Text(lightNumText.c_str());
            std::string posLabel = "Position ##" + lightNumText;
            std::string colorLabel = "Color ##" + lightNumText;
            ImGui::SliderFloat3(posLabel.c_str(),
                &p_Renderer->m_Lights[i].position.x, -10.0f, 10.0f);
            ImGui::SliderFloat3(colorLabel.c_str(),
                &p_Renderer->m_Lights[i].color.x, 0.0f, 5.0f);

            ImGui::Dummy(ImVec2(20, 20));
        }

        //TODO: Add a separate section for editing renderer params
        //ImGui::SliderFloat3("Light pos", &renderer->m_LightPos.x, -100, 100);
    }
    ImGui::End();

    // Scene list. Select a scene and load selected if desired
    ImGui::Begin("Saved Scenes"); {
        if (ImGui::BeginListBox("##SceneList")); {
            for (auto& sceneName : p_SceneManager->m_SceneNames) {
                const bool is_selected = 
                    (p_SceneManager->m_SelectedScene == sceneName);

                if (ImGui::Selectable(sceneName.c_str(), is_selected)) {
                    p_SceneManager->m_SelectedScene = sceneName;
                }

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndListBox();

        if (ImGui::Button("Load Scene")) {
            p_SceneManager->LoadScene(p_SceneManager->m_SelectedScene,
                p_ObjectManager, p_ObjectFactory, 
                p_ResourceManager, p_CollisionWorld, p_Renderer);
        }
    }
    ImGui::End();


    // Object editor
    ImGui::Begin("Objects"); {

        auto& objects = p_ObjectManager->m_ObjectList;
        
        if (ImGui::BeginTabBar("#Tabs")) {
            for (int i = 0; i < objects.size(); i++) {

                std::string str = "Obj " + std::to_string(i);
                if (ImGui::BeginTabItem(objects[i]->GetName().c_str())) {
                    
                    // Transform component
                    ImGui::Dummy(ImVec2(20, 20));
                    ImGui::Text("Transform: ");
                    Transform* tr = objects[i]->GetComponent<Transform*>();
                    if (tr) {
                        ImGui::SliderFloat3("Position", &tr->m_Position.x, -10.0f, 10.0f);
                        ImGui::SliderFloat3("Scale", &tr->m_Scale.x, 0.1f, 1.0f);
                        ImGui::SliderFloat3("Rotation", &tr->m_Rotation.x, -10.0f, 10.0f);
                    }

                    ImGui::Dummy(ImVec2(20, 20));

                    // Model component
                    ImGui::Text("Model: ");
                    ModelComp* md = objects[i]->GetComponent<ModelComp*>();
                    if (md) {
                        if (ImGui::BeginListBox("##ModelList")); {
                            for (auto& modelName : p_ResourceManager->m_ModelNames) {
                                const bool is_selected =
                                    (md->m_ModelSelected == modelName);

                                if (ImGui::Selectable(modelName.c_str(), is_selected)) {
                                    md->m_ModelSelected = modelName;
                                }

                                if (is_selected)
                                    ImGui::SetItemDefaultFocus();
                            }
                        }
                        ImGui::EndListBox();

                        if (ImGui::Button("Select Model")) {
                            md->SetModel(p_ResourceManager);
                            Collider* col = objects[i]->GetComponent<Collider*>();
                            if (col) col->Initialize();
                        }
                    }

                    ImGui::Dummy(ImVec2(20, 20));

                    // Material component
                    ImGui::Text("Material: ");
                    Material* mt = objects[i]->GetComponent<Material*>();
                    if (mt) {
                        ImGui::ColorEdit3("Albedo", &mt->m_Albedo.x);
                        ImGui::SliderFloat("Metalness", &mt->m_Metallic, 0.0f, 1.0f);
                        ImGui::SliderFloat("Roughness", &mt->m_Roughness, 0.0f, 1.0f);
                        ImGui::SliderFloat("AO", &mt->m_AO, 0.0f, 1.0f);
                    }
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::Dummy(ImVec2(20, 20));
    if (ImGui::Button("Save Scene")) {
        p_SceneManager->SaveScene(p_ObjectManager, p_Renderer);
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
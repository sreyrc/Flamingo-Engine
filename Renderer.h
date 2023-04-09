#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>

#include "Camera.h"
#include "Shader.h"
#include "ComputeShader.h"
#include "SphereMesh.h"

#include "Object.h"

#include "FBO.h"
#include "QuadMesh.h"

struct HammerseleyData {
    float N;
    std::vector<float> values;
};

struct Light {
    glm::vec3 m_Position;
    glm::vec3 m_Color;
    float m_Range;

    Light() : m_Position(0), m_Color(1), m_Range(1) {}

    Light(glm::vec3& position, glm::vec3& color, float range) 
        : m_Position(position), m_Color(color), m_Range(range) {}
};

class Renderer
{
public:
	Renderer(Camera* cam, int SCREEN_WIDTH, int SCREEN_HEIGHT);

	void Draw(std::vector<Object*>& objects, ResourceManager* p_ResourceManager,
        int SCREEN_WIDTH, int SCREEN_HEIGHT);
    
    void Deserialize(std::string path);
    nlohmann::json::value_type Serialize();

    void AddLight(glm::vec3 position, glm::vec3 color, float range) {
        m_LocalLights.push_back(Light(position, color, range));
    }

    void HammersleyBlockSetup();

    float m_LineWidth;

    glm::vec3 m_ModelPos;

    // Background color
    glm::vec3 m_BgColor;

    // View position
    glm::vec3 m_ViewPos;

    // Diffuse and specular colors for the model
    glm::vec3 m_DiffuseColor;
    glm::vec3 m_SpecularColor;

    // Large global light - the sun
    Light m_GlobalLight;
    
    // All the lights in the scene;
    std::vector<Light> m_LocalLights;

    // Shaders: 
    Shader* m_ModelShader, *  m_ModelShaderPBR,
        *m_DefShaderGBuffer, * m_DefShaderLighting,
        *m_DefShaderGBufTex,
        *m_MultLocalLightsShader,
        *m_LineShader,
        *m_ShadowShader,
        *m_SkyDomeShader,
        *m_AmbientOcclusionShader;


    ComputeShader* m_HorizontalBlur, *m_VerticalBlur, 
        *m_HorizontalAOBlur, *m_VerticalAOBlur;

    // A ref to the scene camera
    Camera* m_Camera;

    // Transformation matrices
    glm::mat4 m_ProjMat, m_ViewMat, m_ModelMat;
    glm::mat4 m_RotMat;

    glm::mat4 m_ShadowProj, m_ShadowView, m_BMat;

    // Mesh for sphere
    SphereMesh m_SphereMesh, m_SkyDomeMesh;

    // G Buffer for Def. Shading
    FBO m_FBOForDefShading;

    // Store depth values when rendered from
    // the global light's perspective - req for shadows
    FBO m_FBOLightDepth;

    // Store ambient occlusion at each frag
    FBO m_FBOAmbientOcclusion;

    // Blurred shadow-map
    FBO m_FBOLightDepthBlurred;

    // Full-screen quad on which final image is output
    QuadMesh m_Quad;

    int m_KernelHalfWidth = 5;

    float m_CenterRadius = 10.0f;

    float m_ExposureControl = 1.0f;

    // TODO: Make these modifiable via the editor
    float m_s = 1.0f, m_k = 1.0f, m_R = 0.5f;

    bool m_IBLon = false;

    unsigned int m_CubeMapTexID;
    unsigned int m_SkyBoxVAO;
    unsigned int m_SkyBoxVBO;

    unsigned m_Block, m_Block1, m_HammersleyBlock;

    unsigned m_HammerseleyBindPoint;

    HammerseleyData m_HammersleyData;

    HDRTextureSet* m_HDRTexSet;

    // TODO: Remove this stuff
    bool m_DrawSprings = true;
    bool m_DrawSkin = false;
    bool m_DrawMassPoints = true;

    bool m_DebugCollidersOn = true;

private:
    //void SetLightingVars(Shader* shader);
    void SetupLineShaderVars();
    //void DrawGrid();
};

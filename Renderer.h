#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>

#include "Camera.h"
#include "Shader.h"
#include "SphereMesh.h"

#include "Object.h"

#include "FBO.h"
#include "QuadMesh.h"

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

	void Draw(std::vector<Object*>& objects, 
        int SCREEN_WIDTH, int SCREEN_HEIGHT);
    
    void Deserialize(std::string path);
    nlohmann::json::value_type Serialize();

    void AddLight(glm::vec3 position, glm::vec3 color, float range) {
        m_LocalLights.push_back(Light(position, color, range));
    }

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
    Shader* m_ModelShader, * m_ModelShaderPBR,
        *m_DefShaderGBuffer, * m_DefShaderLighting,
        *m_DefShaderGBufTex,
        *m_MultLocalLightsShader,
        *m_LineShader,
        *m_ShadowShader;

    // A ref to the scene camera
    Camera* m_Camera;

    // Transformation matrices
    glm::mat4 m_ProjMat, m_ViewMat, m_ModelMat;
    glm::mat4 m_RotMat;

    glm::mat4 m_ShadowProj, m_ShadowView, m_BMat;

    // Mesh for sphere
    SphereMesh m_SphereMesh;

    // G Buffer for Def. Shading
    FBO m_FBOForDefShading;

    // Store depth values when rendered from
    // the global light's perspective - req for shadows
    FBO m_FBOLightDepth;

    // Full-screen quad on which final image is output
    QuadMesh m_QuadDefShadingOutput;

    unsigned int m_CubeMapTexID;
    unsigned int m_SkyBoxVAO;
    unsigned int m_SkyBoxVBO;

    // TODO: Remove this stuff
    bool m_DrawSprings = true;
    bool m_DrawSkin = false;
    bool m_DrawMassPoints = true;

private:
    //void SetLightingVars(Shader* shader);
    void SetupLineShaderVars();
    //void DrawGrid();
};

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
    glm::vec3 m_Color;
    glm::vec3 m_Position;
    float m_Range;
};

class Renderer
{
public:
	Renderer(Camera* cam, int SCREEN_WIDTH, int SCREEN_HEIGHT);

	void Draw(std::vector<Object*>& objects, 
        int SCREEN_WIDTH, int SCREEN_HEIGHT);
    
    void Deserialize(std::string path);
    nlohmann::json::value_type Serialize();

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
        *m_LineShader;

    // A ref to the scene camera
    Camera* m_Camera;

    // Transformation matrices
    glm::mat4 m_ProjMat, m_ViewMat, m_ModelMat;
    glm::mat4 m_RotMat;

    // Mesh for sphere
    SphereMesh m_SphereMesh;

    // G Buffer for Def. Shading
    FBO FBOForDefShading;

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

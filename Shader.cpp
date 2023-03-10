#include "Shader.h"

#include <glm/gtc/type_ptr.hpp>

void CheckCompileErrors(GLuint shader, std::string type);

Shader::Shader(const char* vertexShaderPath, const char* fragmentShaderPath, const char* geometryPath)
{
    // Retrieve the vertex/fragment source code from filePath
    std::string vertexShaderCodeString;
    std::string fragmentShaderCodeString;
    std::string geometryShaderCodeString;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    std::ifstream gShaderFile;
    
    // Ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // Open files
        vShaderFile.open(vertexShaderPath);
        fShaderFile.open(fragmentShaderPath);
        std::stringstream vShaderStream, fShaderStream;

        // Read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        vShaderFile.close();
        fShaderFile.close();
        
        // Convert stream into string
        vertexShaderCodeString = vShaderStream.str();
        fragmentShaderCodeString = fShaderStream.str();

        if (geometryPath != nullptr)
        {
            gShaderFile.open(geometryPath);
            std::stringstream gShaderStream;
            gShaderStream << gShaderFile.rdbuf();
            gShaderFile.close();
            geometryShaderCodeString = gShaderStream.str();
        }
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
    }

    GLuint vertexShader, fragmentShader;
    
    // Store code in char array
    const GLchar* vertexShaderCode = vertexShaderCodeString.c_str();
    const GLchar* fragmentShaderCode = fragmentShaderCodeString.c_str();

    // Create vertex shader
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderCode, NULL);
    glCompileShader(vertexShader);
    CheckCompileErrors(vertexShader, "VERTEX");

    // Create fragment shader
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL);
    glCompileShader(fragmentShader);
    CheckCompileErrors(fragmentShader, "FRAGMENT");

    // Geometry shader - in case you have one
    unsigned int geometryShader;
    if (geometryPath != nullptr)
    {
        const char* gShaderCode = geometryShaderCodeString.c_str();
        geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometryShader, 1, &gShaderCode, NULL);
        glCompileShader(geometryShader);
        CheckCompileErrors(geometryShader, "GEOMETRY");
    }
    
    // Create the final shader program
    mID = glCreateProgram();
    glAttachShader(mID, vertexShader);
    glAttachShader(mID, fragmentShader);
    if (geometryPath != nullptr)
        glAttachShader(mID, geometryShader);
    glLinkProgram(mID);
    CheckCompileErrors(mID, "PROGRAM");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if (geometryPath != nullptr)
        glDeleteShader(geometryShader);
}

void Shader::Use()
{
    glUseProgram(mID);
}

void Shader::Unuse()
{
    glUseProgram(0);
}

void Shader::SetVec3(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2)
{
    glUniform3f(glGetUniformLocation(mID, name.c_str()), v0, v1, v2);
}

void Shader::SetVec3(const std::string& name, glm::vec3 value)
{
    glUniform3fv(glGetUniformLocation(mID, name.c_str()), 1, &value[0]);
}

void Shader::SetFloat(const std::string& name, GLfloat value)
{
    glUniform1f(glGetUniformLocation(mID, name.c_str()), value);
}

void Shader::SetInt(const std::string& name, GLint value)
{
    glUniform1i(glGetUniformLocation(mID, name.c_str()), value);
}

void Shader::SetMat4(const std::string& name, const glm::mat4& mat)
{
    glUniformMatrix4fv(glGetUniformLocation(mID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

GLuint Shader::GetID()
{
    return mID;
}

void CheckCompileErrors(GLuint shader, std::string type)
{
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

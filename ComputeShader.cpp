#include "ComputeShader.h"

#include <glm/gtc/type_ptr.hpp>


ComputeShader::ComputeShader(const char* computeShaderPath)
{
    // Retrieve the compute source code from filePath
    std::string computeShaderCodeString;
    std::ifstream cShaderFile;

    // Ensure ifstream objects can throw exceptions:
    cShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // Open files
        cShaderFile.open(computeShaderPath);
        std::stringstream cShaderStream;

        // Read file's buffer contents into streams
        cShaderStream << cShaderFile.rdbuf();
        cShaderFile.close();

        // Convert stream into string
        computeShaderCodeString = cShaderStream.str();
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
    }

    GLuint computeShader;

    // Store code in char array
    const GLchar* computeShaderCode = computeShaderCodeString.c_str();

    // Create compute shader
    computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, &computeShaderCode, NULL);
    glCompileShader(computeShader);
    CheckCompileErrors(computeShader, "COMPUTE");

    // Create the final shader program
    mID = glCreateProgram();
    glAttachShader(mID, computeShader);
    glLinkProgram(mID);
    CheckCompileErrors(mID, "PROGRAM");

    glDeleteShader(computeShader);
}


void ComputeShader::CheckCompileErrors(GLuint shader, std::string type)
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

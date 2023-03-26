#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

class ComputeShader
{
public:
	ComputeShader(const char* computeShaderPath);
	void Use() { glUseProgram(mID); };
	void Unuse() { glUseProgram(0); };

	inline GLuint GetID() { return mID; }
	void CheckCompileErrors(GLuint shader, std::string type);

private:
	GLuint mID;
};


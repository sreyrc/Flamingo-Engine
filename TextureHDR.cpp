#include <glad/glad.h>
#include <iostream>

#include "TextureHDR.h"

TextureHDR::TextureHDR(std::string path) : mID(0) {

	int width, height, nrComponents;

	stbi_set_flip_vertically_on_load(false);
	float* l_image = stbi_loadf(path.c_str(), &width, &height, &nrComponents, 0);
	printf("%d %d %d %s\n", nrComponents, width, height, path.c_str());
	if (!l_image) {
		printf("\nRead error on file %s:\n  %s\n\n", path.c_str(), stbi_failure_reason());
		exit(-1);
	}

	//for (int i = 0; i < width*height * 3; ++i)
	  //  printf("%f\n", l_image[i]);

	glGenTextures(1, &mID);   // Get an integer id for this texture from OpenGL
	glBindTexture(GL_TEXTURE_2D, mID);
	glTexImage2D(GL_TEXTURE_2D, 0, (GLint)GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, l_image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 10);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)GL_LINEAR_MIPMAP_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void TextureHDR::Bind() {
	glBindTexture(GL_TEXTURE_2D, mID);
}

void TextureHDR::Unbind() {
	glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint TextureHDR::GetID() {
	return mID;
}

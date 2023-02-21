#pragma once
#include <stdio.h>

#include <glad/glad.h>
#include <vector>

class FBO {
public:
	void CreateFBO(const int width, const int height, int numColorAttachments) {
		m_Width = width;
		m_Height = height;
		m_GBuffers.resize(numColorAttachments);
		std::vector<GLenum> GBufColorAttachments(numColorAttachments);

		// Generate an FBO identifier, and bind it
		glGenFramebuffers(1, &m_FBOID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_FBOID);

		// Create a render buffer, and attach it to FBO's depth attachment
		unsigned int depthBuffer;
		glGenRenderbuffers(1, &depthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Width, m_Height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
			GL_RENDERBUFFER, depthBuffer);
		
		// Create textures and attach FBO's color attachments. The
		// GL_RGBA32F and GL_RGBA constants set this texture to be 32 bit
		// floats for each of the 4 components. Many other choices are
		// possible.
		
		// G-Buffer 0: World Position
		// G-Buffer 1: World Normals
		// G-Buffer 2: Albedo (Diffuse color)
		// G-Buffer 3: Roughness and Metalness
		for (int i = 0; i < numColorAttachments; i++) {
			glGenTextures(1, &m_GBuffers[i]);
			glBindTexture(GL_TEXTURE_2D, m_GBuffers[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, (int)GL_RGBA32F,
				m_Width, m_Height, 0, GL_RGBA, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (int)GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (int)GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)GL_LINEAR);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
				GL_TEXTURE_2D, m_GBuffers[i], 0);

			GBufColorAttachments[i] = GL_COLOR_ATTACHMENT0 + i;
		}
		
		if (numColorAttachments > 0) {
			glDrawBuffers(numColorAttachments, &GBufColorAttachments[0]);
		}

		// Check for completeness/correctness
		int status = (int)glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != int(GL_FRAMEBUFFER_COMPLETE))
			printf("FBO Error: %d\n", status);
		
		// Unbind the FBO until it's ready to be used
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	void Bind() { glBindFramebuffer(GL_FRAMEBUFFER, m_FBOID); }
	void Unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
	
	std::vector<GLenum> m_GBuffers;

private:
	unsigned int m_Width, m_Height, m_FBOID;// m_TextureID;;
};
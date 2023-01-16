#pragma once

#include <glad/glad.h>
#include <stb/stb_image.h>
#include <string>

class Texture {

public:
    Texture() : m_TextureID(0) {}

    Texture(const std::string& path, const std::string& type) : m_TextureID(0)
    {
        stbi_set_flip_vertically_on_load(true);
        //m_Image = stbi_load(path.c_str(), &m_Width, &m_Height, &m_Depth, 4);
        //printf("%d %d %d %s\n", m_Depth, m_Width, m_Height, path.c_str());
        //if (!m_Image) {
        //    printf("\nRead error on file %s:\n  %s\n\n", path.c_str(), stbi_failure_reason());
        //    return;
        //    //exit(-1);
        //}

        //m_Loaded = true;
        //GLenum format{};
        //if (m_Depth == 1)
        //    format = GL_RED;
        //else if (m_Depth == 3)
        //    format = GL_RGB;
        //else if (m_Depth == 4)
        //    format = GL_RGBA;

        //glGenTextures(1, &m_TextureID);   // Get an integer id for this texture from OpenGL
        //glBindTexture(GL_TEXTURE_2D, m_TextureID);
        //glTexImage2D(GL_TEXTURE_2D, 0, format, m_Width, m_Height, 0, format, GL_UNSIGNED_BYTE, m_Image);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 10);
        //glGenerateMipmap(GL_TEXTURE_2D);

        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)GL_LINEAR);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)GL_LINEAR_MIPMAP_LINEAR);

        //glBindTexture(GL_TEXTURE_2D, 0);
        //stbi_image_free(m_Image);

        //m_Type = type;
        m_Type = type;
        //mPath = path;

        int width, height, nrComponents;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            m_Loaded = true;
            GLenum format{};
            switch (nrComponents) {
            case 1: format = GL_RED; break;
            case 3: format = GL_RGB; break;
            case 4: format = GL_RGBA; break;
            }

            glGenTextures(1, &m_TextureID);

            glBindTexture(GL_TEXTURE_2D, m_TextureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else
        {
            std::cout << "Texture failed to load at path: " << path << std::endl;
            stbi_image_free(data);
        }
    }

    // Make a texture availabe to a shader program.  The unit parameter is
    // a small integer specifying which texture unit should load the
    // texture.  The name parameter is the sampler2d in the shader program
    // which will provide access to the texture.
    void BindTexture(const int unit, const int programId, const std::string& name)
    {
        glActiveTexture((GLenum)((int)GL_TEXTURE0 + unit));
        glBindTexture(GL_TEXTURE_2D, m_TextureID);
        int loc = glGetUniformLocation(programId, name.c_str());
        glUniform1i(loc, unit);
    }

    // Unbind a texture from a texture unit whne no longer needed.
    void UnbindTexture(const int unit)
    {
        glActiveTexture((GLenum)((int)GL_TEXTURE0 + unit));
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    inline bool isLoaded() { return m_Loaded; }

    inline std::string GetType() { return m_Type; }

private:
    bool m_Loaded = false;
    unsigned int m_TextureID;
    int m_Width, m_Height, m_Depth;
    unsigned char* m_Image;
    std::string m_Type;
};

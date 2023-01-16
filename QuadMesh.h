#pragma once

#include <glad/glad.h>

class QuadMesh
{
public:
    QuadMesh() {
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);

        glBindVertexArray(m_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(m_Vertices), m_Vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(sizeof(GLfloat) * 3));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(sizeof(GLfloat) * 5));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }

    void BindVAO() {
        glBindVertexArray(m_VAO);
    }

private:
    GLuint m_VAO, m_VBO;
    GLfloat m_Vertices[48] = {
        // positions   // texCoords // normals
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,   0.0f, 0.0f, 1.0f,
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,   0.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f,   0.0f, 0.0f, 1.0f,
    };
};


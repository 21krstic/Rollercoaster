#pragma once
#include <GL/glew.h>

class SpriteRenderer {
public:
    GLuint VAO = 0;
    GLuint VBO = 0;

    void init() {
        // x, y, u, v
        float vertices[] = {
            -1.0f, -1.0f,  0.0f, 0.0f,  // bottom-left
             1.0f, -1.0f,  1.0f, 0.0f,  // bottom-right
             1.0f,  1.0f,  1.0f, 1.0f,  // top-right
            -1.0f,  1.0f,  0.0f, 1.0f   // top-left
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // position
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // texcoord
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void draw() const {
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4); // draw quad as triangle fan
        glBindVertexArray(0);
    }

    void cleanup() {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }
};

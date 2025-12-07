#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // for translate/rotate/scale
#include <glm/gtc/type_ptr.hpp>

class SpriteRenderer {
public:
    GLuint shader;
    GLuint VAO = 0;
    GLuint VBO = 0;

    float vertices[16] = {
        -1.0f, -1.0f,  0.0f, 0.0f,  // bottom-left
         1.0f, -1.0f,  1.0f, 0.0f,  // bottom-right
         1.0f,  1.0f,  1.0f, 1.0f,  // top-right
        -1.0f,  1.0f,  0.0f, 1.0f   // top-left
    };

    SpriteRenderer(GLuint shaderProgram) : shader(shaderProgram) {
        init();
    }

    ~SpriteRenderer() {
        glDeleteVertexArrays(1, &VAO); // :)
    }

    void init() {

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

    void draw(GLuint tex) {
        draw(tex, glm::mat4(1));
    }

    void draw(GLuint tex, const glm::mat4& model) {
        glUseProgram(shader);

        // Pass the model matrix
        glUniformMatrix4fv(glGetUniformLocation(shader, "uModel"), 1, GL_FALSE, glm::value_ptr(model));

        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        glUniform1i(glGetUniformLocation(shader, "spriteTexture"), 0); // make sure uniform name matches shader

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);
    }


    void cleanup() {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }
};

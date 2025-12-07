#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <thread>
#include <chrono>
#include <vector>
#include "../Header/Util.h"
#include "../Header/spriteRenderer.hpp"
#define FORCE_FULLSCREEN false

// Main fajl funkcija sa osnovnim komponentama OpenGL programa

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main()
{
    //////////////////////////////////////////////////////////////////////////////
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    if (FORCE_FULLSCREEN) const GLFWvidmode* mode = glfwGetVideoMode(monitor); // fullscren ili običan //
    else monitor = NULL;
    GLFWwindow* window = glfwCreateWindow(1366, 768, "R o l e r k o s t e r", monitor, NULL);
    if (window == NULL) return endProgram("Prozor nije uspeo da se kreira.");
    glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); // Podešavanje callback funkcije za promenu veličine prozora //

    if (glewInit() != GLEW_OK) return endProgram("GLEW nije uspeo da se inicijalizuje.");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    unsigned shader = createShader("Shaders/vertex.glsl", "Shaders/fragment.glsl");

	glClearColor(0.2f, 0.8f, 0.6f, 1.0f); // fallback boja pozadine //

    SpriteRenderer quad(shader);
    quad.init();
    //////////////////////////////////////////////////////////////////////////////

    double targetFrameTime = 1.0 / 75.0; // Vreme trajanja jednog frejma (u sekundama) za 75 FPS //
    double frameStart = 0, frameEnd;
    double startTime = glfwGetTime();
    int currentFrame = 0;
    float accumulator = 0.0f;
    float potpisFPS = 3.0f;
    float frameTime = 1.0f / potpisFPS;

    glm::mat4 modelPotpis = glm::mat4(1.0f); // Jedinična matrica
    modelPotpis = glm::translate(modelPotpis, glm::vec3(-0.82f, -0.85f, 0.0f));
    modelPotpis = glm::scale(modelPotpis, glm::vec3(0.17));

    GLuint background = loadImageToTexture("Resources/Background.png");
    GLFWcursor* cursor = loadImageToCursor("Resources/cursor.png"); // Kursor sa izgledom šina //
    glfwSetCursor(window, cursor);
    std::vector<std::string> potpisPaths = { // Učitavanje frejmova potpisa //
        "Resources/potpis_1.png",
        "Resources/potpis_2.png",
    };

    std::vector<GLuint> frames;
    frames.reserve(potpisPaths.size());
    for (auto& p : potpisPaths) {
        GLuint t = loadImageToTexture(p.c_str());
        frames.push_back(t);
    }

    while (!glfwWindowShouldClose(window))
    {
        float dt = (float)(glfwGetTime() - frameStart);
        frameStart = glfwGetTime();

        glClear(GL_COLOR_BUFFER_BIT);


		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) // Zatvaranje prozora pritiskom na ESC //
			glfwSetWindowShouldClose(window, true);

        quad.draw(background);

        accumulator += dt;
        while (accumulator >= frameTime) {
            accumulator -= frameTime;
            currentFrame = (currentFrame + 1) % frames.size();
        }
        quad.draw(frames[currentFrame], modelPotpis);

        glfwSwapBuffers(window);
        glfwPollEvents();

        frameEnd = glfwGetTime();
        double elapsed = frameEnd - frameStart;
        if (elapsed < targetFrameTime) {
            std::this_thread::sleep_for(std::chrono::duration<double>(targetFrameTime - elapsed));
        }
    }

    quad.cleanup();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
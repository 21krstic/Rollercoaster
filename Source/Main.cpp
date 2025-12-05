#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <thread>
#include <chrono>
#include "../Header/Util.h"
#include "../Header/spriteRenderer.hpp"

// Main fajl funkcija sa osnovnim komponentama OpenGL programa

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main()
{
	double targetFrameTime = 1.0 / 75.0; // Vreme trajanja jednog frejma (u sekundama) za 75 FPS //
    double frameStart, frameEnd;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1366, 768, "R o l e r k o s t e r", NULL, NULL);
    if (window == NULL) return endProgram("Prozor nije uspeo da se kreira.");
    glfwMakeContextCurrent(window);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); // Podešavanje callback funkcije za promenu veličine prozora //

    GLuint background = loadImageToTexture("Resources/Background.png");
    GLuint potpis_1 = loadImageToTexture("Resources/potpis_1.png");
	GLFWcursor* cursor = loadImageToCursor("Resources/cursor.png"); // Kursor sa izgledom šina //
    glfwSetCursor(window, cursor);

    if (glewInit() != GLEW_OK) return endProgram("GLEW nije uspeo da se inicijalizuje.");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    unsigned shader = createShader("Shaders/vertex.glsl", "Shaders/fragment.glsl");

    glClearColor(0.2f, 0.8f, 0.6f, 1.0f);

    SpriteRenderer quad;
    quad.init();
    while (!glfwWindowShouldClose(window))
    {
        frameStart = glfwGetTime();

        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shader);


		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) // Zatvaranje prozora pritiskom na ESC //
			glfwSetWindowShouldClose(window, true);

        glBindTexture(GL_TEXTURE_2D, background);
        quad.draw();
        glBindTexture(GL_TEXTURE_2D, potpis_1);
        quad.draw();

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
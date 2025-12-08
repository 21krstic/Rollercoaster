#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <thread>
#include <chrono>
#include <vector>
#include <iostream>
#include "../Header/Util.h"
#include "../Header/spriteRenderer.hpp"
#include "../rollercoaster.cpp"
#define FORCE_FULLSCREEN true

// Main fajl funkcija sa osnovnim komponentama OpenGL programa

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

bool mouseInBounds(glm::vec2 mouse, glm::vec2 center, glm::vec2 halfSize) {
    return mouse.x >= center.x - halfSize.x && mouse.x <= center.x + halfSize.x &&
        mouse.y >= center.y - halfSize.y && mouse.y <= center.y + halfSize.y;
}

struct Passenger {
    GLuint baseTexture;
    GLuint beltTexture;
    bool hasBelt = false;
    glm::vec2 pos; // world position
    glm::vec2 size;
};

std::vector<Passenger> passengers;

int main()
{
    //////////////////////////////////////////////////////////////////////////////
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    if (FORCE_FULLSCREEN) const GLFWvidmode* mode = glfwGetVideoMode(monitor); // fullscren ili običan //
    else monitor = NULL;
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "R o l e r k o s t e r", monitor, NULL);
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
    GLuint rollercoaster = loadImageToTexture("Resources/rollercoaster.png");
    GLuint seatbelt = loadImageToTexture("Resources/seatbelt.png");
    GLFWcursor* cursor = loadImageToCursor("Resources/cursor.png"); // Kursor sa izgledom šina //
    glfwSetCursor(window, cursor);
    std::vector<std::string> potpisPaths = { // Učitavanje frejmova potpisa //
        "Resources/potpis_1.png",
        "Resources/potpis_2.png",
    };

    std::vector<std::string> passengerPaths = {
        "Resources/passenger_1.png",
        "Resources/passenger_2.png",
        "Resources/passenger_3.png",
        "Resources/passenger_4.png",
        "Resources/passenger_5.png",
        "Resources/passenger_6.png",
        "Resources/passenger_7.png",
        "Resources/passenger_8.png",
    };

    std::vector<std::string> sickPassengerPaths = {
        "Resources/sick_passenger_1.png",
        "Resources/sick_passenger_2.png",
        "Resources/sick_passenger_3.png",
        "Resources/sick_passenger_4.png",
        "Resources/sick_passenger_5.png",
        "Resources/sick_passenger_6.png",
        "Resources/sick_passenger_7.png",
        "Resources/sick_passenger_8.png",
    }; 

    std::vector<int> passengerSickState(8, 0);
    std::vector<int> passengerBeltState(8, 0);

    std::vector<GLuint> signatureFrames;
    signatureFrames.reserve(potpisPaths.size());
    for (auto& p : potpisPaths) {
        GLuint t = loadImageToTexture(p.c_str());
        signatureFrames.push_back(t);
    }

    std::vector<GLuint> passengerSprites;
    passengerSprites.reserve(passengerPaths.size());
    for (auto& p : passengerPaths) {
        GLuint t = loadImageToTexture(p.c_str());
        passengerSprites.push_back(t);
    }

    std::vector<GLuint> sickPassengerSprites;
    sickPassengerSprites.reserve(sickPassengerPaths.size());
    for (auto& p : sickPassengerPaths) {
        GLuint t = loadImageToTexture(p.c_str());
        sickPassengerSprites.push_back(t);
    }

	GLuint carTexture = loadImageToTexture("Resources/car.png");

    Rollercoaster coaster;    
    coaster.init(trackPoints, carTexture, &quad, 4, 0.06f);

    glm::mat4 coasterModel[4];
    glm::mat4 passengerModel[8];

    for (int i = 0; i < 4; i++) {
        coasterModel[i] = glm::mat4(1.0f);
        coasterModel[i] = glm::scale(coasterModel[i], glm::vec3(0.12f));
        coasterModel[i] = glm::translate(coasterModel[i], glm::vec3(3.0f - i * 1.7f, -4.92f, 0.0f));
    }
    float cartU[4] = { 0,0,0,0 };
    float cartSpeed[4] = { 0.8f,0.8f,0.8f,0.8f };
    float cartAngle[4] = { 0,0,0,0 };
    const float minSpeed = 0.05f, maxSpeed = 2.0f;
    const float accel = 22.2f;
    const float spacing = 0.001f;
    const float curveLength = 3.5f;
    int nextPassenger = 0;

    while (!glfwWindowShouldClose(window))
    {
        float dt = (float)(glfwGetTime() - frameStart);
        frameStart = glfwGetTime();

        glClear(GL_COLOR_BUFFER_BIT);

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) // Zatvaranje prozora pritiskom na ESC //
			glfwSetWindowShouldClose(window, true);

        quad.draw(background);
        //quad.draw(rollercoaster); MOZE BOLJE

        accumulator += dt;
        while (accumulator >= frameTime) {
            accumulator -= frameTime;
            currentFrame = (currentFrame + 1) % signatureFrames.size();
        }
        quad.draw(signatureFrames[currentFrame], modelPotpis);

        bool hasSick = std::any_of(passengerSickState.begin(), passengerSickState.end(), [](int state) { return state == 1; });

        if (coaster.moving && !hasSick) {
            for (int key = GLFW_KEY_1; key <= GLFW_KEY_8; ++key) {
                if (glfwGetKey(window, key) == GLFW_PRESS) {
                    int idx = key - GLFW_KEY_1;  // 0..7
                    if (idx < nextPassenger) { passengerSickState[idx] = 1; 
                    std::cout << "bolesnifikovan";
                    }
                }
            }
        }

        coaster.update(dt);        
        
        static bool lastEnter = false;
        bool enterNow = (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS);
        if (enterNow && !lastEnter) {
            coaster.toggleMoving();
            std::cout << "Toggled ride" << std::endl;

            for (int i = 0; i < 4; ++i) {
                glm::vec4 p = coasterModel[i] * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
                glm::vec3 pos = glm::vec3(p);
                std::cout << "cart " << i << " at " << pos.x << ", " << pos.y << "\n";
            }
        }
        lastEnter = enterNow;

        for (int i = 0; i < 4; ++i) {
            if (coaster.moving) {
                cartU[i] += cartSpeed[0] * dt;
            }

            float uWrapped = fmod(cartU[i] - i * spacing, curveLength);
            if (hasSick && uWrapped < 0.01f) {
                coaster.moving = false;
                std::fill(passengerSickState.begin(), passengerSickState.end(), 0);
            }
            // composite sine
            float a1 = 0.2f, b1 = 12.0f, a2 = 0.15f, b2 = 16.0f, p2 = 3.1f, a3 = 0.00f, b3 = 4.8f, p3 = 0.5f;
            float y = a1 * sin(b1 * uWrapped) + a2 * sin(b2 * uWrapped + p2) + a3 * sin(b3 * uWrapped + p3) - 0.2f;

            float dydu = a1 * b1 * cos(b1 * uWrapped) + a2 * b2 * cos(b2 * uWrapped + p2) + a3 * b3 * cos(b3 * uWrapped + p3);
            if (dydu < 0) cartSpeed[i] += accel * (-dydu) * dt * 0.05f;
            else cartSpeed[i] -= accel * dydu * dt * 0.05f;
            cartSpeed[i] = glm::clamp(cartSpeed[i] * 0.999f, minSpeed, maxSpeed);

            if (hasSick) cartSpeed[i] = std::max(cartSpeed[i] - 0.02f, 0.2f); // imamo bolesnika... usporavaj na 0.2

            float startX = -0.83f + i * 0.2f;
            float x = startX + uWrapped * 0.6f;

            cartAngle[i] = atan(dydu); // rotation in radians

            coasterModel[i] = glm::mat4(1.0f);
            coasterModel[i] = glm::translate(coasterModel[i], glm::vec3(x, y, 0.0f));
            coasterModel[i] = glm::rotate(coasterModel[i], cartAngle[i], glm::vec3(0.0f, 0.0f, 1.0f));
            coasterModel[i] = glm::scale(coasterModel[i], glm::vec3(0.12f));
        }

        static bool lastSpace = false; // dodavanje putnika
        bool spaceNow = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
        if (spaceNow && !lastSpace) {
            if (!coaster.moving) {
                if (nextPassenger < 8) {
                    nextPassenger++;
                    std::cout << "Added passenger " << nextPassenger << "\n";
                }
            }
        }
        lastSpace = spaceNow;

        double mx, my;
        glfwGetCursorPos(window, &mx, &my);

        int width, height;
        glfwGetWindowSize(window, &width, &height);
        glm::vec2 mouse((float)(mx / width * 2.0 - 1.0),
            (float)(1.0 - my / height * 2.0)); // invert y ???? 

        for (auto& p : passengers) {
            if (mouseInBounds(mouse, p.pos, glm::vec2(1.1f, 1.05f))) {
                p.hasBelt = true;
                std::cout << "Seatbelt on\n";
            }
        }

        for (int i = 3; i >= 0; --i) { 
            int start = i * 2;
            int end = std::min(start + 2, nextPassenger);
            for (int j = start; j < end; ++j) {
                passengerModel[j] = glm::mat4(1.0f);

                float xOffset = (j - start == 0 ? 0.04f : -0.04f); // desno pa levo
				float yOffset = 0.1f; // putnici sede malo iznad centra kola

                passengerModel[j] = glm::translate(passengerModel[j], glm::vec3(
                    glm::vec3(coasterModel[3-i][3]) + glm::vec3(xOffset, yOffset, 0.0f)
                ));
                if (passengerBeltState[j]) quad.draw(seatbelt, passengerModel[j]);
                passengerModel[j] = glm::rotate(passengerModel[j], cartAngle[i], glm::vec3(0.0f, 0.0f, 1.0f));
                passengerModel[j] = glm::scale(passengerModel[j], glm::vec3(0.06f));       
                passengerSickState[j] == 0 ? quad.draw(passengerSprites[j], passengerModel[j]) :  // crtanje bolesnika
                                         quad.draw(sickPassengerSprites[j], passengerModel[j]);
            }
        }


        quad.draw(carTexture, coasterModel[0]);
        quad.draw(carTexture, coasterModel[1]);
        quad.draw(carTexture, coasterModel[2]);
        quad.draw(carTexture, coasterModel[3]);

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
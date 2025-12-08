#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include "Header/spriteRenderer.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <algorithm>

static SpriteRenderer* renderer = nullptr; // drugačije ću...

static std::vector<glm::vec2> trackPoints = {
    {0.0f, 0.0f},
    {0.0f, 0.2f},
    {0.1f, 0.4f},   // 1.
    {0.0f, 0.6f},
    {-0.1f, 0.8f},  // 2.
    {0.0f, 1.0f},   // 3.
    {0.0f, 1.2f}
};

struct Passenger {
    glm::vec2 offset;
    GLuint sprite;
    bool seated = false;
    bool beltOn = false;
    bool sick = false;
};

struct Car {
    float t = 0.0f; // 0 = start, 1 = end
    float speed = 0.0f;
    float maxSpeed = 1.0f;
    glm::vec2 position;
    std::vector<Passenger> passengers;
};

class Rollercoaster {
public:
    Rollercoaster() { car.passengers.resize(8); }

    void init(const std::vector<glm::vec2>& points, GLuint carSprite) {
        track = points;
        carTexture = carSprite;
    }

    void addPassenger(GLuint sprite) {
        if (passengerCount < 8) {
            car.passengers[passengerCount].sprite = sprite;
            car.passengers[passengerCount].seated = true;
            passengerCount++;
        }
    }

    void toggleBelt(int idx) {
        if (idx < 0 || idx >= passengerCount) return;
        car.passengers[idx].beltOn = !car.passengers[idx].beltOn;
    }

    void triggerSick(int idx) {
        if (idx < 0 || idx >= passengerCount) return;
        car.passengers[idx].sick = true;
        emergencyStop = true;
        emergencyTimer = 10.0f;
    }

    void startRide() {
        bool allBeltsOn = true;
        for (auto& p : car.passengers) {
            if (p.seated && !p.beltOn) {
                allBeltsOn = false;
                break;
            }
        }
        if (allBeltsOn) moving = true;
    }

    bool rideFinished() const { return car.t >= 1.0f; }

    void update(float dt) {
        car.position = getCarPosition();

        if (emergencyStop) {
            emergencyTimer -= dt;
            car.speed = 0.0f;
            if (emergencyTimer <= 0.0f) {
                emergencyStop = false;
                moving = false;
            }
            return;
        }

        if (moving) {
            float slope = getSlope();
            if (slope < 0) car.speed += 0.5f * dt;
            else if (slope > 0) car.speed -= 0.3f * dt;

            car.speed = glm::clamp(car.speed, 0.0f, car.maxSpeed);
            car.t += car.speed * dt;
            if (car.t >= 1.0f) moving = false;
        }
        else if (car.t > 0.0f) {
            car.speed = 0.1f;
            car.t -= car.speed * dt;
            if (car.t <= 0.0f) {
                car.t = 0.0f;
                passengerCount = 0;
                for (auto& p : car.passengers) {
                    p.seated = false;
                    p.beltOn = false;
                    p.sick = false;
                }
            }
        }
    }

    void drawRollercoaster() {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(car.position, 0.0f));
        renderer->draw(carTexture, model);

        for (auto& p : car.passengers) {
            if (!p.seated) continue;
            glm::mat4 passengerModel = glm::translate(model, glm::vec3(p.offset, 0.0f));
            renderer->draw(p.sprite, passengerModel); // beltOn -> nacrtaj pojas
        }
    }

private:
    std::vector<glm::vec2> track;
    Car car;
    GLuint carTexture;
    int passengerCount = 0;
    bool moving = false;
    bool emergencyStop = false;
    float emergencyTimer = 0.0f;

    glm::vec2 getCarPosition() const {
        if (track.empty()) return glm::vec2(0, 0);
        int idx = std::min((int)(car.t * (track.size() - 1)), (int)track.size() - 1);
        return track[idx];
    }

    float getSlope() const {
        if (track.empty()) return 0.0f;
        int idx = std::min((int)(car.t * (track.size() - 2)), (int)track.size() - 2);
        glm::vec2 a = track[idx];
        glm::vec2 b = track[idx + 1];
        return b.y - a.y;
    }
};


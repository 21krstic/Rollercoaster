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

struct Passenger2 {
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
    glm::vec2 position{ 0.0f,0.0f };
    float initialOffset = 0.0f; // per-car offset so multiple cars can be spaced
    std::vector<Passenger2> passengers;
};

class Rollercoaster {
public:

    void toggleMoving() {
        moving = !moving;
    }

    Rollercoaster() {}

    void init(const std::vector<glm::vec2>& points, GLuint carSprite, SpriteRenderer* r, int numCars = 4, float spacing = 0.06f) {
        track = points;
        carTexture = carSprite;
        renderer = r;
        cars.clear();
        // create numCars cars with spaced initial offsets (negative offsets mean placed behind start)
        for (int i = 0; i < numCars; ++i) {
            Car c;
            c.t = 0.0f;
            c.initialOffset = -i * spacing;
            c.maxSpeed = 0.6f;
            c.passengers.resize(2); // example
            cars.push_back(c);
        }
    }

    void addPassengerToCar(int carIdx, GLuint sprite) {
        if (carIdx < 0 || carIdx >= (int)cars.size()) return;
        auto& car = cars[carIdx];
        for (auto& p : car.passengers) {
            if (!p.seated) {
                p.sprite = sprite;
                p.seated = true;
                return;
            }
        }
    }

    void toggleBelt(int carIdx, int passengerIdx) {
        if (carIdx < 0 || carIdx >= (int)cars.size()) return;
        auto& car = cars[carIdx];
        if (passengerIdx < 0 || passengerIdx >= (int)car.passengers.size()) return;
        car.passengers[passengerIdx].beltOn = !car.passengers[passengerIdx].beltOn;
    }

    void triggerSick(int carIdx, int passengerIdx) {
        if (carIdx < 0 || carIdx >= (int)cars.size()) return;
        auto& car = cars[carIdx];
        if (passengerIdx < 0 || passengerIdx >= (int)car.passengers.size()) return;
        car.passengers[passengerIdx].sick = true;
        emergencyStop = true;
        emergencyTimer = 10.0f;
    }

    void startRide() {
        // require belts on for seated passengers, if you want that rule
        /*for (auto& car : cars) {
            bool ok = true;
            for (auto& p : car.passengers) {
                if (p.seated && !p.beltOn) { ok = false; break; }
            }
            if (ok) carMovingStates.push_back(true);
        }*/
        moving = true;
    }

    bool rideFinished() const {
        for (auto& c : cars) if (c.t < 1.0f) return false;
        return true;
    }

    void update(float dt) {
        if (track.empty()) return;

        if (emergencyStop) {
            emergencyTimer -= dt;
            for (auto& c : cars) c.speed = 0.0f;
            if (emergencyTimer <= 0.0f) {
                emergencyStop = false;
                moving = false;
            }
            return;
        }

        if (!moving) {
            // rewind small amount toward 0 if needed
            for (auto& c : cars) {
                if (c.t > 0.0f) {
                    c.speed = 0.1f;
                    c.t -= c.speed * dt;
                    if (c.t <= 0.0f) {
                        c.t = 0.0f;
                        resetCar(c);
                    }
                }
            }
            return;
        }

        // update all cars
        for (auto& c : cars) {
            // effective t is car.t + initialOffset, clamp to [0,1] for sampling but maintain internal t for movement
            float effT = glm::clamp(c.t + c.initialOffset, 0.0f, 1.0f);
            // compute slope at effT
            float slope = getSlopeAt(effT);
            // simple physics: downhill accelerates, uphill slows
            if (slope < 0.0f) c.speed += 0.8f * dt;
            else if (slope > 0.0f) c.speed -= 0.4f * dt;
            c.speed = glm::clamp(c.speed, 0.0f, c.maxSpeed);
            c.t += c.speed * dt;
            if (c.t >= 1.0f) c.t = 1.0f;
            // update position
            c.position = sampleTrack(effT);
        }
    }

    void drawRollercoaster() {
        for (auto& c : cars) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(c.position, 0.0f));
            model = glm::scale(model, glm::vec3(0.12f));
            if (renderer) renderer->draw(carTexture, model);

            for (auto& p : c.passengers) {
                if (!p.seated) continue;
                glm::mat4 pm = glm::translate(model, glm::vec3(p.offset, 0.0f));
                if (renderer) renderer->draw(p.sprite, pm);
            }
        }
    }

    // expose track for visualizer
    const std::vector<glm::vec2>& getTrack() const { return track; }

    std::vector<glm::vec2> track;
    std::vector<Car> cars;
    GLuint carTexture = 0;
    bool moving = false;
    bool emergencyStop = false;
    float emergencyTimer = 0.0f;

    std::vector<bool> carMovingStates;

    void resetCar(Car& c) {
        c.speed = 0.0f;
        for (auto& p : c.passengers) {
            p.seated = false;
            p.beltOn = false;
            p.sick = false;
        }
    }

    glm::vec2 sampleTrack(float t) const {
        if (track.empty()) return glm::vec2(0.0f);
        float f = t * (track.size() - 1);
        int i = std::min((int)f, (int)track.size() - 2);
        float local = f - i;
        return glm::mix(track[i], track[i + 1], local);
    }

    float getSlopeAt(float t) const {
        if (track.size() < 2) return 0.0f;
        float f = t * (track.size() - 1);
        int i = std::min((int)f, (int)track.size() - 2);
        glm::vec2 a = track[i];
        glm::vec2 b = track[i + 1];
        return b.y - a.y;
    }
};

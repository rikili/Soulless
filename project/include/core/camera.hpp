#pragma once

#include <entities/ecs.hpp>
#include "common.hpp"

class Camera {
public:
    explicit Camera(float width, float height): position(0.0f, 0.0f), width(width), height(height) {
        setOrthographicProjection();
        updateViewMatrix();
    }

    void setPosition(float x, float y);
    void updateCamera(Camera& camera, Entity& player);

    glm::mat4 getViewMatrix() const {
        return viewMatrix;
    }

    glm::mat4 getProjectionMatrix() const {
        return projectionMatrix;
    }

private:
    void setOrthographicProjection();
    void updateViewMatrix();

    glm::vec2 position;
    float width, height;
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
};
#include "input/input_handler.hpp"
#include "entities/general_components.hpp"
#include <cstdio>
#include <iostream>

/*
- player slots for spell 1 and spell 2 (component) ? (needs to hold some spells)
- handle player movement (physics.cpp?)

- player rendering (ask group)
- how to handle spell being fired ??? (give it a velocity... ask group?)
Mergekk
*/

InputHandler::InputHandler() { }

void InputHandler::onKey(int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_W:
            case GLFW_KEY_S:
            case GLFW_KEY_A:
            case GLFW_KEY_D:
                activeMoveKeys.insert(key);
                updateVelocity();
                break;
            case GLFW_KEY_Q:
                printd("Q button pressed.\n");
                // TODO: drop spell 1 and increase HP
                break;
            case GLFW_KEY_E:
                printd("E button pressed.\n");
                // TODO: drop spell 2 and increase HP
                break;
            case GLFW_KEY_F:
                printd("F button pressed.\n");
                // TODO: interact with item on ground (unused for now)
                break;
            default:
                break;
        }
    } else if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_W:
            case GLFW_KEY_S:
            case GLFW_KEY_A:
            case GLFW_KEY_D:
                activeMoveKeys.erase(key);
                updateVelocity();
                break;
            default:
                break;
        }
    }
}

void InputHandler::onMouseMove(vec2 mouse_position) {
    //auto player = Entity();
    //Motion& motion = registry.motions.emplace(player);
    //motion.position = { 320, 240 };
    //motion.angle = 0.f;
    //motion.velocity = { 0.f, 0.f };
    //// Entity player = player_wizard;
    //Motion &playerMotion = registry.motions.get(player);

    //float dx = mouse_position.x - playerMotion.position.x;
    //float dy = mouse_position.y - playerMotion.position.y;

    //float angle = atan2(dy, dx);
    //playerMotion.angle = angle;

    //constexpr std::array<float, 8> cardinalAngles = {
    //    0.f,                  // East
    //    -M_PI / 4,            // North-East
    //    -M_PI / 2,            // North
    //    -3 * M_PI / 4,        // North-West
    //    -M_PI,                // West
    //    3 * M_PI / 4,         // South-West
    //    M_PI / 2,             // South
    //    M_PI / 4              // South-East
    //};

    //float closestAngle = cardinalAngles[0];
    //float smallestDifference = std::abs(angle - closestAngle);

    //for (const float& cardinalAngle : cardinalAngles) {
    //    float difference = std::abs(angle - cardinalAngle);
    //    
    //    if (difference < smallestDifference) {
    //        smallestDifference = difference;
    //        closestAngle = cardinalAngle;
    //    }
    //}

    //playerMotion.angle = closestAngle;
    //printd("New angle in degrees: %f\n", playerMotion.angle * 180 / M_PI);
}

void InputHandler::onMouseKey(int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (button) {
            case GLFW_MOUSE_BUTTON_LEFT:
                // TODO: Shoot spell 1
                printd("Left mouse button pressed.\n");
                break;
            case GLFW_MOUSE_BUTTON_RIGHT:
                // TODO: Shoot spell 2
                printd("Right mouse button pressed.\n");
                break;
            default:
                break;
        }
    }
}

void InputHandler::updateVelocity() {
    // TODO: get global player
    // Entity player = player_wizard;
    auto player = Entity();
    Motion& motion = registry.motions.emplace(player);
    motion.position = { 50, 50 };
    motion.angle = 0.f;
    motion.velocity = { 0.f, 0.f };

    auto& motion_registry = registry.motions;
    Motion& playerMotion = motion_registry.get(player);

    int verticalDir = activeMoveKeys.count(GLFW_KEY_S) - activeMoveKeys.count(GLFW_KEY_W);
    int horizontalDir = activeMoveKeys.count(GLFW_KEY_D) - activeMoveKeys.count(GLFW_KEY_A);

    float maxSpeed = 10; // TODO: get this from component

    if (horizontalDir != 0 || verticalDir != 0) {
        float normalizedSpeed = maxSpeed;

        if (horizontalDir != 0 && verticalDir != 0) {
            normalizedSpeed = sqrt((maxSpeed * maxSpeed) / 2); 
        }

        playerMotion.velocity.x = horizontalDir * normalizedSpeed;
        playerMotion.velocity.y = verticalDir * normalizedSpeed;
    } else {
        playerMotion.velocity.x = 0;
        playerMotion.velocity.y = 0;
    }

    printd("New velocity is: %f, %f\n", playerMotion.velocity.x, playerMotion.velocity.y);
}

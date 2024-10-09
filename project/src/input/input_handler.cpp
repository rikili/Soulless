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
            case GLFW_KEY_K:
                registry.debug = true;
            default:
                break;
        }
    }
}

void InputHandler::onMouseMove(vec2 mouse_position) {
    Entity& player = registry.players.entities[0];
    Motion& playerMotion = registry.motions.get(player);

    float dx = mouse_position.x - playerMotion.position.x;
    float dy = mouse_position.y - playerMotion.position.y;

    float angle = atan2(dy, dx);
    playerMotion.angle = angle;

    constexpr std::array<float, 8> cardinalAngles = {
        0.f,                  // East
        -M_PI / 4,            // North-East
        -M_PI / 2,            // North
        -3 * M_PI / 4,        // North-West
        -M_PI,                // West
        3 * M_PI / 4,         // South-West
        M_PI / 2,             // South
        M_PI / 4              // South-East
    };

    float closestAngle = cardinalAngles[0];
    float smallestDifference = std::abs(angle - closestAngle);

    for (const float& cardinalAngle : cardinalAngles) {
        float difference = std::abs(angle - cardinalAngle); 
        if (difference < smallestDifference) {
            smallestDifference = difference;
            closestAngle = cardinalAngle;
        }
    }

    playerMotion.angle = closestAngle;
}

void create_player_projectile(Entity& player_ent, double x, double y)
{

    Entity projectile_ent;
    Projectile& projectile = registry.projectiles.emplace(projectile_ent);
    Motion& projectile_motion = registry.motions.emplace(projectile_ent);
    Deadly& deadly = registry.deadlies.emplace(projectile_ent);
    Damage& damage = registry.damages.emplace(projectile_ent);
    RenderRequest& request = registry.render_requests.emplace(projectile_ent);

    deadly.to_enemy = true;

    Motion& player_motion = registry.motions.get(player_ent);
    projectile_motion.position = player_motion.position;

    // TODO: change once finalization is needed
    projectile.type = DamageType::fire;
    projectile_motion.velocity = vec2({ cos(player_motion.angle), sin(player_motion.angle) });
    damage.value = 25.f;

    request.mesh = "basic";
    request.shader = "basic";
}

void invoke_player_cooldown(Player& player, bool is_left)
{
    player.cooldown = 700;
}

void cast_player_spell(double x, double y, bool is_left)
{
    Entity& player_ent = registry.players.entities[0];
    Player& player = registry.players.get(player_ent);
    if (player.cooldown > 0)
    {
        printd("Player is on cooldown -- %f\n", player.cooldown);
        return;
    }
    create_player_projectile(player_ent,  x, y);
    invoke_player_cooldown(player, is_left);
}

void InputHandler::onMouseKey(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (button) {
            case GLFW_MOUSE_BUTTON_LEFT:
                printd("Left mouse button pressed.\n");
                double x, y;
                glfwGetCursorPos(window, &x, &y);
                cast_player_spell(x, y, true);
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
    auto player = registry.players.entities[0];

    auto& motion_registry = registry.motions;
    Motion& playerMotion = motion_registry.get(player);

    int verticalDir = activeMoveKeys.count(GLFW_KEY_S) - activeMoveKeys.count(GLFW_KEY_W);
    int horizontalDir = activeMoveKeys.count(GLFW_KEY_D) - activeMoveKeys.count(GLFW_KEY_A);

    float maxSpeed = 1; // TODO: get this from component

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
    printd("New position is: %f, %f\n", playerMotion.position.x, playerMotion.position.y);
}

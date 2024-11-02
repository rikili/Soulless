#include "input/input_handler.hpp"
#include "entities/general_components.hpp"
#include "utils/angle_functions.hpp"
#include <cstdio>
#include <iostream>
#include "sound/sound_manager.hpp"

#include <SDL.h>
#include <SDL_mixer.h>

/*
- player slots for spell 1 and spell 2 (component) ? (needs to hold some spells)
- handle player movement (physics.cpp?)

- player rendering (ask group)
- how to handle spell being fired ??? (give it a velocity... ask group?)
Mergekk
*/

InputHandler::InputHandler() {}

bool isPlayerDead()
{
    Entity& player_ent = registry.players.entities[0];
    return registry.deaths.has(player_ent);
}

bool isTutorialOn()
{
    return globalOptions.tutorial;
}

bool isPause()
{
    return globalOptions.pause;
}

void InputHandler::onKey(int key, int scancode, int action, int mods)
{
    SoundManager* soundManager = SoundManager::getSoundManager();

    if (isTutorialOn() && key == GLFW_KEY_SPACE)
    {
        if (!globalOptions.pause)
        {
            soundManager->playMusic(Song::MAIN);
        }
        else
        {
            soundManager->toggleMusic();
        }
        globalOptions.tutorial = false;
        globalOptions.pause = false;
        return;
    }

    if (isPlayerDead())
    {
        return;
    }

    if (action == GLFW_PRESS)
    {
        switch (key)
        {
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
            // TODO: NEED A NEW KEY FOR... interact with item on ground (unused for now)
        case GLFW_KEY_F:
            if (mods & GLFW_MOD_SHIFT)
            {
                globalOptions.showFps = !globalOptions.showFps;
            }
            break;
        case GLFW_KEY_T:
            soundManager->toggleMusic();
            globalOptions.tutorial = true;
            globalOptions.pause = true;
            break;
        default:
            break;
        }
    }
    else if (action == GLFW_RELEASE)
    {
        switch (key)
        {
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

void InputHandler::onMouseMove(vec2 mouse_position)
{
    if (isPlayerDead() || isTutorialOn())
    {
        return;
    }

    Entity& player = registry.players.entities[0];
    Motion& playerMotion = registry.motions.get(player);

    Entity& camera = registry.cameras.entities[0];
    Camera& cameraEntity = registry.cameras.get(camera);

    float xNDC = (2.f * mouse_position.x) / window_width_px - 1.f;
    float yNDC = 1.f - (2.f * mouse_position.y) / window_height_px;
    mat4 inverseView = glm::inverse(registry.projectionMatrix * registry.viewMatrix);
    vec4 world = inverseView * vec4(xNDC, yNDC, 0.f, 1.f);

    float dx = world.x - playerMotion.position.x;
    float dy = world.y - playerMotion.position.y;

    // printd("CAMERA: %f, %f\n", cameraEntity.position.x, cameraEntity.position.y);
    // printd("x: %f, y: %f \n", world.x, world.y);

    playerMotion.angle = find_closest_angle(dx, dy);
}

void create_player_projectile(Entity& player_ent, double x, double y)
{

    Entity projectile_ent;
    Projectile& projectile = registry.projectiles.emplace(projectile_ent);
    Motion& projectile_motion = registry.motions.emplace(projectile_ent);
    Deadly& deadly = registry.deadlies.emplace(projectile_ent);
    Damage& damage = registry.damages.emplace(projectile_ent);
    RenderRequest& request = registry.render_requests.emplace(projectile_ent);
    Motion& player_motion = registry.motions.get(player_ent);

    Animation& player_animation = registry.animations.get(player_ent);
    player_animation.state = EntityState::ATTACKING;
    player_animation.frameTime = 30.f;
    player_motion.currentDirection = angleToDirection(player_motion.angle);
    player_animation.initializeAtRow((int)player_motion.currentDirection);

    deadly.to_enemy = true;

    projectile_motion.scale = FIRE_SCALE;
    projectile_motion.collider = FIRE_COLLIDER;
    projectile_motion.position = player_motion.position;
    projectile_motion.angle = player_motion.angle;

    // TODO: change once finalization is needed
    projectile.type = DamageType::fire;
    projectile.range = FIRE_RANGE;
    projectile_motion.velocity = vec2({ cos(player_motion.angle), sin(player_motion.angle) });
    damage.value = FIRE_DAMAGE;

    request.mesh = "sprite";
    request.texture = "fireball";
    request.shader = "sprite";
    request.type = PROJECTILE;
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
        return;
    }

    create_player_projectile(player_ent, x, y);

    SoundManager* soundManager = SoundManager::getSoundManager();
    soundManager->playSound(SoundEffect::FIRE);

    invoke_player_cooldown(player, is_left);
}

void InputHandler::onMouseKey(GLFWwindow* window, int button, int action, int mods)
{
    if (isTutorialOn())
    {
        globalOptions.tutorial = false;
        SoundManager* soundManager = SoundManager::getSoundManager();
        soundManager->playMusic(Song::MAIN);
        return;
    }

    if (isPlayerDead())
    {
        return;
    }

    if (action == GLFW_PRESS)
    {
        switch (button)
        {
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

void InputHandler::updateVelocity()
{
    // TODO: get global player
    // Entity player = player_wizard;
    auto player = registry.players.entities[0];

    auto& motion_registry = registry.motions;
    Motion& playerMotion = motion_registry.get(player);

    int verticalDir = activeMoveKeys.count(GLFW_KEY_S) - activeMoveKeys.count(GLFW_KEY_W);
    int horizontalDir = activeMoveKeys.count(GLFW_KEY_D) - activeMoveKeys.count(GLFW_KEY_A);

    float maxSpeed = 0.5f; // TODO: get this from component

    if (horizontalDir != 0 || verticalDir != 0)
    {
        float normalizedSpeed = maxSpeed;

        if (horizontalDir != 0 && verticalDir != 0)
        {
            normalizedSpeed = sqrt((maxSpeed * maxSpeed) / 2);
        }

        playerMotion.velocity.x = horizontalDir * normalizedSpeed;
        playerMotion.velocity.y = verticalDir * normalizedSpeed;
    }
    else
    {
        playerMotion.velocity.x = 0;
        playerMotion.velocity.y = 0;
    }

    //  printd("New velocity is: %f, %f\n", playerMotion.velocity.x, playerMotion.velocity.y);
    //  printd("New position is: %f, %f\n", playerMotion.position.x, playerMotion.position.y);
}

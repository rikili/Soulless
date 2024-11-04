#include "input/input_handler.hpp"
#include "entities/general_components.hpp"
#include "utils/angle_functions.hpp"
#include "utils/spell_queue.hpp"
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
            printf("FPS: %d\n", globalOptions.fps);
            break;
        case GLFW_KEY_T:
            if (!isTutorialOn() && mods & GLFW_MOD_SHIFT) {
                soundManager->toggleMusic();
                globalOptions.tutorial = true;
                globalOptions.pause = true;
            }
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
            registry.debug = !registry.debug;
            break;
        case GLFW_KEY_J:
            registry.game_over = true;
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

    this->worldMousePosition = vec2(world.x, world.y);

    float dx = world.x - playerMotion.position.x;
    float dy = world.y - playerMotion.position.y;

    // printd("CAMERA: %f, %f\n", cameraEntity.position.x, cameraEntity.position.y);
    // printd("MOUSE x: %f, y: %f \n", world.x, world.y);

    playerMotion.angle = find_closest_angle(dx, dy);
}

void invoke_player_cooldown(Player& player, bool is_left)
{
    player.cooldown = 700;
}

void InputHandler::onMouseKey(GLFWwindow* window, int button, int action, int mods)
{
    if (isTutorialOn()) {
        SoundManager* soundManager = SoundManager::getSoundManager();
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

    // fixes issue where mouse position isn't set until mouse is moved
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    this->onMouseMove({ x, y });

    if (action == GLFW_PRESS)
    {
        switch (button)
        {
        case GLFW_MOUSE_BUTTON_LEFT:
            printd("Left mouse button pressed.\n");
            cast_player_spell(this->worldMousePosition.x, this->worldMousePosition.y, true);
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

SoundEffect convertSpellToSoundEffect(SpellType spellType) {
    switch (spellType) {
        case SpellType::FIRE:
            return SoundEffect::FIRE;
        case SpellType::WATER:
            return SoundEffect::WATER;
        case SpellType::LIGHTNING:
            return SoundEffect::LIGHTNING;
        default:
            throw std::invalid_argument("Unknown SpellType");
    }
}

void InputHandler::cast_player_spell(double x, double y, bool is_left)
{
    Entity& player_ent = registry.players.entities[0];
    Player& player = registry.players.get(player_ent);
    if (player.cooldown > 0)
    {
        return;
    }

    SpellQueue& spell_queue = player.spell_queue;
    SpellType spell = spell_queue.useSpell(is_left);

    create_player_projectile(player_ent, x, y, spell);

    SoundManager* soundManager = SoundManager::getSoundManager();
    soundManager->playSound(convertSpellToSoundEffect(spell));

    invoke_player_cooldown(player, is_left);
}

void InputHandler::create_player_projectile(Entity& player_ent, double x, double y, SpellType spell)
{

    const Entity projectile_ent;
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

    projectile_motion.position = player_motion.position;
    projectile_motion.angle = player_motion.angle;
    projectile_motion.velocity = vec2({ cos(player_motion.angle), sin(player_motion.angle) });

    switch (spell) {
    case SpellType::FIRE:
        deadly.to_enemy = true;
        projectile_motion.scale = FIRE_SCALE;
        projectile_motion.collider = FIRE_COLLIDER;
        projectile.type = DamageType::fire;
        projectile.range = FIRE_RANGE;
        damage.value = FIRE_DAMAGE;
        request.texture = "fireball";
        request.type = PROJECTILE;
        break;
    case SpellType::WATER:
    {
        deadly.to_projectile = true;
        projectile_motion.scale = WATER_SCALE;
        projectile_motion.collider = WATER_COLLIDER;
        projectile.type = DamageType::water;
        projectile.range = WATER_RANGE;
        damage.value = WATER_DAMAGE;
        SpellState& spellState = registry.spellStates.emplace(projectile_ent);
        spellState.state = State::ACTIVE;
        spellState.timer = WATER_LIFETIME;
        request.texture = "barrier";
        request.type = OVER_PLAYER;
        break;
    }
    case SpellType::LIGHTNING:
    {
        // printd("Mouse position when casting lightning: %f, %f\n", x, y); 
        projectile_motion.position = { x, y };
        projectile_motion.angle = 0.f;
        projectile_motion.scale = LIGHTNING_SCALE;
        projectile_motion.collider = LIGHTNING_COLLIDER;
        projectile.type = DamageType::lightning;
        projectile.range = LIGHTNING_RANGE;
        damage.value = LIGHTNING_CASTING_DAMAGE;
        SpellState& spellState = registry.spellStates.emplace(projectile_ent);
        spellState.state = State::CASTING;
        spellState.timer = LIGHTNING_CASTING_LIFETIME;
        request.texture = "lightning1";
        request.type = PROJECTILE;
        break;
    }
    case SpellType::ICE:
        // TODO
        break;
    default: // Should not happen
        break;
    }

    request.mesh = "sprite";
    request.shader = "sprite";
}

void InputHandler::invoke_player_cooldown(Player& player, bool is_left)
{
    player.cooldown = 700;
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

    if (horizontalDir != 0 || verticalDir != 0)
    {
        float normalizedSpeed = PLAYER_VELOCITY;

        if (horizontalDir != 0 && verticalDir != 0)
        {
            normalizedSpeed = sqrt((PLAYER_VELOCITY * PLAYER_VELOCITY) / 2);
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

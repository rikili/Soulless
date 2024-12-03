#include "input/input_handler.hpp"
#include "entities/general_components.hpp"
#include "utils/angle_functions.hpp"
#include "utils/spell_queue.hpp"
#include <cstdio>
#include <iostream>
#include "sound/sound_manager.hpp"

#include <SDL.h>
#include <SDL_mixer.h>

#include "isystems/IRenderSystem.hpp"
#include "utils/serializer.hpp"

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
    return registry.deaths.has(player_ent) || registry.game_over;
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

    if (action == GLFW_PRESS && key == GLFW_KEY_L && (globalOptions.pause || globalOptions.tutorial) && !globalOptions.loadingOldGame) {
        // printd("Loading game\n");
        // globalOptions.loadingOldGame = true;
    }

    if (action == GLFW_PRESS && key == GLFW_KEY_S && (globalOptions.pause || globalOptions.tutorial) && !globalOptions.loadingOldGame && !registry.deaths.has(registry.players.entities[0])) {
        // printd("Saving game\n");
        // Serializer::serialize();
    }


    if (isTutorialOn() || globalOptions.pause)
    {
        if (key == GLFW_KEY_SPACE && !globalOptions.loadingOldGame && action == GLFW_PRESS) {
            // if (!globalOptions.pause)
            // {
            //     soundManager->playMusic(Song::MAIN);
            // }
            // else
            // {
            //     soundManager->toggleMusic();
            // }

            globalOptions.tutorial = false;
            if (!renderer->isPlayingVideo() && !globalOptions.introPlayed)
            {
                this->renderer->playCutscene("intro.mp4", Song::INTRO);
                globalOptions.introPlayed  = true;
                return;
            }
            if (globalOptions.introPlayed)
            {
                globalOptions.pause = false;
                globalOptions.tutorial = false;
                soundManager->toggleMusic();
            }

            if (renderer->isPlayingVideo())
            {
                renderer->stopVideo();
                globalOptions.tutorial = false;
                if (!globalOptions.bossdefeatScene)
                {
                    globalOptions.pause = false;
                }
            }
        }

        if (key == GLFW_KEY_1 || key == GLFW_KEY_2 || key == GLFW_KEY_3 || key == GLFW_KEY_4)
        {
            globalOptions.showingTab = key - GLFW_KEY_0;
        }

        return;
    }

    if (isPlayerDead() || globalOptions.pause)
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
            drop_player_spell(true);
            break;
        case GLFW_KEY_E:
            printd("E button pressed.\n");
            drop_player_spell(false);
            break;
            // TODO: NEED A NEW KEY FOR... interact with item on ground (unused for now)
        case GLFW_KEY_R:
            switch (interactProx.in_proximity)
            {
            case Proximity::BOSS_ALTAR:
            {
                printd("Interact - spawning dark lord\n");
                enemySpawnTimers.darklord = true;
                break;
            }
            case Proximity::PLASMA_SUMMON:
            {
                printd("Interact - plasma request\n");
                instanceEvents.activate_plasma_altar = true;
                break;
            }
            }
            break;
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
        case GLFW_KEY_P:
            globalOptions.debugSpellSpawn = true;
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

    // Entity& camera = registry.cameras.entities[0];
    // Camera& cameraEntity = registry.cameras.get(camera);

    float xNDC = (2.f * mouse_position.x) / window_width_px - 1.f;
    float yNDC = 1.f - (2.f * mouse_position.y) / window_height_px;
    mat4 inverseView = glm::inverse(registry.projectionMatrix * registry.viewMatrix);
    vec4 world = inverseView * vec4(xNDC, yNDC, 0.f, 1.f);

    this->worldMousePosition = vec2(world.x, world.y);

    float dx = world.x - playerMotion.position.x;
    float dy = world.y - playerMotion.position.y;

    // printd("CAMERA: %f, %f\n", cameraEntity.position.x, cameraEntity.position.y);
    // printd("MOUSE x: %f, y: %f \n", world.x, world.y);

    playerMotion.angle = atan2(dy, dx);
}

void InputHandler::onMouseKey(GLFWwindow* window, int button, int action, int mods)
{
    if (isTutorialOn() && !globalOptions.loadingOldGame) {
        // SoundManager* soundManager = SoundManager::getSoundManager();
        // if (!globalOptions.pause)
        // {
        //     soundManager->playMusic(Song::MAIN);
        // }
        // else
        // {
        //     soundManager->toggleMusic();
        // }
        // globalOptions.tutorial = false;
        // globalOptions.pause = false;
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
            // printd("Left mouse button pressed.\n");
            cast_player_spell(this->worldMousePosition.x, this->worldMousePosition.y, true);
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            // printd("Right mouse button pressed.\n");
            cast_player_spell(this->worldMousePosition.x, this->worldMousePosition.y, false);
            break;
        default:
            break;
        }
    }
}

void InputHandler::reset()
{
    activeMoveKeys.clear();
}

void InputHandler::drop_player_spell(bool is_left) {
    Entity& player_ent = registry.players.entities[0];
    Player& player = registry.players.get(player_ent);
    if ((is_left && player.leftCooldown) || (!is_left && player.rightCooldown)) {
        return;
    }

    SpellQueue& spell_queue = player.spell_queue;
    spell_queue.discardSpell(is_left);

    Health& health = registry.healths.get(player_ent);
    health.health = std::min(PLAYER_HEALTH, health.health + PLAYER_HEAL_AMOUNT);

    SoundManager* soundManager = SoundManager::getSoundManager();
    soundManager->playSound(SoundEffect::DISCARD_SPELL);

    invoke_player_cooldown(player, is_left, true);
}


void InputHandler::cast_player_spell(double x, double y, bool is_left)
{
    Entity& player_ent = registry.players.entities[0];
    Player& player = registry.players.get(player_ent);
    if ((is_left && player.leftCooldown) || (!is_left && player.rightCooldown)) {
        return;
    }

    SpellQueue& spell_queue = player.spell_queue;
    std::pair<SpellType, int> spell = spell_queue.useSpell(is_left);

    SpellFactory::createSpellProjectile(registry, player_ent, spell.first, spell.second, x, y);

    invoke_player_cooldown(player, is_left, false);
}

void InputHandler::invoke_player_cooldown(Player& player, bool is_left, bool is_heal)
{

    float cooldown = is_heal ? PLAYER_HEAL_COOLDOWN : PLAYER_SPELL_COOLDOWN;

    if (is_left)
    {
        player.leftCooldown = cooldown;
        player.leftCooldownTotal = cooldown;
    }
    else
    {
        player.rightCooldown = cooldown;
        player.rightCooldownTotal = cooldown;
    }
}

void InputHandler::updateVelocity()
{
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



void InputHandler::setRenderer(IRenderSystem* renderer)
{
    this->renderer = renderer;
}

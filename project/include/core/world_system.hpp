#pragma once
#include "isystems/IWorldSystem.hpp"
#include "isystems/IRenderSystem.hpp"
#include "isystems/IInputHandler.hpp"
#include "graphics/particle_system.hpp"
#include "collision_system.hpp"
#include "core/common.hpp"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>
#include <queue>

class WorldSystem final : public IWorldSystem {
public:
   explicit WorldSystem(IRenderSystem* renderer);
   ~WorldSystem() override;

   bool step(float elapsed_ms) override;
   bool isOver() const override;
   void initialize() override;
   void restartGame() override;
   void reloadGame() override;

   void handleAI(float elapsed_ms) override;
   void handleProjectiles(float elapsed_ms) override;
   void handleTimers(float elapsed_ms) override;
   void handleSpellStates(float elapsed_ms) override;
   void handleMovements(float elapsed_ms) override;
   void handleHealthBars() override;
   void handleAnimations() override;

   Entity getPlayer() const override;
   void setRenderer(IRenderSystem* renderer) override;

   void createTileGrid();

private:
   void loadBackgroundObjects();
   Entity createPlayer();
   void computeNewDirection(Entity e);
   void createEnemy(EnemyType type, vec2 position, vec2 velocity);
   Entity createBackgroundObject(vec2 position, vec2 scale, AssetId texture, bool animate);
   void handle_enemy_logic(float elapsed_ms_since_last_update);
   std::string peToString(Entity e);
   void handleRain();
   void handleCollectible(const float elapsed_ms_since_last_update);
   void createCollectible(const vec2 position, const SpellType);
   void createAltar();
   void createPlasmaAltar();
   void handleInteractable();
   void removeInteractable(InteractableType type);

   // Mix_Music* background_music;
   GLFWwindow* window{};
   IRenderSystem* renderer;
   CollisionSystem* collision_system;
   // IInputHandler* input_handler;
   Entity player_mage;
   ParticleSystem particleSystem;
   float powerup_timer;

   bool did_boss_spawn = false;
   bool did_plasma_altar_spawn = false;
   float boss_music_delay_timer = 0;
   bool bossDefeated = false;
   
   std::queue<vec2> lightnings_to_create; // positions to source lightning from, only for MAX lightning
};

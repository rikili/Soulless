#pragma once
#include "isystems/IWorldSystem.hpp"
#include "isystems/IRenderSystem.hpp"
#include "isystems/IInputHandler.hpp"
#include "collision_system.hpp"
#include "core/common.hpp"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

class WorldSystem final : public IWorldSystem {
public:
   explicit WorldSystem(IRenderSystem* renderer);
   ~WorldSystem() override;

   bool step(float elapsed_ms) override;
   bool isOver() const override;
   void initialize() override;
   void restartGame() override;
   
   void handleAI(float elapsed_ms) override;
   void handleProjectiles(float elapsed_ms) override;
   void handleTimers(float elapsed_ms) override;
   void handleSpellStates(float elapsed_ms) override;
   void handleMovements(float elapsed_ms) override;
   void handleHealthBars() override;
   void handleAnimations() override;

   Entity getPlayer() const override;
   void setRenderer(IRenderSystem* renderer) override;
   
   float getFarmerSpawnTimer() const override { return farmer_spawn_timer; }
   float getArcherSpawnTimer() const override { return archer_spawn_timer; }
   float getKnightSpawnTimer() const override { return knight_spawn_timer; }
   void setSpawnTimers(float farmer, float archer, float knight) override;

   void createTileGrid();

private:
   void loadBackgroundObjects();
   Entity createPlayer();
   void computeNewDirection(Entity e);
   void createEnemy(EnemyType type, vec2 position, vec2 velocity);
   void createFarmer(vec2 position, vec2 velocity);
   void createArcher(vec2 position, vec2 velocity);
   void createKnight(vec2 position, vec2 velocity);
   Entity createBackgroundObject(vec2 position, vec2 scale, AssetId texture, bool animate);
   void handle_enemy_logic(float elapsed_ms_since_last_update);

   // Mix_Music* background_music;
   GLFWwindow* window{};
   IRenderSystem* renderer;
   CollisionSystem* collision_system;
   // IInputHandler* input_handler;
   Entity player_mage;

   float farmer_spawn_timer = 0.0f;
   float archer_spawn_timer = 60000.0f;
   float knight_spawn_timer = 120000.0f;
};
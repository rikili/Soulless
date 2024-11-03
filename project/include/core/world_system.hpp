#pragma once

#include "collision_system.hpp"
#include "core/common.hpp"
#include "render_system.hpp"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

/**
 * Container for all our entities and game logic.
 */
class WorldSystem
{
public:
	explicit WorldSystem(RenderSystem* renderer);
	~WorldSystem();
	bool step(float elapsed_ms);
	bool is_over() const;
	void set_renderer(RenderSystem* renderer);
	void handle_projectiles(float elapsed_ms_since_last_update);
	void handle_timers(float elapsed_ms_since_last_update);
	void handle_movements(float elapsed_ms_since_last_update);
	void handle_enemy_attacks(float elapsed_ms_since_last_update);
	void handle_health_bars();
	void create_enemy_projectile(Entity& enemy_ent);
	void invoke_enemy_cooldown(Entity& enemy_ent);
	void initialize();
	void restartGame();
	Entity getPlayer();

	// World state specific variables
	float farmer_spawn_timer = 0.0f;
	float archer_spawn_timer = 60000.0f; // Spawns in at 1 minute
	float knight_spawn_timer = 120000.0f; // Spawns in at 2 minutes

private:
	// helper functions to set the world
	void loadBackgroundObjects();

	Entity createPlayer();
	void createEnemy(EnemyType type, vec2 position, vec2 velocity);
	void createFarmer(vec2 position, vec2 velocity);
	void createArcher(vec2 position, vec2 velocity);
	void createKnight(vec2 position, vec2 velocity);
	Entity createBackgroundObject(vec2 position, vec2 scale, AssetId texture, bool animate);
	void handle_enemy_logic(float elapsed_ms_since_last_update);


	Mix_Music* background_music;

	GLFWwindow* window{};
	RenderSystem* renderer;
	CollisionSystem* collision_system;
	InputHandler inputHandler;
	Entity player_mage;
};

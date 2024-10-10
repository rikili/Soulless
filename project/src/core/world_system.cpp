#include "core/world_system.hpp"

#include "entities/ecs_registry.hpp"

WorldSystem::WorldSystem(RenderSystem* renderer)
{
	this->renderer = renderer;
	this->collision_system = new CollisionSystem(renderer);
}

WorldSystem::~WorldSystem() {}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {

	this->handle_projectiles(elapsed_ms_since_last_update);
	this->handle_timers(elapsed_ms_since_last_update);
	this->handle_movements(elapsed_ms_since_last_update);
	this->collision_system->handle_collisions();

	this->handleEnemyLogic(elapsed_ms_since_last_update);

	return true;
}

void WorldSystem::set_renderer(RenderSystem* renderer)
{
	this->renderer = renderer;
}

/**
 * @brief Handle projectiles to reduce their range at each step and mark for deletion if they are out of range
 * @param 
 */
void WorldSystem::handle_projectiles(float elapsed_ms_since_last_update)
{
	for (Entity& projectile_ent : registry.projectiles.entities)
	{
		if (registry.deaths.has(projectile_ent))
		{
			continue;
		}

		Projectile& projectile = registry.projectiles.get(projectile_ent);
		Motion& motion = registry.motions.get(projectile_ent);
		projectile.range -= sqrt(motion.velocity.x * motion.velocity.x + motion.velocity.y * motion.velocity.y) * elapsed_ms_since_last_update;
		if (projectile.range <= 0)
		{
			registry.deaths.emplace(projectile_ent);
			// printd("Marked for removal due to distance travelled - Entity value: %u\n", static_cast<unsigned>(projectile_ent));
		}
	}
}

/**
 * @brief In charge of updating the position of all entities with a motion component
 * @param elapsed_ms_since_last_update
 */
void WorldSystem::handle_movements(float elapsed_ms_since_last_update)
{
	ComponentContainer<Motion> &motions_registry = registry.motions;
	// Update all motions
	for (Entity entity : motions_registry.entities) {
		Motion& motion = motions_registry.get(entity);
		motion.position += motion.velocity * elapsed_ms_since_last_update;
		if (registry.render_requests.has(entity)) {
			RenderRequest& render_request = registry.render_requests.get(entity);
			render_request.smooth_position.update(motion.position.y);
		}
	}
}

/**
 * @brief In charge of updating timers and their side effects
 * @param elapsed_ms_since_last_update
 */
void WorldSystem::handle_timers(float elapsed_ms_since_last_update)
{
	for (Entity& hit_ent : registry.onHits.entities)
	{
		OnHit& hit = registry.onHits.get(hit_ent);
		hit.invincibility_timer -= elapsed_ms_since_last_update;
		if (hit.invincibility_timer < 0)
		{
			registry.onHits.remove(hit_ent);
		}
	}

	for (Entity& dead_ent : registry.deaths.entities)
	{
		Death& death = registry.deaths.get(dead_ent);
		death.timer -= elapsed_ms_since_last_update;
		if (death.timer < 0)
		{
			// TODO: add custom player death
			if (!registry.players.has(dead_ent))
			{
				registry.remove_all_components_of(dead_ent);
			}
		}
	}

	for (Entity& player_ent : registry.players.entities)
	{
		Player& player = registry.players.get(player_ent);
		player.cooldown -= elapsed_ms_since_last_update;
		if (player.cooldown < 0)
		{
			player.cooldown = 0;
		}
	}
}

/**
 * Initialize the game world
 */
void WorldSystem::initialize()
{
	// Create a player
	player_mage = this->createPlayer();
}


Entity WorldSystem::createPlayer() {
	auto player = Entity();

	registry.players.emplace(player);
	Motion& motion = registry.motions.emplace(player);
	motion.position = { window_width_px / 2.0f, window_height_px / 2.0f };  // Center of the screen
	motion.velocity = { 0.0f, 0.0f };
	motion.scale = { 0.5f, 0.5f };

	Health& health = registry.healths.emplace(player);
	health.health = 100;
	health.maxHealth = 100;
	// TODO: Add resistances here!

	// Player& player_component = registry.players.emplace(player);
	// // TODO: Add player initialization code here!

	RenderRequest& request = registry.render_requests.emplace(player);
	request.mesh = "sprite";
	request.texture = "mage";
	request.shader = "sprite";
	request.type = PLAYER;

	return player;
}


void WorldSystem::createEnemy(vec2 position, vec2 velocity)
{
	Entity enemy;
	Motion& motion = registry.motions.emplace(enemy);
	motion.position = position;
	motion.velocity = velocity;
	motion.scale = { 0.7f, 0.7f };

	Enemy& enemy_component = registry.enemies.emplace(enemy);
	enemy_component.range = 500.f;

	Health& health = registry.healths.emplace(enemy);
	health.health = 100;
	health.maxHealth = 100;

	Deadly& deadly = registry.deadlies.emplace(enemy);
	deadly.to_projectile = true;
	deadly.to_enemy = false;
	deadly.to_player = false;

	Damage& damage = registry.damages.emplace(enemy);
	damage.value = 10.f;
	damage.type = DamageType::enemy;

	RenderRequest& request = registry.render_requests.emplace(enemy);
	request.mesh = "sprite";
	request.texture = "archer";
	request.shader = "sprite";
	request.type = ENEMY;
}

/**
 * @brief Handles the logic for spawning enemies and their movement direction towards the player
 * @param elapsed_ms_since_last_update
 * @return void
 * If the enemy spawn timer has elapsed, a new enemy is spawned at a random location
 * Enemies are spawned outside the window and move towards the player
 */
void WorldSystem::handleEnemyLogic(const float elapsed_ms_since_last_update)
{
	this->enemy_spawn_timer -= elapsed_ms_since_last_update;
	const bool should_spawn = this->enemy_spawn_timer <= 0;
	if (should_spawn)
	{
		this->enemy_spawn_timer = ENEMY_SPAWN_INTERVAL_MS; // Reset the timer

		std::random_device rd;  // Random device
		std::mt19937 gen(rd()); // Mersenne Twister generator
		enum SIDE { TOP, RIGHT, BOTTOM, LEFT }; // Side of the window to spawn from
		std::uniform_int_distribution<int> side_dis(TOP, LEFT);
		const int side = side_dis(gen);

		float candidate_x = 0.f, candidate_y = 0.f;
		std::uniform_real_distribution<float> dis(0.f, 1.f);
		constexpr float offset_x = window_width_px / 10.f;
		constexpr float offset_y = window_height_px / 10.f;

		switch(side) {
		case TOP:
			candidate_x = dis(gen) * window_width_px;
			candidate_y = -offset_y;
			break;
		case RIGHT:
			candidate_x = window_width_px + offset_x;
			candidate_y = dis(gen) * window_height_px;
			break;
		case BOTTOM:
			candidate_x = dis(gen) * window_width_px;
			candidate_y = window_height_px + offset_y;
			break;
		case LEFT:
		default: // Should never happen but just in case
			candidate_x = -offset_x;
			candidate_y = dis(gen) * window_height_px;
			break;
		}
		const vec2 position = { candidate_x, candidate_y };
		this->createEnemy(position, { 0, 0 });
	}

	// Reorient enemies towards the player
	for (const auto& enemy : registry.enemies.entities)
	{
		Motion& motion = registry.motions.get(enemy);
		const Enemy& enemy_component = registry.enemies.get(enemy);
		const vec2* position = &motion.position;
		const vec2 des = registry.motions.get(player_mage).position;
		vec2 distance = { des.x - position->x, des.y - position->y };
		if (enemy_component.range <= sqrt(distance.x * distance.x + distance.y * distance.y))
		{
			const vec2 velocity = glm::normalize(distance) * ENEMY_BASIC_VELOCITY;
			motion.velocity = velocity;
		} else {
			motion.velocity = { 0, 0 };
		}

	}
}
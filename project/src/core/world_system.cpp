#include "core/world_system.hpp"

#include "entities/ecs_registry.hpp"
#include "sound/sound_manager.hpp"
#include "utils/isometric_helper.hpp"
#include "graphics/tile_generator.hpp"

WorldSystem::WorldSystem(IRenderSystem* renderer)
{
	this->renderer = renderer;
	this->collision_system = new CollisionSystem(renderer);
}

WorldSystem::~WorldSystem() {}

// Should the game be over ?
bool WorldSystem::isOver() const {
	return bool(glfwWindowShouldClose(window));
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	if (!registry.players.has(player_mage) || registry.game_over) {
		printd("\n----------------\nGame Over! Resetting...\n----------------\n");
		registry.game_over = false;
		registry.clear_all_components();
		this->restartGame();
		return true;
	}

	this->handleProjectiles(elapsed_ms_since_last_update);
	this->handle_enemy_logic(elapsed_ms_since_last_update);
	this->handleMovements(elapsed_ms_since_last_update);
	this->collision_system->detect_collisions();
	this->collision_system->resolve_collisions();
	this->handleAnimations();
	this->handleHealthBars();
	this->handleTimers(elapsed_ms_since_last_update);
	this->handleAI(elapsed_ms_since_last_update);
	this->handleSpellStates(elapsed_ms_since_last_update);
	registry.collision_registry.clear_collisions();
	return true;
}

void WorldSystem::handleAI(float elapsed_ms_since_last_update) {

	if (registry.game_over) {
		return;
	}

	for (Entity& entity : registry.ai_systems.entities) {
		AI_SYSTEM::tickForEntity(&entity, elapsed_ms_since_last_update);
	}
}

void WorldSystem::setRenderer(IRenderSystem* renderer)
{
	this->renderer = renderer;
}

void WorldSystem::handleHealthBars() {

	// For each healthbar
	for (Entity& entity : registry.healthBars.entities) {

		// Needed to get entity to which the healthbar is assigned
		HealthBar& healthbar = registry.healthBars.get(entity);

		// Check to ensure healthbar is assigned and has been given a position
		if (healthbar.assigned && registry.motions.has(entity)) {

			// Update healthbar position to match the entity it's assigned to
			Entity assignedTo = healthbar.assignedTo;
			if (!registry.motions.has(assignedTo)) {
				printd("Error updating health bar: it is assigned to an entity without a motion component\n");
				return;
			}
			registry.motions.get(entity).position.x = registry.motions.get(assignedTo).position.x;
			registry.motions.get(entity).position.y = registry.motions.get(assignedTo).position.y + HEALTH_BAR_Y_OFFSET;
		}
		else {
			printd("Error updating health bar: it is either unassigned or does not have a motion component.\n");
		}
	}
}


void WorldSystem::handleAnimations() {
	Motion& playerMotion = registry.motions.get(player_mage);
	Animation& playerAnimation = registry.animations.get(player_mage);
	RenderRequest& playerRR = registry.render_requests.get(player_mage);

	if (playerAnimation.state == EntityState::ATTACKING) {
		playerAnimation.oneTime = true;
		playerRR.texture = "mage-attack";
	}
	else {

		if (playerMotion.velocity.x == 0 && playerMotion.velocity.y == 0) {
			playerRR.texture = "mage-idle";
		}
		else {
			playerRR.texture = "mage-walk";
		}

		if (playerMotion.currentDirection == playerMotion.oldDirection) {
			return;
		}

		playerAnimation.initializeAtRow((int)playerMotion.currentDirection);
	}

	// printd("Current: %d\n", playerMotion.currentDirection);



}
/**
 * @brief Handle projectiles to reduce their range at each step and mark for
 * deletion if they are out of range
 * @param
 */
void WorldSystem::handleProjectiles(float elapsed_ms_since_last_update)
{
	for (Entity& projectile_ent : registry.projectiles.entities)
	{
		if (registry.deaths.has(projectile_ent))
		{
			continue;
		}

		Projectile& projectile = registry.projectiles.get(projectile_ent);
		Motion& motion = registry.motions.get(projectile_ent);
		Deadly& deadly = registry.deadlies.get(projectile_ent);
		projectile.range -= sqrt(motion.velocity.x * motion.velocity.x + motion.velocity.y * motion.velocity.y) * elapsed_ms_since_last_update;

		if (deadly.to_enemy && projectile.type == DamageType::fire)
		{
			vec2 scale_factor = FIRE_SCALE + ((FIRE_RANGE - projectile.range) / FIRE_RANGE) * (FIRE_SCALE_FACTOR * FIRE_SCALE - FIRE_SCALE);
			motion.scale.x = scale_factor.x;
			motion.scale.y = scale_factor.y;
		}

		if (projectile.range <= 0)
		{
			registry.deaths.emplace(projectile_ent);
			// printd("Marked for removal due to distance travelled - Entity value:
			// %u\n", static_cast<unsigned>(projectile_ent));
		}
	}
}

/**
 * @brief In charge of updating the position of all entities with a motion
 * component
 * @param elapsed_ms_since_last_update
 */
void WorldSystem::handleMovements(float elapsed_ms_since_last_update)
{
	ComponentContainer<Motion>& motions_registry = registry.motions;
	Motion& player_motion = motions_registry.get(player_mage);

	// Update all motions
	for (Entity entity : motions_registry.entities)
	{
		Motion& motion = motions_registry.get(entity);

		if (registry.players.has(entity) || registry.enemies.has(entity))
		{
			float x_offset = motion.collider.x * motion.scale.x;
			float y_offset = motion.collider.y * motion.scale.y;
			motion.position = glm::clamp(motion.position + motion.velocity * elapsed_ms_since_last_update, { x_offset, y_offset }, { window_width_px - x_offset, window_height_px - y_offset });
		}

		// not a player nor enemy
		else if (registry.projectiles.has(entity)) {
			Projectile& projectile = registry.projectiles.get(entity);

			if (projectile.type == DamageType::water) {
				motion.position = player_motion.position;
			}
			else if (projectile.type == DamageType::lightning) {
				// lightning attack doesn't move
			}
			else {
				motion.position += motion.velocity * elapsed_ms_since_last_update;
			}
		}

		if (registry.enemies.has(entity))
		{
			// Enemy& enemy = registry.enemies.get(entity);
			motion.angle = atan2(player_motion.position.y - motion.position.y,
				player_motion.position.x - motion.position.x);
			// printd("Enemy angle towards player: %f\n", motion.angle);
		}

		// Only player for now, can be expanded
		if (registry.players.has(entity)) {
			computeNewDirection(entity);
		}

		RenderRequest& render_request = registry.render_requests.get(entity);
		render_request.smooth_position.update(motion.position.y);
	}
}

void WorldSystem::computeNewDirection(Entity e) {
	Motion& motion = registry.motions.get(e);
	motion.oldDirection = motion.currentDirection;


	float xVel = motion.velocity.x;
	float yVel = motion.velocity.y;

	if (xVel == 0 && yVel == 0) {
		return;
	}

	if (xVel == 0) {
		if (yVel < 0) {
			motion.currentDirection = Direction::N;
		}
		else {
			motion.currentDirection = Direction::S;
		}
		return;
	}

	if (yVel == 0) {
		if (xVel > 0) {
			motion.currentDirection = Direction::E;
		}
		else {
			motion.currentDirection = Direction::W;
		}
		return;
	}

	if (xVel < 0) {
		if (yVel < 0) {
			motion.currentDirection = Direction::NW;
		}
		else {
			motion.currentDirection = Direction::SW;
		}
		return;
	}

	if (xVel > 0) {
		if (yVel < 0) {
			motion.currentDirection = Direction::NE;
		}
		else {
			motion.currentDirection = Direction::SE;
		}
		return;
	}

}
/**
 * @brief In charge of updating timers and their side effects
 * @param elapsed_ms_since_last_update
 */
void WorldSystem::handleTimers(float elapsed_ms_since_last_update)
{

	for (Entity& hit_ent : registry.onHits.entities)
	{
		OnHit& hit = registry.onHits.get(hit_ent);
		hit.invincibility_timer -= elapsed_ms_since_last_update;
		if (hit.invincibility_timer < PLAYER_INVINCIBILITY_TIMER - 200.f) {
			hit.invicibilityShader = true;

			if (hit.invincibility_timer < 0)
			{
				registry.onHits.remove(hit_ent);
			}
		}
	}

	for (Entity& healed_ent : registry.onHeals.entities)
	{
		OnHeal& heal = registry.onHeals.get(healed_ent);
		heal.heal_time -= elapsed_ms_since_last_update;
		if (heal.heal_time < 0)
		{
			registry.onHeals.remove(healed_ent);
		}
	}

	for (Entity& dead_ent : registry.deaths.entities)
	{
		Death& death = registry.deaths.get(dead_ent);
		death.timer -= elapsed_ms_since_last_update;
		if (death.timer < 0)
		{
			if (registry.players.has(dead_ent))
			{
				registry.game_over = true;
			}
			registry.remove_all_components_of(dead_ent);
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

	for (Entity& enemy_ent : registry.enemies.entities)
	{
		Enemy& enemy = registry.enemies.get(enemy_ent);
		enemy.cooldown -= elapsed_ms_since_last_update;
		if (enemy.cooldown < 0)
		{
			// printd("Enemy cooldown is less than 0\n");
			enemy.cooldown = 0;
		}
	}
}

/**
 * @brief Handle spell states. Update spells and their states based on timers.
 * @param elapsed_ms_since_last_update
 */
void WorldSystem::handleSpellStates(float elapsed_ms_since_last_update)
{
	for (Entity& spell_ent : registry.spellStates.entities) {
		SpellState& spell_state = registry.spellStates.get(spell_ent);
		RenderRequest& request = registry.render_requests.get(spell_ent);
		Damage& damage = registry.damages.get(spell_ent);
		Projectile& projectile = registry.projectiles.get(spell_ent);
		Deadly& deadly = registry.deadlies.get(spell_ent);

		spell_state.timer -= elapsed_ms_since_last_update;

		// explicit state change when timer is done
		if (spell_state.timer <= 0) {

			State current_state = spell_state.state;

			switch (current_state) {
			case State::CASTING: {
				spell_state.state = State::CHARGING;

				if (projectile.type == DamageType::lightning) {
					request.texture = "lightning2";
					spell_state.timer = LIGHTNING_CHARGING_LIFETIME;
				}
				break;
			}
			case State::CHARGING: {
				spell_state.state = State::ACTIVE;

				if (projectile.type == DamageType::lightning) {
					deadly.to_enemy = true;
					damage.value = LIGHTNING_ACTIVE_DAMAGE;
					request.texture = "lightning3";
					spell_state.timer = LIGHTNING_ACTIVE_LIFETIME;
				}
				break;
			}
			case State::ACTIVE: {
				spell_state.state = State::COMPLETE;
				break;
			}
			case State::COMPLETE: {
				if (!registry.deaths.has(spell_ent))
				{
					registry.deaths.emplace(spell_ent);
				}
				break;
			}
			default: {
				break;
			}
			}

		}
	}
}


/**
 * Initialize the game world
 */
void WorldSystem::initialize() {
	restartGame();
}

void WorldSystem::restartGame() {
	if (registry.players.entities.size() > 0)
	{
		registry.clear_all_components();
	}
	if (!globalOptions.tutorial) {
		SoundManager* soundManager = SoundManager::getSoundManager();
		soundManager->playMusic(Song::MAIN);
	}
	player_mage = this->createPlayer();
	this->createTileGrid();
	loadBackgroundObjects();
	this->renderer->initializeCamera();

	// Reset enemy spawn timers (rework this if needed)
	archer_spawn_timer = 60000.f;
	knight_spawn_timer = 120000.f;
}

void WorldSystem::createTileGrid() {
	int w, h;
	glfwGetFramebufferSize(renderer->getGLWindow(), &w, &h);
	vec2 gridDim = IsometricGrid::getGridDimensions(w, h);
	int numCols = static_cast<int>(gridDim.x);
	int numRows = static_cast<int>(gridDim.y);
	BatchRenderer *batchRenderer = new BatchRenderer();
	TileGenerator tileGenerator(numCols, numRows, w, h, true);
	tileGenerator.generateTiles(batchRenderer);
	renderer->addSubRenderer("tiles", batchRenderer);
}

Entity WorldSystem::createPlayer() {
	auto player = Entity();

	Player& player_component = registry.players.emplace(player);
	player_component.spell_queue = SpellQueue();


	Motion& motion = registry.motions.emplace(player);
	motion.position = { window_width_px / 2.0f,
										 window_height_px / 2.0f }; // Center of the screen
	motion.velocity = { 0.0f, 0.0f };
	motion.scale = { 1.f, 1.f };

	Health& health = registry.healths.emplace(player);
	health.health = PLAYER_HEALTH;
	health.maxHealth = PLAYER_MAX_HEALTH;

	auto healthBar = Entity();
	Motion& healthBarMotion = registry.motions.emplace(healthBar);
	healthBarMotion.position = { window_width_px / 2.0f, window_height_px / 2.0f + HEALTH_BAR_Y_OFFSET };
	healthBarMotion.scale = { 0.5, 0.5 };

	HealthBar& healthBarComp = registry.healthBars.emplace(healthBar);
	healthBarComp.assignHealthBar(player);
	// TODO: Add resistances here!

	Animation& animation = registry.animations.emplace(player);
	animation.spriteCols = 15;
	animation.spriteRows = 8;
	animation.spriteCount = 120;
	animation.frameCount = 15;
	animation.initializeAtFrame(0.0f);

	RenderRequest& request = registry.render_requests.emplace(player);
	request.mesh = "sprite";
	request.texture = "mage-idle";
	request.shader = "animatedsprite";
	request.type = PLAYER;

	RenderRequest& healthBarRequest = registry.render_requests.emplace(healthBar);
	healthBarRequest.mesh = "sprite";
	healthBarRequest.texture = "healthbar";
	healthBarRequest.shader = "healthbar";
	healthBarRequest.type = PLAYER;

	MeshCollider& collider = registry.mesh_colliders.emplace(player);
	collider.mesh = "mage_collider";

	return player;
}


void WorldSystem::createEnemy(EnemyType type, vec2 position, vec2 velocity)
{
	EnemyType enemy_type = type;

	switch (enemy_type) {
	case EnemyType::FARMER:
		createFarmer(position, velocity);
		break;
	case EnemyType::ARCHER:
		createArcher(position, velocity);
		break;
	case EnemyType::KNIGHT:
		createKnight(position, velocity);
		break;
	}
}



void WorldSystem::createFarmer(vec2 position, vec2 velocity)
{
	Entity enemy;

	Enemy& enemy_component = registry.enemies.emplace(enemy);
	enemy_component.type = EnemyType::FARMER;
	enemy_component.range = FARMER_RANGE;
	enemy_component.cooldown = FARMER_COOLDOWN;

	Motion& motion = registry.motions.emplace(enemy);
	motion.position = position;
	motion.velocity = velocity;
	motion.scale = { 0.5f, 0.5f };

	Health& health = registry.healths.emplace(enemy);
	health.health = FARMER_HEALTH;
	health.maxHealth = FARMER_HEALTH;

	Deadly& deadly = registry.deadlies.emplace(enemy);
	deadly.to_projectile = true;
	deadly.to_player = true;

	Damage& damage = registry.damages.emplace(enemy);
	damage.value = FARMER_DAMAGE;

	RenderRequest& request = registry.render_requests.emplace(enemy);
	request.mesh = "sprite";
	request.texture = "farmer";
	request.shader = "sprite";
	request.type = ENEMY;

	AI_SYSTEM::initAIComponent(&enemy);
}

void WorldSystem::createArcher(vec2 position, vec2 velocity)
{
	Entity enemy;

	Enemy& enemy_component = registry.enemies.emplace(enemy);
	enemy_component.type = EnemyType::ARCHER;
	enemy_component.range = ARCHER_RANGE;
	enemy_component.cooldown = ARCHER_COOLDOWN;

	Motion& motion = registry.motions.emplace(enemy);
	motion.position = position;
	motion.velocity = velocity;
	motion.scale = { 0.5f, 0.5f };

	Health& health = registry.healths.emplace(enemy);
	health.health = ARCHER_HEALTH;
	health.maxHealth = ARCHER_HEALTH;

	Deadly& deadly = registry.deadlies.emplace(enemy);
	deadly.to_projectile = true;

	Damage& damage = registry.damages.emplace(enemy);
	damage.value = ARCHER_DAMAGE;

	RenderRequest& request = registry.render_requests.emplace(enemy);
	request.mesh = "sprite";
	request.texture = "archer";
	request.shader = "sprite";
	request.type = ENEMY;

	AI_SYSTEM::initAIComponent(&enemy);
}

void WorldSystem::createKnight(vec2 position, vec2 velocity)
{
	Entity enemy;

	Enemy& enemy_component = registry.enemies.emplace(enemy);
	enemy_component.type = EnemyType::KNIGHT;
	enemy_component.range = KNIGHT_RANGE;
	enemy_component.cooldown = KNIGHT_COOLDOWN;

	Motion& motion = registry.motions.emplace(enemy);
	motion.position = position;
	motion.velocity = velocity;
	motion.scale = { 0.5f, 0.5f };

	Health& health = registry.healths.emplace(enemy);
	health.health = KNIGHT_HEALTH;
	health.maxHealth = KNIGHT_HEALTH;

	Deadly& deadly = registry.deadlies.emplace(enemy);
	deadly.to_projectile = true;

	Damage& damage = registry.damages.emplace(enemy);
	damage.value = KNIGHT_DAMAGE;

	RenderRequest& request = registry.render_requests.emplace(enemy);
	request.mesh = "sprite";
	request.texture = "knight";
	request.shader = "sprite";
	request.type = ENEMY;

	AI_SYSTEM::initAIComponent(&enemy);
}

void WorldSystem::loadBackgroundObjects() {
	// createBackgroundObject({ window_width_px / 4, window_height_px / 4 }, { 0.75, 0.75 }, "tree", false);

	Entity campfire = createBackgroundObject({ window_width_px / 2, window_height_px / 2 + 50.f }, { 0.5, 0.5 }, "campfire", true);
	Animation& campfireAnimation = registry.animations.emplace(campfire);
	campfireAnimation.frameTime = 100.f;
	campfireAnimation.spriteCols = 6;
	campfireAnimation.spriteRows = 1;
	campfireAnimation.frameCount = 6;
	Interactable& campfireInteractable = registry.interactables.emplace(campfire);
	campfireInteractable.type = InteractableType::HEALER;
}

Entity WorldSystem::createBackgroundObject(vec2 position, vec2 scale, AssetId texture, bool animate)
{
	Entity object;

	Motion& motion = registry.motions.emplace(object);
	motion.position = position;
	motion.velocity = { 0.f, 0.f };
	motion.scale = scale;

	RenderRequest& request = registry.render_requests.emplace(object);
	request.mesh = "sprite";
	request.texture = texture;
	if (animate) {
		request.shader = "animatedsprite";
	}
	else {
		request.shader = "sprite";
	}

	return object;
}

/**
 * @brief Handles the logic for spawning enemies and their movement direction
 * towards the player
 * @param elapsed_ms_since_last_update
 * @return void
 * If the enemy spawn timer has elapsed, a new enemy is spawned at a random
 * location Enemies are spawned outside the window and move towards the player
 */
void WorldSystem::handle_enemy_logic(const float elapsed_ms_since_last_update)
{
	this->farmer_spawn_timer -= elapsed_ms_since_last_update;
	this->archer_spawn_timer -= elapsed_ms_since_last_update;
	this->knight_spawn_timer -= elapsed_ms_since_last_update;

	const bool should_spawn_farmer = this->farmer_spawn_timer <= 0;
	const bool should_spawn_archer = this->archer_spawn_timer <= 0;
	const bool should_spawn_knight = this->knight_spawn_timer <= 0;

	if (should_spawn_farmer || should_spawn_archer || should_spawn_knight)
	{
		std::random_device rd;	// Random device
		std::mt19937 gen(rd()); // Mersenne Twister generator
		enum SIDE
		{
			TOP,
			RIGHT,
			BOTTOM,
			LEFT
		}; // Side of the window to spawn from
		std::uniform_int_distribution<int> side_dis(TOP, LEFT);
		const int side = side_dis(gen);

		float candidate_x = 0.f, candidate_y = 0.f;
		std::uniform_real_distribution<float> dis(0.f, 1.f);
		constexpr float offset_x = window_width_px / 10.f;
		constexpr float offset_y = window_height_px / 10.f;

		switch (side) {
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

		if (should_spawn_farmer) {
			this->farmer_spawn_timer = FARMER_SPAWN_INTERVAL_MS;
			this->createEnemy(EnemyType::FARMER, position, { 0, 0 });
		}

		if (should_spawn_archer) {
			this->archer_spawn_timer = ARCHER_SPAWN_INTERVAL_MS;
			this->createEnemy(EnemyType::ARCHER, position, { 0, 0 });
		}

		if (should_spawn_knight) {
			this->knight_spawn_timer = KNIGHT_SPAWN_INTERVAL_MS;
			this->createEnemy(EnemyType::KNIGHT, position, { 0, 0 });
		}
	}


}


void WorldSystem::setSpawnTimers(float farmer, float archer, float knight) {
	this->farmer_spawn_timer = farmer;
	this->archer_spawn_timer = archer;
	this->knight_spawn_timer = knight;
}


Entity WorldSystem::getPlayer()  const {
	return player_mage;
}
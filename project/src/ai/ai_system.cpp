#include "ai/ai_system.hpp"
#include "entities/ecs_registry.hpp"


AIComponent& AI_SYSTEM::initAIComponent(Entity* entity) {
    AIComponent aiComponent;
    
    // Create root selector (tries each behaviour in order)
    auto root = new ControlNode(ControlNode::ControlType::SELECTOR);
    
    // 1. Check if health is low -> don't move
    auto healthCheck = new ConditionNode(
        [entity = *entity](float elapsed_ms) {  // Capture entity value
            if (!registry.healths.has(entity)) {
                return false;
            }
            // Get health from entity's health component
            auto& health = registry.healths.get(entity);
            return health.health <= LOW_HEALTH_THRESHOLD;
        }
    );
    
    auto fleeFromPlayer = new ActionNode(
        [entity = *entity](float elapsed_ms) {
            // Could play hurt animation or other feedback
            auto& motion = registry.motions.get(entity);
            auto& enemy = registry.enemies.get(entity);

            Entity player_mage = registry.players.entities[0];
            if (!registry.motions.has(player_mage)) {
                return NodeState::FAILURE;
            }
            auto& player_motion = registry.motions.get(player_mage);
            float distance = glm::distance(motion.position, player_motion.position);


            if (distance < enemy.range) {
                return NodeState::SUCCESS;
            }

            if (distance > enemy.range * 2) {
                motion.velocity = {0, 0};
                return NodeState::SUCCESS;
            }

            EnemyType type = enemy.type;
            if (type != EnemyType::FARMER) {
                motion.velocity = {0, 0};
                return NodeState::SUCCESS;
            }

            vec2 direction_normalized = glm::normalize(motion.position - player_motion.position);
            motion.velocity = direction_normalized * FARMER_VELOCITY * 0.9f; // wounded farmer


            // Recover health
            auto& health = registry.healths.get(entity);
            health.health += HEALTH_RECOVERY_RATE * elapsed_ms / 1000.0f;

            return NodeState::RUNNING;
        }
    );
    
    auto lowHealthSequence = new ControlNode(ControlNode::ControlType::SEQUENCE);
    lowHealthSequence->children.push_back(healthCheck);
    lowHealthSequence->children.push_back(fleeFromPlayer);
    
    // 2. Check if in range -> attack
 auto inRangeCheck = new ConditionNode(
    [entity = *entity](float elapsed_ms) {
            auto& motion = registry.motions.get(entity);
            Entity player_mage = registry.players.entities[0];
            auto& player_motion = registry.motions.get(player_mage);
            auto& enemy = registry.enemies.get(entity);
            float range = enemy.range;
            float distance = glm::distance(motion.position, player_motion.position);
            return distance < range;
        }
    );
    
    auto attackAction = new ActionNode(
        [entity = *entity](float elapsed_ms) {
            Enemy& enemy = registry.enemies.get(entity);
            Entity player_mage = registry.players.entities[0];

            if (enemy.cooldown <= 0) {
                create_enemy_projectile(entity);
                invoke_enemy_cooldown(entity);
            }
            // This could trigger animation, spawn projectile, etc.
            return NodeState::SUCCESS;
        },
        0.0f,
        false
    );
    
    auto attackSequence = new ControlNode(ControlNode::ControlType::SEQUENCE);
    attackSequence->children.push_back(inRangeCheck);
    attackSequence->children.push_back(attackAction);
    
    // 3. Move towards player (fallback behavior)
auto moveToPlayer = new ActionNode(
    [entity = *entity](float elapsed_ms) {
        if (!registry.motions.has(entity)) {
            return NodeState::FAILURE;
        }
        auto& motion = registry.motions.get(entity);
        auto& enemy = registry.enemies.get(entity);

            Entity player_mage = registry.players.entities[0];
            if (!registry.motions.has(player_mage)) {
                return NodeState::FAILURE;
            }
            auto& player_motion = registry.motions.get(player_mage);
            float distance = glm::distance(motion.position, player_motion.position);

            // printf("distance: %f, range: %f\n", distance, enemy.range);

            if (distance < enemy.range) {
                return NodeState::SUCCESS;
            }

            // Move towards player
            float speed = 0.0f;
            EnemyType type = enemy.type;
            switch (type) {
                case EnemyType::FARMER:
                    speed = FARMER_VELOCITY;
                    break;
                case EnemyType::ARCHER:
                    speed = ARCHER_VELOCITY;
                    break;
                case EnemyType::KNIGHT:
                    speed = KNIGHT_VELOCITY;
                    break;
            }
            vec2 direction_normalized = glm::normalize(player_motion.position - motion.position);
            motion.velocity = direction_normalized * speed;
            return NodeState::RUNNING;  // Keep moving
        },
        3000.0f,
        false
    );

    
    // Build the complete tree
    root->children.push_back(lowHealthSequence);
    root->children.push_back(attackSequence);
    root->children.push_back(moveToPlayer);
    
    aiComponent.root = root;
    
    // Add to registry
    registry.ai_systems.emplace(*entity, aiComponent);
    return registry.ai_systems.get(*entity);
}

void AI_SYSTEM::tickForEntity(Entity* entity, float elapsed_ms) {
    if (!registry.enemies.has(*entity)) {
        return;
    }
    if (!registry.ai_systems.has(*entity)) {
        return;
    }
    auto& aiComponent = registry.ai_systems.get(*entity);
    if (aiComponent.root) {
        aiComponent.root->tick(elapsed_ms);
    }
}

void AI_SYSTEM::create_enemy_projectile(const Entity& enemy_ent)
{
	Entity projectile_ent;
	Projectile& projectile = registry.projectiles.emplace(projectile_ent);
	Motion& projectile_motion = registry.motions.emplace(projectile_ent);
	Deadly& deadly = registry.deadlies.emplace(projectile_ent);
	Damage& damage = registry.damages.emplace(projectile_ent);
	RenderRequest& request = registry.render_requests.emplace(projectile_ent);
	Motion& enemy_motion = registry.motions.get(enemy_ent);
	Enemy& enemy = registry.enemies.get(enemy_ent);

	deadly.to_player = true;

	projectile_motion.scale = { 0.525f, 0.525f };
	projectile_motion.position = enemy_motion.position;
	projectile_motion.angle = enemy_motion.angle;

	float attack_velocity;
	float attack_damage;
	std::string attack_texture;
	switch (enemy.type) {
	case EnemyType::FARMER:
		attack_velocity = PITCHFORK_VELOCITY;
		attack_damage = PITCHFORK_DAMAGE;
		attack_texture = "pitchfork";
		break;
	case EnemyType::ARCHER:
		attack_velocity = ARROW_VELOCITY;
		attack_damage = ARROW_DAMAGE;
		attack_texture = "arrow";
		break;
	case EnemyType::KNIGHT:
		attack_velocity = KNIGHT_VELOCITY;
		attack_damage = SWORD_DAMAGE;
		attack_texture = "filler";
		break;
	default: // Should not happen but just in case
		attack_velocity = PITCHFORK_VELOCITY;
		attack_damage = PITCHFORK_DAMAGE;
		attack_texture = "pitchfork";
		break;
	}

	projectile.type = DamageType::elementless;
	projectile.range = enemy.range;
	projectile_motion.velocity = vec2({ cos(enemy_motion.angle), sin(enemy_motion.angle) }) * attack_velocity;
	damage.value = attack_damage;

	request.mesh = "sprite";
	request.texture = attack_texture;
	request.shader = "sprite";
	request.type = PROJECTILE;
}

void AI_SYSTEM::invoke_enemy_cooldown(const Entity& enemy_ent)
{
	Enemy& enemy = registry.enemies.get(enemy_ent);
	EnemyType enemy_type = enemy.type;

	switch (enemy_type) {
	case EnemyType::FARMER:
		enemy.cooldown = FARMER_COOLDOWN;
		break;
	case EnemyType::ARCHER:
		enemy.cooldown = ARCHER_COOLDOWN;
		break;
	case EnemyType::KNIGHT:
		enemy.cooldown = KNIGHT_COOLDOWN;
		break;
	}
}

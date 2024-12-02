#include "ai/ai_system.hpp"
#include "entities/ecs_registry.hpp"
#include "utils/angle_functions.hpp"

#include "sound/sound_manager.hpp"

AIComponent& AI_SYSTEM::initAIComponent(Entity* entity) {
    AIComponent aiComponent;

    auto root = new ControlNode(ControlNode::ControlType::SELECTOR);

    auto healthCheck = new ConditionNode(
        [entity = *entity](float elapsed_ms) {
            if (!registry.healths.has(entity)) {
                return false;
            }
            auto& health = registry.healths.get(entity);
            return health.health / health.maxHealth <= LOW_HEALTH_THRESHOLD;
        }
    );

    auto fleeFromPlayer = new ActionNode(
        [entity = *entity](float elapsed_ms) {
            if (!registry.motions.has(entity)) {
                return NodeState::FAILURE;
            }
            if (registry.players.size() == 0) {
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


            if (distance < enemy.range) {
                return NodeState::SUCCESS;
            }

            if (distance > enemy.range * 2) {
                motion.velocity = { 0, 0 };
                return NodeState::SUCCESS;
            }

            EnemyType type = enemy.type;
            if (type != EnemyType::ARCHER) {
                motion.velocity = { 0, 0 };
                return NodeState::SUCCESS;
            }

            vec2 direction_normalized = glm::normalize(motion.position - player_motion.position);
            motion.velocity = direction_normalized * KNIGHT_VELOCITY * 0.9f; // wounded knight
            // Recover health
            auto& health = registry.healths.get(entity);
            health.health += HEALTH_RECOVERY_RATE * elapsed_ms / 1000.0f;

            return NodeState::SUCCESS;
        }
    );

    auto lowHealthSequence = new ControlNode(ControlNode::ControlType::SEQUENCE);
    lowHealthSequence->children.push_back(healthCheck);
    lowHealthSequence->children.push_back(fleeFromPlayer);

    auto inRangeCheck = new ConditionNode(
        [entity = *entity](float elapsed_ms) {
            if (!registry.motions.has(entity)) {
                return false;
            }
            if (registry.players.size() == 0) {
                return false;
            }
            auto& motion = registry.motions.get(entity);
            Entity player_mage = registry.players.entities[0];
            auto& player_motion = registry.motions.get(player_mage);
            auto& enemy = registry.enemies.get(entity);
            float range = enemy.range;
            float distance = glm::distance(motion.position, player_motion.position);

            if (enemy.type == EnemyType::DARKLORD && enemy.secondCooldown <= 0) {
                return true; 
            }

            return distance < range;
        }
    );

    auto attackAction = new ActionNode(
        [entity = *entity](float elapsed_ms) {
            if (!registry.enemies.has(entity)) {
                return NodeState::FAILURE;
            }
            if (registry.players.size() == 0) {
                return NodeState::FAILURE;
            }
            Enemy& enemy = registry.enemies.get(entity);

            /*Motion& motion = registry.motions.get(entity);
            motion.velocity = { 0, 0 };*/

            if (enemy.cooldown <= 0) {
                if (enemy.type == EnemyType::SLASHER) {
                    slash(entity);
                }
                else {
                    create_enemy_projectile(entity, true);
                }
                invoke_enemy_cooldown(entity, true);
            }

            if (enemy.type == EnemyType::DARKLORD && enemy.secondCooldown <= 0) {
                Entity player = registry.players.entities[0];
                Motion &motionPlayer = registry.motions.get(player);
                Motion &motionDarkLord = registry.motions.get(entity);
                float distance = glm::distance(motionPlayer.position, motionDarkLord.position);

                SoundManager* soundManager = SoundManager::getSoundManager();
                soundManager->playSound(SoundEffect::COMEHERE);
                create_enemy_projectile(entity, false);
                invoke_enemy_cooldown(entity, false);
            }
            return NodeState::SUCCESS;
        },
        0.0f,
        false
    );

    auto attackSequence = new ControlNode(ControlNode::ControlType::SEQUENCE);
    attackSequence->children.push_back(inRangeCheck);
    attackSequence->children.push_back(attackAction);

    auto moveToPlayer = new ActionNode(
        [entity = *entity](float elapsed_ms) {
            if (!registry.motions.has(entity)) {
                return NodeState::FAILURE;
            }
            if (registry.players.size() == 0) {
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

            if (enemy.movementRestricted) {
                return NodeState::FAILURE;
            }

            float speed = 0.0f;
            EnemyType type = enemy.type;
            switch (type) {
            case EnemyType::KNIGHT:
                speed = KNIGHT_VELOCITY;
                break;
            case EnemyType::ARCHER:
                speed = ARCHER_VELOCITY;
                break;
            case EnemyType::PALADIN:
                speed = PALADIN_VELOCITY;
                break;
            case EnemyType::SLASHER:
                speed = SLASHER_VELOCITY;
                break;
            case EnemyType::DARKLORD:
                speed = DARKLORD_VELOCITY;
                break;
            }
            vec2 direction_normalized = glm::normalize(player_motion.position - motion.position);
            motion.velocity = direction_normalized * speed;
            return NodeState::SUCCESS;
        },
        0.0f,
        true
    );


    root->children.push_back(lowHealthSequence);
    root->children.push_back(attackSequence);
    root->children.push_back(moveToPlayer);

    aiComponent.root = root;

    registry.ai_systems.emplace(*entity, aiComponent);
    return registry.ai_systems.get(*entity);
}

// TODO: linear interpolation
void AI_SYSTEM::slash(const Entity& enemy_ent) {
    Motion& enemy_motion = registry.motions.get(enemy_ent);
    Enemy& enemy = registry.enemies.get(enemy_ent);

    Animation& enemy_animation = registry.animations.get(enemy_ent);
    enemy_animation.state = EntityState::ATTACKING;
    enemy_animation.frameTime = 30.f;
    enemy_motion.currentDirection = angleToDirection(find_closest_angle(enemy_motion.angle));
    enemy_animation.initializeAtRow((int)enemy_motion.currentDirection);

    enemy_motion.velocity *= 2.f;
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

void AI_SYSTEM::create_enemy_projectile(const Entity& enemy_ent, bool mainSpell)
{
    Entity projectile_ent;
    Projectile& projectile = registry.projectiles.emplace(projectile_ent);
    Motion& projectile_motion = registry.motions.emplace(projectile_ent);
    Deadly& deadly = registry.deadlies.emplace(projectile_ent);
    Damage& damage = registry.damages.emplace(projectile_ent);
    RenderRequest& request = registry.render_requests.emplace(projectile_ent);
    Motion& enemy_motion = registry.motions.get(enemy_ent);
    Enemy& enemy = registry.enemies.get(enemy_ent);

    Animation& enemy_animation = registry.animations.get(enemy_ent);
    enemy_animation.state = EntityState::ATTACKING;
    enemy_animation.frameTime = 30.f;
    enemy_motion.currentDirection = angleToDirection(find_closest_angle(enemy_motion.angle));
    enemy_animation.initializeAtRow((int)enemy_motion.currentDirection);

    deadly.to_player = true;

    projectile_motion.scale = { 0.525f, 0.525f };
    projectile_motion.position = enemy_motion.position;
    projectile_motion.angle = enemy_motion.angle;

    projectile.type = DamageType::elementless;
    projectile.range = enemy.range;
    projectile.isActive = true;

    float attack_velocity;
    float attack_damage;
    std::string attack_texture;
    switch (enemy.type) {
    case EnemyType::KNIGHT:
        attack_velocity = PITCHFORK_VELOCITY;
        attack_damage = PITCHFORK_DAMAGE;
        attack_texture = "pitchfork";
        break;
    case EnemyType::ARCHER:
        attack_velocity = ARROW_VELOCITY;
        attack_damage = ARROW_DAMAGE;
        attack_texture = "arrow";
        break;
    case EnemyType::PALADIN:
        attack_velocity = PALADIN_VELOCITY;
        attack_damage = SWORD_DAMAGE;
        attack_texture = "filler";
        break;
    case EnemyType::DARKLORD:
        if (mainSpell) {
            attack_velocity = DARKLORD_RAZOR_SPEED;
            attack_damage = DARKLORD_RAZOR_DAMAGE;
            attack_texture = "plasma";
            projectile_motion.scale = { 0.75f, 0.75f };
            projectile_motion.collider = { 37.5f, 37.5f };
            projectile.type = DamageType::plasma;
        } else {
            Entity player = registry.players.entities[0];
            Motion &motionPlayer = registry.motions.get(player);

            attack_velocity = 0.0f;
            attack_damage = 0;
            attack_texture = "portal";
            projectile_motion.collider = { 17.5f, 20.f };
            projectile.type = DamageType::portal;
            projectile.range = DARKLORD_PORTAL_COOLDOWN / 4.0f;
            projectile.sourcePosition = enemy_motion.position;

            projectile_motion.angle = 0.0f;
            projectile_motion.scale = { 0.5f, 0.5f };

            float distanceAway = 1000;

            vec2 initial_position = vec2(
                motionPlayer.position.x + distanceAway * motionPlayer.velocity.x,
                motionPlayer.position.y + distanceAway * motionPlayer.velocity.y
            );

            initial_position.x = std::max(0.0f + projectile_motion.collider.x, std::min(initial_position.x, static_cast<float>(window_width_px) - projectile_motion.collider.x));
            initial_position.y = std::max(0.0f + projectile_motion.collider.y, std::min(initial_position.y, static_cast<float>(window_height_px) - projectile_motion.collider.y));

            projectile_motion.position = initial_position;
        }
        break;
    default:
        attack_velocity = PITCHFORK_VELOCITY;
        attack_damage = PITCHFORK_DAMAGE;
        attack_texture = "pitchfork";
        break;
    }


    projectile_motion.velocity = vec2({ cos(enemy_motion.angle), sin(enemy_motion.angle) }) * attack_velocity;
    damage.value = attack_damage;

    request.mesh = "sprite";
    request.texture = attack_texture;
    request.shader = "sprite";
    request.type = PROJECTILE;
}

void AI_SYSTEM::invoke_enemy_cooldown(const Entity& enemy_ent, bool first)
{
    Enemy& enemy = registry.enemies.get(enemy_ent);
    EnemyType enemy_type = enemy.type;

    switch (enemy_type) {
    case EnemyType::KNIGHT:
        enemy.cooldown = KNIGHT_COOLDOWN;
        break;
    case EnemyType::ARCHER:
        enemy.cooldown = ARCHER_COOLDOWN;
        break;
    case EnemyType::PALADIN:
        enemy.cooldown = PALADIN_COOLDOWN;
        break;
    case EnemyType::SLASHER:
        enemy.cooldown = SLASHER_COOLDOWN;
        break;
    case EnemyType::DARKLORD:
        if (first) {
            enemy.cooldown = DARKLORD_RAZOR_COOLDOWN;
        } else {
            enemy.secondCooldown = DARKLORD_PORTAL_COOLDOWN;
        }
        break;
    }
}


void AI_SYSTEM::cleanNodeTree(Node* node)
{
    if (!node) {
        return;
    }

    // Recursively clean up all child nodes
    for (Node* child : node->children) {
        cleanNodeTree(child);
    }

    delete node;
}

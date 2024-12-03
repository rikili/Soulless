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

            // Slasher has no low-health behaviour, Darklord is coded in attack sequence
            if (registry.enemies.get(entity).normalBehaviour ||
                registry.enemies.get(entity).type == EnemyType::SLASHER ||
                registry.enemies.get(entity).type == EnemyType::DARKLORD) {

                return false;
            }

            return health.health / health.maxHealth <= LOW_HEALTH_THRESHOLD;
        }
    );

    auto lowHealthBehaviour = new ActionNode(
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

            vec2 awayFromPlayer = glm::normalize(motion.position - player_motion.position);
            vec2 towardsPlayer = glm::normalize(player_motion.position - motion.position);

            EnemyType type = enemy.type;

            // Knight low-health behaviour:
            //  - Retreats, periodically blocking until reaching a distance threshold.
            //  - It will then move towards the player, continuing to block to protect itself and other enemies.
            if (type == EnemyType::KNIGHT) {

                if (distance > KNIGHT_RETREAT_DISTANCE) {
                    enemy.altAttackPattern = true;
                }

                if (enemy.altAttackPattern) {
                    motion.velocity = towardsPlayer * KNIGHT_VELOCITY;
                }
                else {
                    motion.velocity = awayFromPlayer * KNIGHT_VELOCITY * 0.8f; // wounded knight
                }

                if (enemy.eTimer <= 0) {
                    enemy.eTimer = KNIGHT_BLOCK_COOLDOWN;
                }

                enemy.eTimer -= elapsed_ms;
                Animation& knightAnimation = registry.animations.get(entity);

                if (enemy.eTimer < KNIGHT_BLOCK_COOLDOWN / 2.0) {
                    enemy.blocking = true;

                    motion.velocity = { 0, 0 };
                    motion.angle = atan2(player_motion.position.y - motion.position.y,
                                         player_motion.position.x - motion.position.x);
                    motion.oldDirection = motion.currentDirection;
                    motion.currentDirection = angleToDirection(find_closest_angle(motion.angle));
                    knightAnimation.state = AnimationState::BLOCKING;
                }
                else {
                    enemy.blocking = false;
                    knightAnimation.state = AnimationState::WALKING;
                }

                return NodeState::SUCCESS;
            }

            // Archer low-health behaviour:
            //  - Retreats, healing itself until reaching the low health threshold.
            //  - It will then return to attack the player as normal.
            if (type == EnemyType::ARCHER) {

                Animation& archerAnimation = registry.animations.get(entity);
                motion.velocity = awayFromPlayer * ARCHER_VELOCITY * 2.25f;
                archerAnimation.state = AnimationState::RUNNING;

                if (distance > ARCHER_RETREAT_DISTANCE) {
                    motion.velocity = towardsPlayer * ARCHER_VELOCITY;
                    archerAnimation.state = AnimationState::WALKING;
                    enemy.normalBehaviour = true;
                }
                return NodeState::SUCCESS;
            }


            // Paladin low-health behaviour:
            // - Does a stationary battle cry then charges at the player, deals damage on contact.
            // - Once it makes contact with the edge of the map it will return to attack the player as normal.
            if (type == EnemyType::PALADIN) {

                Animation& paladinAnimation = registry.animations.get(entity);
                Deadly& paladinDeadly = registry.deadlies.get(entity);

                if (!enemy.altAttackPattern) {

                    motion.velocity = { 0, 0 };
                    motion.angle = atan2(player_motion.position.y - motion.position.y,
                        player_motion.position.x - motion.position.x);
                    motion.oldDirection = motion.currentDirection;
                    motion.currentDirection = angleToDirection(find_closest_angle(motion.angle));

                    paladinAnimation.state = AnimationState::BATTLECRY;
                    paladinAnimation.frameTime = 100.f;

                    enemy.eTimer = 5000.f;

                    paladinDeadly.to_player = true;
                    enemy.altAttackPattern = true;
                }

                if (enemy.eTimer >= 0.f) {
                    if (enemy.eTimer <= 5000.f - paladinAnimation.frameTime * (paladinAnimation.frameCount - 1)) {
                        paladinAnimation.state = AnimationState::RUNNING;
                        paladinAnimation.frameTime = 25.f;
                        motion.velocity = towardsPlayer * PALADIN_VELOCITY * 10.0f;
                        enemy.eTimer = -1.f;
                    }

                    enemy.eTimer -= elapsed_ms;
                }

                float x_offset = motion.collider.x * motion.scale.x;
                float y_offset = motion.collider.y * motion.scale.y;

                if (motion.position.x <= x_offset
                    || motion.position.x >= window_width_px - x_offset
                    || motion.position.y <= y_offset
                    || motion.position.y >= window_height_px - y_offset) {
                    motion.velocity = towardsPlayer * PALADIN_VELOCITY;
                    paladinAnimation.state = AnimationState::WALKING;
                    paladinAnimation.frameTime = DEFAULT_LOOP_TIME;
                    paladinDeadly.to_player = false;
                    enemy.normalBehaviour = true;
                }

            }

            else {
                vec2 direction_normalized = glm::normalize(motion.position - player_motion.position);
                motion.velocity = direction_normalized * KNIGHT_VELOCITY * 0.9f; // wounded knight
            }

            return NodeState::SUCCESS;
        }
    );

    auto lowHealthSequence = new ControlNode(ControlNode::ControlType::SEQUENCE);
    lowHealthSequence->children.push_back(healthCheck);
    lowHealthSequence->children.push_back(lowHealthBehaviour);

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

            // Idle if player dead
            Entity player_mage = registry.players.entities[0];
            if (registry.deaths.has(player_mage)) {
                registry.motions.get(entity).velocity = { 0, 0 };
                return NodeState::SUCCESS;
            }

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
            auto& health = registry.healths.get(entity);

            Entity player_mage = registry.players.entities[0];
            if (!registry.motions.has(player_mage)) {
                return NodeState::FAILURE;
            }

            // Idle if player dead
            if (registry.deaths.has(player_mage)) {
                motion.velocity = { 0, 0 };
                return NodeState::SUCCESS;
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
                if (health.health / health.maxHealth <= BOSS_LOW_HEALTH_THRESHOLD) {
                    speed = DARKLORD_VELOCITY * 1.5;
                }
                else {
                    speed = DARKLORD_VELOCITY;
                }
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
    enemy_animation.state = AnimationState::ATTACKING;
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
    enemy_animation.state = AnimationState::ATTACKING;
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
            projectile_motion.collider = { 25.f, 32.5f };
            projectile.type = DamageType::portal;
            projectile.range = DARKLORD_PORTAL_COOLDOWN / 4.0f;
            projectile.sourcePosition = enemy_motion.position;

            projectile_motion.angle = 0.0f;
            projectile_motion.scale = { 0.75f, 0.75f };

            float distanceAway = 800;

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
    auto& health = registry.healths.get(enemy_ent);

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
            if (health.health / health.maxHealth <= BOSS_LOW_HEALTH_THRESHOLD) {
                enemy.cooldown = DARKLORD_RAZOR_COOLDOWN / 4.0f;
            } else {
                enemy.cooldown = DARKLORD_RAZOR_COOLDOWN;
            }
        }
        else {
            if (health.health / health.maxHealth <= BOSS_LOW_HEALTH_THRESHOLD) {
                enemy.secondCooldown = DARKLORD_PORTAL_COOLDOWN / 2.0f;
            } else {
                enemy.secondCooldown = DARKLORD_PORTAL_COOLDOWN;
            }
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

#include "utils/enemy_factory.hpp"

namespace EnemyFactory {

Entity createEnemy(
    ECSRegistry& registry, 
    EnemyType type, 
    vec2 position, 
    vec2 velocity, 
    float range, 
    float cooldown, 
    float health, 
    float maxHealth,
    float damage, 
    const std::string& texture, 
    bool deadlyToPlayer
) {
    Entity enemy;

    Enemy& enemy_component = registry.enemies.emplace(enemy);
    enemy_component.type = type;
    enemy_component.range = range;
    enemy_component.cooldown = cooldown;

    Motion& motion = registry.motions.emplace(enemy);
    motion.position = position;
    motion.velocity = velocity;
    motion.scale = { 0.5f, 0.5f }; // Default scale

    Health& health_component = registry.healths.emplace(enemy);
    health_component.health = health;
    health_component.maxHealth = maxHealth;

    Deadly& deadly = registry.deadlies.emplace(enemy);
    deadly.to_projectile = true;
    deadly.to_player = deadlyToPlayer;

    Damage& damage_component = registry.damages.emplace(enemy);
    damage_component.value = damage;

    RenderRequest& request = registry.render_requests.emplace(enemy);
    request.mesh = "sprite";
    request.texture = texture;
    request.shader = "sprite";
    request.type = ENEMY;

    AI_SYSTEM::initAIComponent(&enemy);

    return enemy;
}

Entity createKnight(ECSRegistry& registry, vec2 position, vec2 velocity) {
    return createEnemy(
        registry, 
        EnemyType::KNIGHT, 
        position, 
        velocity, 
        KNIGHT_RANGE, 
        KNIGHT_COOLDOWN, 
        KNIGHT_HEALTH, 
        KNIGHT_HEALTH, 
        KNIGHT_DAMAGE, 
        "knight",
        false
    );
}

Entity createFarmer(ECSRegistry& registry, vec2 position, vec2 velocity) {
    return createEnemy(
        registry, 
        EnemyType::FARMER, 
        position, 
        velocity, 
        FARMER_RANGE, 
        FARMER_COOLDOWN, 
        FARMER_HEALTH, 
        FARMER_HEALTH, 
        FARMER_DAMAGE, 
        "farmer", 
        true
    );
}

Entity createArcher(ECSRegistry& registry, vec2 position, vec2 velocity) {
    return createEnemy(
        registry, 
        EnemyType::ARCHER, 
        position, 
        velocity, 
        ARCHER_RANGE, 
        ARCHER_COOLDOWN, 
        ARCHER_HEALTH, 
        ARCHER_HEALTH, 
        ARCHER_DAMAGE, 
        "archer",
        false
    );
}

// used for reloadability
Entity createKnight(ECSRegistry& registry, vec2 position, vec2 velocity, float cooldown, float health) {
    return createEnemy(
        registry, 
        EnemyType::KNIGHT, 
        position, 
        velocity, 
        KNIGHT_RANGE, 
        cooldown,
        health,
        KNIGHT_HEALTH,
        KNIGHT_DAMAGE, 
        "knight",
        false
    );
}

Entity createFarmer(ECSRegistry& registry, vec2 position, vec2 velocity, float cooldown, float health) {
    return createEnemy(
        registry, 
        EnemyType::FARMER, 
        position, 
        velocity, 
        FARMER_RANGE, 
        cooldown,
        health,
        FARMER_HEALTH,
        FARMER_DAMAGE, 
        "farmer", 
        true
    );
}

Entity createArcher(ECSRegistry& registry, vec2 position, vec2 velocity, float cooldown, float health) {
    return createEnemy(
        registry, 
        EnemyType::ARCHER, 
        position, 
        velocity, 
        ARCHER_RANGE, 
        cooldown,
        health,
        ARCHER_HEALTH,
        ARCHER_DAMAGE, 
        "archer",
        false
    );
}

}

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
        bool deadlyToPlayer,
        vec2 scale
    ) {
        Entity enemy;

        Enemy& enemy_component = registry.enemies.emplace(enemy);
        enemy_component.type = type;
        enemy_component.range = range;
        enemy_component.cooldown = cooldown;

        Motion& motion = registry.motions.emplace(enemy);
        motion.position = position;
        motion.velocity = velocity;
        motion.scale = scale;

        Health& health_component = registry.healths.emplace(enemy);
        health_component.health = health;
        health_component.maxHealth = maxHealth;

        auto healthBar = Entity();
        HealthBar& healthBarComp = registry.healthBars.emplace(healthBar);
        healthBarComp.assignHealthBar(enemy);
        healthBarComp.position = { motion.position.x, motion.position.y - HEALTH_BAR_Y_OFFSET };

        Deadly& deadly = registry.deadlies.emplace(enemy);
        deadly.to_projectile = true;
        deadly.to_player = deadlyToPlayer;

        Damage& damage_component = registry.damages.emplace(enemy);
        damage_component.value = damage;

        Animation& animation = registry.animations.emplace(enemy);
        animation.spriteCols = 15;
        animation.spriteRows = 8;
        animation.spriteCount = 120;
        animation.frameCount = 15;
        animation.initializeAtFrame(0.0f);

        RenderRequest& request = registry.render_requests.emplace(enemy);
        request.mesh = "sprite";
        request.texture = texture;
        request.shader = "animatedsprite";
        request.type = ENEMY;

        AI_SYSTEM::initAIComponent(&enemy);

        return enemy;
    }

    Entity createPaladin(ECSRegistry& registry, vec2 position, vec2 velocity) {
        return createEnemy(
            registry,
            EnemyType::PALADIN,
            position,
            velocity,
            PALADIN_RANGE,
            PALADIN_COOLDOWN,
            PALADIN_HEALTH,
            PALADIN_HEALTH,
            PALADIN_DAMAGE,
            "paladin-idle",
            false
        );
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
            "knight-idle",
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
            "archer-idle",
            false
        );
    }

    Entity createSlasher(ECSRegistry& registry, vec2 position, vec2 velocity) {
        return createEnemy(
            registry,
            EnemyType::SLASHER,
            position,
            velocity,
            SLASHER_RANGE,
            SLASHER_COOLDOWN,
            SLASHER_HEALTH,
            SLASHER_HEALTH,
            SLASHER_DAMAGE,
            "slasher-idle",
            true
        );
    }

    Entity createDarkLord(ECSRegistry& registry, vec2 position, vec2 velocity) {
        return createEnemy(
            registry,
            EnemyType::DARKLORD,
            position,
            velocity,
            DARKLORD_RANGE,
            DARKLORD_COOLDOWN,
            DARKLORD_HEALTH,
            DARKLORD_HEALTH,
            DARKLORD_DAMAGE,
            "darklord-idle",
            true,
            { 2.f, 2.f }
        );
    }

    // used for reloadability
    Entity createPaladin(ECSRegistry& registry, vec2 position, vec2 velocity, float cooldown, float health) {
        return createEnemy(
            registry,
            EnemyType::PALADIN,
            position,
            velocity,
            PALADIN_RANGE,
            cooldown,
            health,
            PALADIN_HEALTH,
            PALADIN_DAMAGE,
            "paladin-idle",
            false
        );
    }

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
            "knight-idle",
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
            "archer-idle",
            false
        );
    }

    Entity createSlasher(ECSRegistry& registry, vec2 position, vec2 velocity, float cooldown, float health) {
        return createEnemy(
            registry,
            EnemyType::SLASHER,
            position,
            velocity,
            SLASHER_RANGE,
            cooldown,
            health,
            SLASHER_HEALTH,
            SLASHER_DAMAGE,
            "slasher-idle",
            false
        );
    }

    Entity createDarkLord(ECSRegistry& registry, vec2 position, vec2 velocity, float cooldown, float health) {
        return createEnemy(
            registry,
            EnemyType::DARKLORD,
            position,
            velocity,
            DARKLORD_RANGE,
            cooldown,
            health,
            DARKLORD_HEALTH,
            DARKLORD_DAMAGE,
            "darklord-idle",
            true,
            { 2.f, 2.f }
        );

    }

}

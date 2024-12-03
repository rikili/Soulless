#include "utils/enemy_factory.hpp"

namespace EnemyFactory {

    Entity createEnemy(
        ECSRegistry& registry,
        EnemyType type,
        vec2 position,
        vec2 velocity,
        float range,
        float cooldown,
        float secondCooldown,
        float health,
        float maxHealth,
        float damage,
        const std::string& texture,
        bool deadlyToPlayer,
        float healthScale,
        vec2 scale
    ) {
        Entity enemy;

        Enemy& enemy_component = registry.enemies.emplace(enemy);
        enemy_component.type = type;
        enemy_component.range = range;
        enemy_component.cooldown = cooldown;
        enemy_component.secondCooldown = secondCooldown;

        Motion& motion = registry.motions.emplace(enemy);
        motion.position = position;
        motion.velocity = velocity;
        motion.scale = scale;

        Health& health_component = registry.healths.emplace(enemy);
        health_component.health = health * healthScale;
        health_component.maxHealth = maxHealth * healthScale;

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

    Entity createPaladin(ECSRegistry& registry, vec2 position, vec2 velocity, float healthScale) {
        return createEnemy(
            registry,
            EnemyType::PALADIN,
            position,
            velocity,
            PALADIN_RANGE,
            PALADIN_COOLDOWN,
            -1,
            PALADIN_HEALTH,
            PALADIN_HEALTH,
            PALADIN_DAMAGE,
            "paladin-idle",
            false,
            healthScale
        );
    }

    Entity createKnight(ECSRegistry& registry, vec2 position, vec2 velocity, float healthScale) {
        return createEnemy(
            registry,
            EnemyType::KNIGHT,
            position,
            velocity,
            KNIGHT_RANGE,
            KNIGHT_COOLDOWN,
            -1,
            KNIGHT_HEALTH,
            KNIGHT_HEALTH,
            KNIGHT_DAMAGE,
            "knight-idle",
            true,
            healthScale
        );
    }

    Entity createArcher(ECSRegistry& registry, vec2 position, vec2 velocity, float healthScale) {
        return createEnemy(
            registry,
            EnemyType::ARCHER,
            position,
            velocity,
            ARCHER_RANGE,
            ARCHER_COOLDOWN,
            -1,
            ARCHER_HEALTH,
            ARCHER_HEALTH,
            ARCHER_DAMAGE,
            "archer-idle",
            false,
            healthScale
        );
    }

    Entity createSlasher(ECSRegistry& registry, vec2 position, vec2 velocity, float healthScale) {
        return createEnemy(
            registry,
            EnemyType::SLASHER,
            position,
            velocity,
            SLASHER_RANGE,
            SLASHER_COOLDOWN,
            -1,
            SLASHER_HEALTH,
            SLASHER_HEALTH,
            SLASHER_DAMAGE,
            "slasher-idle",
            true,
            healthScale
        );
    }

    Entity createDarkLord(ECSRegistry& registry, vec2 position, vec2 velocity, float healthScale) {
        return createEnemy(
            registry,
            EnemyType::DARKLORD,
            position,
            velocity,
            DARKLORD_RANGE,
            DARKLORD_RAZOR_COOLDOWN,
            DARKLORD_PORTAL_COOLDOWN,
            DARKLORD_HEALTH,
            DARKLORD_HEALTH,
            DARKLORD_DAMAGE,
            "darklord-idle",
            true,
            healthScale,
            { 2.f, 2.f }
        );
    }

    // used for reloadability
    Entity createPaladin(ECSRegistry& registry, vec2 position, vec2 velocity, float cooldown, float health, float healthScale) {
        return createEnemy(
            registry,
            EnemyType::PALADIN,
            position,
            velocity,
            PALADIN_RANGE,
            cooldown,
            -1,
            health,
            PALADIN_HEALTH,
            PALADIN_DAMAGE,
            "paladin-idle",
            false,
            healthScale
        );
    }

    Entity createKnight(ECSRegistry& registry, vec2 position, vec2 velocity, float cooldown, float health, float healthScale) {
        return createEnemy(
            registry,
            EnemyType::KNIGHT,
            position,
            velocity,
            KNIGHT_RANGE,
            cooldown,
            -1,
            health,
            KNIGHT_HEALTH,
            KNIGHT_DAMAGE,
            "knight-idle",
            true,
            healthScale
        );
    }

    Entity createArcher(ECSRegistry& registry, vec2 position, vec2 velocity, float cooldown, float health, float healthScale) {
        return createEnemy(
            registry,
            EnemyType::ARCHER,
            position,
            velocity,
            ARCHER_RANGE,
            cooldown,
            -1,
            health,
            ARCHER_HEALTH,
            ARCHER_DAMAGE,
            "archer-idle",
            false,
            healthScale
        );
    }

    Entity createSlasher(ECSRegistry& registry, vec2 position, vec2 velocity, float cooldown, float health, float healthScale) {
        return createEnemy(
            registry,
            EnemyType::SLASHER,
            position,
            velocity,
            SLASHER_RANGE,
            cooldown,
            -1,
            health,
            SLASHER_HEALTH,
            SLASHER_DAMAGE,
            "slasher-idle",
            false,
            healthScale
        );
    }

    Entity createDarkLord(ECSRegistry& registry, vec2 position, vec2 velocity, float cooldown, float health, float healthScale) {
        return createEnemy(
            registry,
            EnemyType::DARKLORD,
            position,
            velocity,
            DARKLORD_RANGE,
            cooldown,
            cooldown,
            health,
            DARKLORD_HEALTH,
            DARKLORD_DAMAGE,
            "darklord-idle",
            true,
            healthScale,
            { 2.f, 2.f }
        );

    }

}

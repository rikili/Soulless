#include "entities/ecs_registry.hpp"

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
        bool deadlyToPlayer = false,
        float healthScale = 1.f,
        vec2 scale = { 1.f, 1.f }
    );

    Entity createPaladin(ECSRegistry& registry, vec2 position, vec2 velocity, float healthScale);
    Entity createPaladin(ECSRegistry& registry, vec2 position, vec2 velocity, float cooldown, float health, float healthScale);

    Entity createKnight(ECSRegistry& registry, vec2 position, vec2 velocity, float healthScale);
    Entity createKnight(ECSRegistry& registry, vec2 position, vec2 velocity, float cooldown, float health, float healthScale);

    Entity createArcher(ECSRegistry& registry, vec2 position, vec2 velocity, float healthScale);
    Entity createArcher(ECSRegistry& registry, vec2 position, vec2 velocity, float cooldown, float health, float healthScale);

    Entity createSlasher(ECSRegistry& registry, vec2 position, vec2 velocity, float healthScale);
    Entity createSlasher(ECSRegistry& registry, vec2 position, vec2 velocity, float cooldown, float health, float healthScale);

    Entity createDarkLord(ECSRegistry& registry, vec2 position, vec2 velocity, float healthScale);
    Entity createDarkLord(ECSRegistry& registry, vec2 position, vec2 velocity, float cooldown, float health, float healthScale);
}

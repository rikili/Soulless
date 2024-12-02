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
        vec2 scale = { 1.f, 1.f }
    );

    Entity createPaladin(ECSRegistry& registry, vec2 position, vec2 velocity);
    Entity createPaladin(ECSRegistry& registry, vec2 position, vec2 velocity, float cooldown, float health);

    Entity createKnight(ECSRegistry& registry, vec2 position, vec2 velocity);
    Entity createKnight(ECSRegistry& registry, vec2 position, vec2 velocity, float cooldown, float health);

    Entity createArcher(ECSRegistry& registry, vec2 position, vec2 velocity);
    Entity createArcher(ECSRegistry& registry, vec2 position, vec2 velocity, float cooldown, float health);

    Entity createSlasher(ECSRegistry& registry, vec2 position, vec2 velocity);
    Entity createSlasher(ECSRegistry& registry, vec2 position, vec2 velocity, float cooldown, float health);

    Entity createDarkLord(ECSRegistry& registry, vec2 position, vec2 velocity);
    Entity createDarkLord(ECSRegistry& registry, vec2 position, vec2 velocity, float cooldown, float health);
}

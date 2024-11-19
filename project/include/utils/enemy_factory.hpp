#include "entities/ecs_registry.hpp"

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
        bool deadlyToPlayer = false
    );

    Entity createPaladin(ECSRegistry& registry, vec2 position, vec2 velocity);
    Entity createPaladin(ECSRegistry& registry, vec2 position, vec2 velocity, float cooldown, float health);

    Entity createKnight(ECSRegistry& registry, vec2 position, vec2 velocity);
    Entity createKnight(ECSRegistry& registry, vec2 position, vec2 velocity, float cooldown, float health);

    Entity createArcher(ECSRegistry& registry, vec2 position, vec2 velocity);
    Entity createArcher(ECSRegistry& registry, vec2 position, vec2 velocity, float cooldown, float health);
}

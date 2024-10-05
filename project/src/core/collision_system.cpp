#include "core/collision_system.hpp"
#include "entities/ecs_registry.hpp"

CollisionSystem::CollisionSystem(RenderSystem* renderer)
{
    this->renderer = renderer;
}

void CollisionSystem::init()
{
}


void CollisionSystem::handle_collisions()
{
    std::vector<Entity> to_destroy;

    for (Entity entity : registry.motions.entities)
    {
        // Check if the entity is colliding with any other entity
        for (Entity other_entity : registry.motions.entities)
        {
            if (entity == other_entity)
            {
                continue;
            }

            Motion& motion = registry.motions.get(entity);
            Motion& other_motion = registry.motions.get(other_entity);
            Deadly& deadly = registry.deadlies.get(entity);

            // Neutral entity such as a tree or a rock
            if (!deadly.to_enemy && !deadly.to_player && !deadly.to_projectile) {
                continue;
            }

            // Check if the two entities are colliding
            if (motion.position.x < other_motion.position.x + other_motion.scale.x &&
                motion.position.x + motion.scale.x > other_motion.position.x &&
                motion.position.y < other_motion.position.y + other_motion.scale.y &&
                motion.position.y + motion.scale.y > other_motion.position.y)
            {
                if (deadly.to_enemy && registry.enemies.has(other_entity)){
                    // this->applyDamage(entity, other_entity);
                } else if (deadly.to_player && registry.players.has(other_entity)) {
                    this->applyDamage(entity, other_entity);
                    to_destroy.push_back(entity);
                } else
                {
                }
            }
        }
    }

    for (const Entity entity : to_destroy)
    {
        printf("Entity %u has been destroyed\n", static_cast<unsigned>(entity));
        registry.remove_all_components_of(entity);
        this->renderer->removeRenderRequest(entity);
    }
}

void CollisionSystem::applyDamage(Entity attacker, Entity victim)
{
    const Damage& damage = registry.damages.get(attacker);
    Health& health = registry.healths.get(victim);
    if (health.health - damage.value <= 0) {
        health.health = 0;
    } else {
        health.health -= damage.value;
    }

    Health& health2 = registry.healths.get(attacker);
    health2.health = 0;

}
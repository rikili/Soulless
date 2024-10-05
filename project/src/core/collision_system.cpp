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

            // Check if the two entities are colliding
            if (motion.position.x < other_motion.position.x + other_motion.scale.x &&
                motion.position.x + motion.scale.x > other_motion.position.x &&
                motion.position.y < other_motion.position.y + other_motion.scale.y &&
                motion.position.y + motion.scale.y > other_motion.position.y)
            {
                Deadly& deadly = registry.deadlies.get(entity);
                if (deadly.to_enemy && registry.enemies.has(other_entity)){
                    // printf("Entity %d is deadly to entity %d\n", static_cast<int>(entity), static_cast<int>(other_entity));
                }
                else if (deadly.to_player && registry.players.has(other_entity))
                {
                    // printf("Entity %d is deadly to entity %d\n", static_cast<int>(entity), static_cast<int>(other_entity));
                }
                else
                {
                    // printf("Collision detected between entity %d and entity %d\n", static_cast<int>(entity), static_cast<int>(other_entity));
                }

                // printf("Collision detected between entity %d and entity %d\n", static_cast<int>(entity), static_cast<int>(other_entity));
            }
        }
    }
}
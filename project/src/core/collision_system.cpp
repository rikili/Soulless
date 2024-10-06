#include "core/collision_system.hpp"
#include "entities/ecs_registry.hpp"

CollisionSystem::CollisionSystem(RenderSystem* renderer)
{
    this->renderer = renderer;
}

void CollisionSystem::init()
{
}

bool check_collision(Entity& entity, Entity& other_entity)
{
    Motion& motion = registry.motions.get(entity);
    Motion& other_motion = registry.motions.get(other_entity);

    return (motion.position.x < other_motion.position.x + other_motion.collider.x &&
        motion.position.x + motion.collider.x > other_motion.position.x &&
        motion.position.y < other_motion.position.y + other_motion.collider.y &&
        motion.position.y + motion.collider.y > other_motion.position.y);
}

void CollisionSystem::handle_collisions()
{
    std::vector<Entity> to_destroy;
    std::vector<Entity> visited;

    for (Entity entity : registry.motions.entities)
    {
        // Check if the entity is colliding with any other entity
        for (Entity other_entity : registry.motions.entities)
        {
            if (entity == other_entity)
            {
                continue;
            }

            // TODO: make this better
            bool checked_already = false;
            for (Entity check_entity : visited)
            {
                if (check_entity == other_entity)
                {
                    checked_already = true;
                }
            }
            if (checked_already)
            {
                continue;
            }

            if (!check_collision(entity, other_entity))
            {
                continue;
            }

            bool is_main_deadly = registry.deadlies.has(entity);
            bool is_other_deadly = registry.deadlies.has(other_entity);

            if (!is_main_deadly && !is_other_deadly)
            {
                // TODO
            }
            else
            {
                Entity& deadly_target = is_main_deadly ? entity : other_entity;
                Entity& other_target = is_main_deadly ? other_entity : entity;

                bool is_player = registry.players.has(other_target);

                bool is_deadly_projectile = registry.projectiles.has(deadly_target);
                bool is_other_projectile = registry.projectiles.has(other_target);


                if (!is_deadly_projectile && !is_other_projectile)
                {
                    if (!is_player)
                    {
                        // none are projectile and player isn't involved, skip
                        continue;
                    }
                    // player is involved and non are projectiles, close collision
                }

                if (is_main_deadly && is_other_deadly)
                {
                    if (is_other_projectile)
                    {
                        deadly_target = other_entity;
                        other_target = entity;
                    }
                }

                Deadly& deadly = registry.deadlies.get(deadly_target);
                    
                // DEBUG: Collisions
                if (registry.debug)
                {
                    registry.list_all_components_of(deadly_target);
                    registry.list_all_components_of(other_target);
                }

                if (registry.projectiles.has(deadly_target))
                {

                    // projectile <-> player collision
                    if (deadly.to_player && registry.players.has(other_target))
                    {
                        // TODO
                    }
                    // projectile <-> enemy collision
                    else if (deadly.to_enemy && registry.enemies.has(other_target))
                    {
                        this->applyDamage(deadly_target, other_target);
                    }
                    else
                    {
                            
                    }
                    // TODO: possible feature
                    /*else if (deadly.to_projectile && registry.projectiles.has(other_target))
                    {

                    }*/
                }

            }
        }
        visited.push_back(entity);
    }

    for (const Entity entity : to_destroy)
    {
        printf("Entity %u has been destroyed\n", static_cast<unsigned>(entity));
        registry.remove_all_components_of(entity);
        // this->renderer->removeRenderRequest(entity);
    }
}

void CollisionSystem::applyDamage(Entity attacker, Entity victim)
{
    if (registry.onHits.has(victim))
    {
        return;
    }

    const Damage& damage = registry.damages.get(attacker);
    Health& health = registry.healths.get(victim);
    if (health.health - damage.value <= 0) {
        health.health = 0;
        Death& death = registry.deaths.emplace(victim);
        death.timer = 300;
    } else {
        health.health -= damage.value;
        OnHit& hit = registry.onHits.emplace(victim);
        if (registry.players.has(victim))
        {
            hit.invincibility_timer = 3000;
        }
        else
        {
            hit.invincibility_timer = 1000;
        }
    }

    if (registry.projectiles.has(attacker))
    {
        registry.remove_all_components_of(attacker);
    }
}
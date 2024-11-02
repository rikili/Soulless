#include "core/collision_system.hpp"
#include "entities/ecs_registry.hpp"
#include "sound/sound_manager.hpp"

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

    vec2 bounding_box = motion.collider * motion.scale;
    vec2 other_bounding_box = other_motion.collider * other_motion.scale; // Its x and y means width and height

    return (
        // Horizontal intersection tests
        motion.position.x < other_motion.position.x + other_bounding_box.x &&
        other_motion.position.x < motion.position.x + bounding_box.x &&

        // Vertical intersection tests
        motion.position.y < other_motion.position.y + other_bounding_box.y &&
        other_motion.position.y < motion.position.y + bounding_box.y
        );
}

void CollisionSystem::handle_collisions()
{
    std::vector<Entity> to_destroy;
    std::vector<Entity> visited;

    // TODO: THIS IS A CRUTCH FIX (until Ricky's collision system)
    // handle barrier collision before all other collisions
    for (Entity entity : registry.motions.entities) {

        if (!registry.projectiles.has(entity) || !registry.deadlies.has(entity) || registry.deaths.has(entity)) {
            continue;
        }

        Projectile& projectile = registry.projectiles.get(entity);
        Deadly& deadly = registry.deadlies.get(entity);

        // check it's a projectile of water type + deadly to projectiles
        if (projectile.type == DamageType::water && deadly.to_projectile) {

            // check for collisions with entity projectiles
            for (Entity other_entity : registry.motions.entities) {
                if (entity == other_entity) {
                    continue;
                }
                if (registry.deaths.has(other_entity)) {
                    continue;
                }
                if (!check_collision(entity, other_entity)) {
                    continue;
                }
                if (registry.deadlies.has(other_entity)) {
                    Deadly& deadly = registry.deadlies.get(other_entity);
                    if (deadly.to_player) {
                        this->applyDamage(other_entity, entity);
                    }
                }
            }
        }
    }


    for (Entity entity : registry.motions.entities)
    {
        // Check if the entity is colliding with any other entity
        for (Entity other_entity : registry.motions.entities)
        {

            // Skip entities marked for removal
            if (registry.deaths.has(entity) || registry.deaths.has(other_entity))
            {
                continue;
            }

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

                // Assumes player isn't deadly, they shouldn't be
                bool is_player = registry.players.has(other_target);

                bool is_deadly_projectile = registry.projectiles.has(deadly_target);
                bool is_other_projectile = registry.projectiles.has(other_target);

                // Neither entities involved are projectiles
                if (!is_deadly_projectile && !is_other_projectile)
                {
                    if (!is_player)
                    {
                        // none are projectile and player isn't involved, skip
                        continue;
                    }
                    // player is involved and none are projectiles
                    // player <-> enemy collision
                    this->applyDamage(deadly_target, other_target);
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
                        this->applyDamage(deadly_target, other_target);
                    }
                    // projectile <-> enemy collision
                    else if (deadly.to_enemy && registry.enemies.has(other_target))
                    {
                        // printd("Enemy has been hit by a projectile!\n");
                        this->applyDamage(deadly_target, other_target);
                    }
                    else
                    {
                    }

                }

            }
        }
        visited.push_back(entity);
    }

    // TODO: Doesn't seem to do anything right now -- Maybe we can remove since we remove `death` entities in handleTimers()
    // for (const Entity entity : to_destroy)
    // {
    //     printf("Entity %u has been destroyed\n", static_cast<unsigned>(entity));
    //     registry.remove_all_components_of(entity);
    //     // this->renderer->removeRenderRequest(entity);
    // }
}

void CollisionSystem::applyDamage(Entity attacker, Entity victim)
{
    if (registry.onHits.has(victim))
    {
        return;
    }

    SoundManager* soundManager = SoundManager::getSoundManager();

    if (registry.healths.has(victim)) {
        const Damage& damage = registry.damages.get(attacker);
        Health& health = registry.healths.get(victim);
        if (health.health - damage.value <= 0) {
            health.health = 0;
            Death& death = registry.deaths.emplace(victim);

            if (registry.players.has(victim))
            {
                printd("Player has died!\n");
                // set player velocity to 0: to prevent bug where player moves after death
                Motion& motion = registry.motions.get(victim);
                motion.velocity = { 0.0f, 0.0f };
                soundManager->playSound(SoundEffect::PLAYER_DEFEATED);
                soundManager->playMusic(Song::DEFEAT);
                death.timer = 7000;
            }
            else {
                death.timer = 10;
            }

        }
        else {
            // TODO: Need to change based on entity type
            if (!registry.players.has(victim)) {
                soundManager->playSound(SoundEffect::VILLAGER_DAMAGE);
            }
            else if (registry.players.has(victim)) {
                soundManager->playSound(SoundEffect::PITCHFORK_DAMAGE);
            }

            health.health -= damage.value;
            OnHit& hit = registry.onHits.emplace(victim);
            if (registry.players.has(victim))
            {
                printd("Player has been hit! Remaining health: %f\n", health.health);
                hit.invincibility_timer = PLAYER_INVINCIBILITY_TIMER;
            }
            else
            {
                hit.invincibility_timer = ENEMY_INVINCIBILITY_TIMER;
            }
        }
    }
    // victim doesn't have health
    else {
        Projectile& victim_projectile = registry.projectiles.get(victim);
        if (victim_projectile.type == DamageType::water) {
            registry.deaths.emplace(victim);
        }
    }

    if (registry.projectiles.has(attacker) && !registry.deaths.has(attacker))
    {
        Projectile& attacker_projectile = registry.projectiles.get(attacker);

        // lightning spell shouldn't die after hitting one target
        if (attacker_projectile.type == DamageType::lightning)
        {
            // do nothing
        }
        else {
            registry.deaths.emplace(attacker);
        }

        // printd("Marked for removal due to collision -> Entity value: %u\n", static_cast<unsigned>(attacker));
    }
}

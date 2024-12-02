#include "core/collision_system.hpp"
#include "entities/ecs_registry.hpp"
#include "utils/spell_factory.hpp"
#include "sound/sound_manager.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <random>

CollisionSystem::CollisionSystem(IRenderSystem* renderer)
{
    this->renderer = renderer;
}

void CollisionSystem::init()
{
}

static std::vector<vec2> get_corners(const Entity& entity)
{
    const Motion& motion = registry.motions.get(entity);
    vec2 bounding_box = motion.collider / 2.f;
    return std::vector<vec2>({
        { motion.position - bounding_box },
        { motion.position + vec2({ bounding_box.x, -bounding_box.y })},
        { motion.position + vec2({ -bounding_box.x, bounding_box.y })},
        { motion.position + bounding_box }
        });
}

static bool check_collision(const Entity& entity, const Entity& other_entity)
{
    Motion& motion = registry.motions.get(entity);
    Motion& other_motion = registry.motions.get(other_entity);

    vec2 bounding_box = motion.collider / 2.f;
    vec2 other_bounding_box = other_motion.collider / 2.f; // Its x and y means width and height

    return (
        std::abs(motion.position.x - other_motion.position.x) <= bounding_box.x + other_bounding_box.x
        && std::abs(motion.position.y - other_motion.position.y) <= bounding_box.y + other_bounding_box.y
        );
}

static mat4 create_transform(const Motion& motion, bool do_rotate = false)
{
    mat4 transform = mat4(1.f);
    transform = translate(transform, glm::vec3({ motion.position, 1.0f }));
    transform = scale(transform, vec3({ motion.collider / 2.f, 1.f }));
    if (do_rotate) rotate(transform, motion.angle, { 0.f, 0.f, 1.f });
    return transform;
}

static vec2 transformed_vertex(const vec2& vertex, const mat4& transform)
{
    vec4 transformed = transform * vec4({ vertex.x, vertex.y, 1.f, 1.f });
    return { transformed.x, transformed.y };
}

static void draw_debug(const vec2& vertex, const vec2& other_vertex, vec3 color = { 1.f, 0.f, 0.f }, DebugType type = DebugType::outline)
{
    vec2 line_len = vec2({ 1.f, distance(vertex, other_vertex) / 2.f });
    vec2 line_pos = (vertex + other_vertex) / 2.f;
    vec2 diff = other_vertex - vertex;
    float angle = std::atan2(diff.y, diff.x) + M_PI / 2.f;

    Entity new_debug;
    DebugRequest& vertex_debug = registry.debug_requests.emplace(new_debug);
    vertex_debug.collider = line_len;
    vertex_debug.position = line_pos;
    vertex_debug.angle = angle;
    vertex_debug.color = color;
    vertex_debug.type = type;
}

static void draw_bounding_box(const Entity& entity, const vec3 color = { 1.f, 0.f, 0.f })
{
    Motion& motion = registry.motions.get(entity);
    const Entity box_entity;
    DebugRequest& debug = registry.debug_requests.emplace(box_entity);
    debug.position = motion.position;
    debug.collider = motion.collider / 2.f;
    debug.color = color;
}

static void draw_vertices(const Entity& entity, const Mesh& mesh)
{
    const Motion& motion = registry.motions.get(entity);
    mat4 transform = create_transform(motion);

    for (int i = 0; i < mesh.vertices.size(); i += 3)
    {
        Entity vertex_ent;
        DebugRequest& vertex_debug = registry.debug_requests.emplace(vertex_ent);
        vertex_debug.collider = { 1, 1 };
        vertex_debug.type = DebugType::fill;
        vertex_debug.position = transformed_vertex({ mesh.vertices[i], mesh.vertices[i + 1] }, transform);
    }
}

static float get_normal_point(const vec2& point, const vec2& normal)
{
    return dot(point, normal) / length(normal);
}

static vec2 project_to_normal(const std::vector<vec2>& vertices, const vec2& normal)
{
    float min = INFINITY;
    float max = -INFINITY;
    float project_val;

    for (const vec2& vertex : vertices)
    {
        project_val = get_normal_point(vertex, normal);
        if (project_val > max) max = project_val;
        if (project_val < min) min = project_val;
    }

    return { min, max };
}

// specifically triangle to rectangle check
static bool check_overlap(const std::vector<vec2>& vertices, const std::vector<vec2>& corners)
{
    std::vector<vec2> normals;
    for (int i = 0; i < vertices.size(); i++)
    {
        vec2 edge = vertices[(i + 1) % vertices.size()] - vertices[i];
        normals.push_back({ -edge.y, edge.x });
    }
    normals.push_back({ 1, 0 });
    normals.push_back({ 0, 1 });

    for (const vec2& normal : normals)
    {
        vec2 triangle_min_max = project_to_normal(vertices, normal);
        vec2 rect_min_max = project_to_normal(corners, normal);
        if (triangle_min_max[1] < rect_min_max[0] || rect_min_max[1] < triangle_min_max[0])
        {
            return false;
        }
    }
    return true;
}

void CollisionSystem::detect_collisions()
{
    std::unordered_set<Entity> visited;
    for (const Entity& entity : registry.motions.entities)
    {
        if (registry.deaths.has(entity)) {
            visited.insert(entity);
            continue;
        }

        if (registry.players.has(entity) && registry.debug)
        {
            draw_vertices(entity, *renderer->getMesh("mage_collider")); // can fetch mesh from component as well
        }

        for (const Entity& other_entity : registry.motions.entities)
        {
            if (entity == other_entity) continue;
            if (visited.find(other_entity) != visited.end()) continue;
            if (registry.deaths.has(other_entity)) continue;
            if (check_collision(entity, other_entity))
            {
                registry.collision_registry.register_collision(entity, other_entity);
            }
        }
        visited.insert(entity);
    }

    if (registry.debug)
    {
        for (const Entity& entity : registry.motions.entities)
        {
            if (registry.collision_registry.get_collision_by_ent(entity).size() > 0)
            {
                draw_bounding_box(entity, { 0.f, 1.f, 0.f });
            }
            else
            {
                draw_bounding_box(entity);
            }
        }
    }
}

bool CollisionSystem::is_mesh_colliding(const Entity& primary, const Entity& other_entity)
{
    assert(registry.mesh_colliders.has(primary) && "Missing collider mesh for target");

    const Motion& motion = registry.motions.get(primary);
    const Mesh collision_mesh = *renderer->getMesh(registry.mesh_colliders.get(primary).mesh);

    mat4 transform = create_transform(motion);
    std::vector<vec2> vertices;

    for (int i = 0; i < collision_mesh.vertices.size(); i += 3)
    {
        vertices.push_back({ collision_mesh.vertices[i], collision_mesh.vertices[i + 1] });
    }

    bool result = false;
    for (int i = 0; i < collision_mesh.indices.size(); i += 3)
    {
        const vec2 vertex1 = transformed_vertex(vertices[collision_mesh.indices[i]], transform);
        const vec2 vertex2 = transformed_vertex(vertices[collision_mesh.indices[i + 1]], transform);
        const vec2 vertex3 = transformed_vertex(vertices[collision_mesh.indices[i + 2]], transform);
        std::vector<vec2> triangle = std::vector<vec2>({ vertex1, vertex2, vertex3 });

        if (check_overlap(triangle, get_corners(other_entity)))
        {
            if (registry.debug)
            {
                draw_debug(vertex1, vertex2, { 0.f, 1.f, 0.f }, DebugType::fill);
                draw_debug(vertex2, vertex3, { 0.f, 1.f, 0.f }, DebugType::fill);
                draw_debug(vertex3, vertex1, { 0.f, 1.f, 0.f }, DebugType::fill);
            }
            result = true;
        }
    }
    return result;
}

bool CollisionSystem::isWaterProtected(const Entity& player, const Entity& attacker)
{
    for (auto& spell : registry.spellStates.entities)
    {
        SpellState& state = registry.spellStates.get(spell);
        if (state.isBarrier)
        {
            if (state.seen.find(attacker) != state.seen.end()) return true;

            RenderRequest& request = registry.render_requests.get(spell);
            state.barrier_level++;
            if (state.barrier_level >= 3) state.barrier_level = 3;
            state.seen.insert(attacker);
            printf("level %d\n", state.barrier_level);
            request.texture = "barrier-" + std::to_string(state.barrier_level);
            return true;
        }
    }
    return false;
}

void CollisionSystem::resolve_collisions()
{
    // Ordering matters for which types of entites are checked (careful when changing)

    // Water Barrier with other projectile collisions (can't be done within the projectile <-> projectile logic below)
    std::unordered_map<SpellType, int, SpellTypeHash> cycle_progress;
    std::unordered_map<PostResolution, std::pair<Entity, std::vector<Entity>>> post_resolutions;
    std::unordered_set<Entity> to_deactivate;
    std::unordered_set<Entity> to_delete;

    // Projectile collisions
    for (const Entity& proj_entity : registry.projectiles.entities)
    {
        Projectile& projectile = registry.projectiles.get(proj_entity);

        std::unordered_set<Entity> other_entities = registry.collision_registry.get_collision_by_ent(proj_entity);
        const Deadly& deadly = registry.deadlies.get(proj_entity);
        for (const Entity& other_entity : other_entities)
        {

            // projectile <-> player
            if (registry.players.has(other_entity) && deadly.to_player)
            {
                if (is_mesh_colliding(other_entity, proj_entity))
                {
                    HitTypes hit_response = applyDamage(proj_entity, other_entity, cycle_progress);
                    if (hit_response == HitTypes::hit || hit_response == HitTypes::absorbed)
                    {
                        registry.deaths.emplace(proj_entity);
                    }
                    projectile.isActive = false;
                }
            }

            // projectile <-> enemies
            else if (registry.enemies.has(other_entity) && deadly.to_enemy)
            {
                // if not a spell, currently not a case OR inactive
                if (!registry.spellProjectiles.has(proj_entity))
                {
                    registry.collision_registry.remove_collision(proj_entity, other_entity);
                    continue;
                }

                // must be spell
                SpellProjectile& spell_proj = registry.spellProjectiles.get(proj_entity);
                bool isMaxLevel = spell_proj.level >= MAX_SPELL_LEVEL;
                bool didHit = false;

                // Max level wind pull effect
                if (isMaxLevel && spell_proj.type == SpellType::WIND) {
                    spell_proj.victims.insert(other_entity);
                    Motion& enemyMotion = registry.motions.get(other_entity);
                    Motion& windMotion = registry.motions.get(proj_entity);
                    vec2 direction_normalized = glm::normalize(windMotion.position - enemyMotion.position);
                    enemyMotion.velocity = direction_normalized * 0.05f;
                    registry.enemies.get(other_entity).movementRestricted = true;
                }

                if (projectile.isActive)
                {
                    didHit = applyDamage(proj_entity, other_entity, cycle_progress, !isMaxLevel) == HitTypes::hit;
                }

                if (isMaxLevel)
                {
                    switch (spell_proj.type)
                    {
                    case (SpellType::FIRE):
                    {
                        if (spell_proj.isPostAttack)
                        {
                            // splash
                            to_deactivate.insert(proj_entity);
                        }
                        else
                        {
                            // projectile
                            to_delete.insert(proj_entity);
                            if (post_resolutions.find(PostResolution::FIRE_PROJECTILE) != post_resolutions.end())
                            {
                                post_resolutions[PostResolution::FIRE_PROJECTILE].second.push_back(other_entity);
                            }
                            else
                            {
                                post_resolutions[PostResolution::FIRE_PROJECTILE].first = proj_entity;
                                post_resolutions[PostResolution::FIRE_PROJECTILE].second.push_back(other_entity);
                            }
                        }
                    }
                    case (SpellType::LIGHTNING):
                    {
                        to_deactivate.insert(proj_entity);
                    }
                    case (SpellType::WATER): {}
                    }
                }
                else
                {
                    switch (spell_proj.type)
                    {
                    case (SpellType::WATER):
                    {
                        to_deactivate.insert(proj_entity);
                        break;
                    }
                    case (SpellType::ICE):
                    case (SpellType::FIRE):
                    {
                        projectile.isActive = false;
                        to_delete.insert(proj_entity);
                    }
                    case (SpellType::LIGHTNING):
                    case (SpellType::PLASMA):
                    case (SpellType::WIND):
                    {
                        break;
                    }
                    }
                }
            }

            else if (registry.projectiles.has(other_entity) && deadly.to_projectile)
            {
                // projectile <-> projectile interaction
            }

            registry.collision_registry.remove_collision(proj_entity, other_entity);
        }
    }

    // Enemies collisions
    for (const Entity& enemy_entity : registry.enemies.entities)
    {
        std::unordered_set<Entity> other_entities = registry.collision_registry.get_collision_by_ent(enemy_entity);
        const Deadly& deadly = registry.deadlies.get(enemy_entity);
        for (const Entity& other_entity : other_entities)
        {

            if (registry.players.has(other_entity) && deadly.to_player)
            {
                if (is_mesh_colliding(other_entity, enemy_entity))
                {
                    applyDamage(enemy_entity, other_entity, cycle_progress);
                }
            }

            else if (registry.enemies.has(other_entity) && deadly.to_enemy)
            {
                // enemy <-> enemy interaction
            }

            registry.collision_registry.remove_collision(enemy_entity, other_entity);
        }
    }

    // TODO: add check for terrain collisions

    for (const Entity& interactable_entity : registry.interactables.entities)
    {
        std::unordered_set<Entity> other_entities = registry.collision_registry.get_collision_by_ent(interactable_entity);
        const Interactable& interactable = registry.interactables.get(interactable_entity);
        for (const Entity& other_entity : other_entities)
        {
            if (!registry.collision_registry.check_collision(interactable_entity, other_entity)) continue;
            if (registry.players.has(other_entity))
            {
                if (interactable.type == InteractableType::POWER && is_mesh_colliding(other_entity, interactable_entity))
                {
                    SoundManager* sound = SoundManager::getSoundManager();
                    sound->playSound(SoundEffect::POWERUP_PICKUP);

                    SpellUnlock& unlock = registry.spellUnlocks.get(interactable_entity);
                    pickupSpell(other_entity, unlock.type);
                    Decay& decay = registry.decays.get(interactable_entity);
                    decay.timer = 0;
                }
                if (interactable.type == InteractableType::BOSS)
                {
                    interactProx.in_proximity = Proximity::BOSS_ALTAR;
                }
                else if (interactable.type == InteractableType::PLASMA)
                {
                    interactProx.in_proximity = Proximity::PLASMA_SUMMON;
                }
            }

            registry.collision_registry.remove_collision(interactable_entity, other_entity);
        }
    }

    // Post-collision resolutions
    Player& player = registry.players.get(registry.players.entities[0]);
    for (const auto& spell : cycle_progress)
    {
        player.spell_queue.addProgressSpell(spell.first, spell.second);
    }
    resolve_post_effects(post_resolutions);
    post_resolutions.clear();
    for (const Entity& ent : to_delete)
    {
        registry.deaths.emplace(ent);
    }
    for (const Entity& ent : to_deactivate)
    {
        Projectile& proj = registry.projectiles.get(ent);
        proj.isActive = false;
    }

}

void CollisionSystem::resolve_post_effects(std::unordered_map<PostResolution, std::pair<Entity, std::vector<Entity>>> resolutions)
{
    for (auto& resolution : resolutions)
    {
        switch (resolution.first)
        {
        case PostResolution::FIRE_PROJECTILE:
        {
            for (const Entity& target : resolution.second.second)
            {
                Motion& motion = registry.motions.get(target);
                SpellFactory::createSpellResolution(registry, motion.position, PostResolution::FIRE_PROJECTILE, resolution.second.first);

            }
        }
        }
    }
}

HitTypes CollisionSystem::applyDamage(Entity attacker, Entity victim, std::unordered_map<SpellType, int, SpellTypeHash>& tracker, bool do_scaling)
{
    SoundManager* soundManager = SoundManager::getSoundManager();

    if (registry.players.has(victim) && registry.projectiles.has(attacker) && registry.projectiles.get(attacker).type == DamageType::portal && !registry.deaths.has(victim)) {
        soundManager->playSound(SoundEffect::PORTAL_DAMAGE);
        Motion& playerMotion = registry.motions.get(victim);
        Projectile& projectile = registry.projectiles.get(attacker);
        playerMotion.position = projectile.sourcePosition;
        Debuff& debuff = registry.debuffs.emplace(victim);
        debuff.type = DebuffType::SLOW;
        debuff.timer = 2000.f;
        debuff.strength = 0.3f;

        registry.deaths.emplace(attacker);
        return HitTypes::notHit;
    }

    if (registry.healths.has(victim)) {
        const Damage& damage = registry.damages.get(attacker);
        float damageValue = damage.value;

        // check on-hits
        if (registry.onHits.has(victim)) {
            OnHit& hit = registry.onHits.get(victim);

            if (hit.invuln_tracker[victim] > 0.f && hit.isAllImmune)
            {
                return HitTypes::notHit;
            }
            else
            {
                if (hit.invuln_tracker.find(attacker) != hit.invuln_tracker.end())
                {
                    if (hit.invuln_tracker[attacker] > 0.f) return HitTypes::notHit;
                }
            }
        }

        if (do_scaling && registry.spellProjectiles.has(attacker))
        {
            const SpellProjectile& proj = registry.spellProjectiles.get(attacker);
            switch (proj.type)
            {
            case SpellType::WATER:
                damageValue *= WATER_SCALING[proj.level - 1];
                break;
            case SpellType::FIRE:
                damageValue *= FIRE_SCALING[proj.level - 1];
                break;
            case SpellType::LIGHTNING:
                damageValue *= LIGHTNING_SCALING[proj.level - 1];
                break;
            case SpellType::ICE:
                damageValue *= ICE_SCALING[proj.level - 1];
                break;

            }
        }

        Health& health = registry.healths.get(victim);

        if (registry.players.has(victim))
        {
            if (isWaterProtected(victim, attacker)) return HitTypes::absorbed;
        }

        if (registry.enemies.has(victim)) {
            const SpellProjectile& proj = registry.spellProjectiles.get(attacker);

            // Knight will block non-max fire and ice spells
            if (registry.enemies.get(victim).blocking &&
                ((proj.type == SpellType::FIRE || proj.type == SpellType::ICE) && proj.level != MAX_SPELL_LEVEL)) {
                soundManager->playSound(SoundEffect::SHIELD_BLOCK);
                return HitTypes::absorbed;
            }
        }

        printd("Damage dealt: %f\n", damageValue);

        // if damage is greater than remaining health
        if (health.health - damageValue <= 0 && !registry.deaths.has(victim)) {
            health.health = 0;
            Death& death = registry.deaths.emplace(victim);
            bool bossDeath = false;

            // if player death
            if (registry.players.has(victim))
            {
                printd("Player has died!\n");
                soundManager->playSound(SoundEffect::PLAYER_DEFEATED);

                if (!soundManager->isMusicPlaying()) {
                    soundManager->toggleMusic();
                }
                soundManager->playMusic(Song::DEFEAT);
                death.timer = 7000;
            }
            else {
                // Assumed this has to be an enemy
                death.timer = 1000.f;
                if (registry.enemies.get(victim).type == EnemyType::DARKLORD) {
                    death.timer = 4000.f;
                    bossDeath = true;
                    soundManager->toggleMusic();
                    soundManager->playSound(SoundEffect::CHOIR);
                }
            }

            if (registry.spellProjectiles.has(attacker) && registry.enemies.has(victim))
            {
                const SpellProjectile& spell = registry.spellProjectiles.get(attacker);
                if (tracker[spell.type])
                {
                    tracker[spell.type]++;
                }
                else
                {
                    tracker[spell.type] = 1;
                }

            }

            // Stop movement and remove from AI system to prevent any future movement
            Motion& motion = registry.motions.get(victim);
            motion.velocity = { 0.0f, 0.0f };
            registry.ai_systems.remove(victim);

            // Trigger death animation
            Animation& dead_ent_animation = registry.animations.get(victim);
            dead_ent_animation.state = AnimationState::DYING;
            dead_ent_animation.frameTime = 50.f;
            if (bossDeath) {
                dead_ent_animation.frameTime = 175.f;
            }
            dead_ent_animation.initializeAtRow((int)motion.currentDirection);

        }
        else {
            // TODO: Need to change based on entity type
            if (!registry.players.has(victim)) {
                soundManager->playSound(SoundEffect::VILLAGER_DAMAGE);
            }
            else if (registry.players.has(victim)) {
                soundManager->playSound(SoundEffect::PITCHFORK_DAMAGE);
            }

            health.health -= damageValue;

            if (registry.players.has(victim))
            {
                OnHit& onHit = registry.onHits.has(victim) ? registry.onHits.get(victim) : registry.onHits.emplace(victim);
                printd("Player has been hit! Remaining health: %f\n", health.health);
                onHit.isAllImmune = true;
                onHit.invuln_tracker[victim] = PLAYER_INVINCIBILITY_TIMER;
            }
            else if (registry.enemies.has(victim))
            {
                OnHit& onHit = registry.onHits.has(victim) ? registry.onHits.get(victim) : registry.onHits.emplace(victim);
                // add invulnerability timer for entity
                if (damage.type == DamageType::wind || damage.type == DamageType::plasma)
                {
                    onHit.invuln_tracker[attacker] = ENEMY_INVINCIBILITY_TIMER;
                }

                onHit.invuln_tracker[victim] = ENEMY_INVINCIBILITY_TIMER;
            }
            return HitTypes::hit;
        }
    }
    // projectile <-> projectile -- projectiles don't have health
    else {
    }
    return HitTypes::notHit;
}

void CollisionSystem::applyHealing(Entity target)
{
    Health& health = registry.healths.get(target);

    if (registry.onHeals.has(target) || health.health == PLAYER_HEALTH) {
        return;
    }

    health.health = std::min(PLAYER_HEALTH, health.health + 10.0f);
    printd("Player has been healed! New health: %f\n", health.health);

    OnHeal& heal = registry.onHeals.emplace(target);
    heal.heal_time = PLAYER_HEAL_COOLDOWN;
}

void CollisionSystem::pickupSpell(Entity target, SpellType type)
{
    Player& player = registry.players.get(target);
    if (player.spell_queue.hasSpell(type))
    {
        player.spell_queue.addProgressSpell(type, 10);
    }
    else
    {
        player.spell_queue.levelSpell(type);
    }
}

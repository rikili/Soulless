#include "core/collision_system.hpp"
#include "entities/ecs_registry.hpp"
#include "sound/sound_manager.hpp"
#include <glm/ext/matrix_clip_space.hpp>

CollisionSystem::CollisionSystem(RenderSystem* renderer)
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

static void draw_bounding_box(const Entity& entity, const vec3 color = {1.f, 0.f, 0.f})
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

        bool collided = false;
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
                draw_debug(vertex3, vertex1,  { 0.f, 1.f, 0.f }, DebugType::fill);
            }
            result = true;
        }
    }
    return result;
}

void CollisionSystem::resolve_collisions()
{
    // Ordering matters for which types of entites are checked (careful when changing)

    // Projectile collisions
    for (const Entity& proj_entity : registry.projectiles.entities)
    {
        std::unordered_set<Entity> other_entities = registry.collision_registry.get_collision_by_ent(proj_entity);
        const Deadly& deadly = registry.deadlies.get(proj_entity);
        for (const Entity& other_entity : other_entities)
        {
            if (!registry.collision_registry.check_collision(proj_entity, other_entity)) continue;
            if (registry.players.has(other_entity) && deadly.to_player)
            {
                if (is_mesh_colliding(other_entity, proj_entity)) applyDamage(proj_entity, other_entity);
            }
            else if (registry.enemies.has(other_entity) && deadly.to_enemy)
            {
                applyDamage(proj_entity, other_entity);
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
            if (!registry.collision_registry.check_collision(enemy_entity, other_entity)) continue;
            if (registry.players.has(other_entity) && deadly.to_player)
            {
                if (is_mesh_colliding(other_entity, enemy_entity)) applyDamage(enemy_entity, other_entity);
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
                if (is_mesh_colliding(other_entity, interactable_entity)) applyHealing(other_entity);
            }

            registry.collision_registry.remove_collision(interactable_entity, other_entity);
        }
    }
}

void CollisionSystem::applyDamage(Entity attacker, Entity victim)
{
    if (registry.onHits.has(victim))
    {
        return;
    }

    SoundManager* soundManager = SoundManager::getSoundManager();

    const Damage& damage = registry.damages.get(attacker);
    Health& health = registry.healths.get(victim);
    if (health.health - damage.value <= 0 && !registry.deaths.has(victim)) {
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

    if (registry.projectiles.has(attacker) && !registry.deaths.has(attacker))
    {
        registry.deaths.emplace(attacker);
        // printd("Marked for removal due to collision -> Entity value: %u\n", static_cast<unsigned>(attacker));
    }
}

void CollisionSystem::applyHealing(Entity target)
{
    Health& health = registry.healths.get(target);

    if (registry.onHeals.has(target) || health.health == PLAYER_HEALTH) {
        return;
    }

    health.health += 10.f;
    printd("Player has been healed! New health: %f\n", health.health);

    OnHeal& heal = registry.onHeals.emplace(target);
    heal.heal_time = PLAYER_HEAL_COOLDOWN;
}

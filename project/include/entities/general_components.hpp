#pragma once

#include <core/common.hpp>
#include <utils/constants.hpp>
#include <utils/spell_queue.hpp>
#include <utils/state.hpp>
#include <map>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <glm/vec2.hpp>

using AssetId = std::string;

// Motion Component
struct Motion {
    vec2 position = { 0, 0 };
    vec2 velocity = { 0, 0 };
    vec2 scale = { 0.75, 0.75 };
    vec2 collider = { 25, 25 };

    float mass = 0;
    float speedModifier = 1.f;
    float angle = 0;

    Direction currentDirection = Direction::E;
    Direction oldDirection = Direction::E;
};

// Resistance Modifier
struct ResistanceModifier {};


// Health Component
struct Health
{
    float health = 0;
    float maxHealth = 0;
    ResistanceModifier resistance_modifier;
};

struct HealthBar
{
    Entity assignedTo{};
    bool assigned = false;
    void assignHealthBar(Entity& ent) {
        this->assignedTo = ent;
        assigned = true;
    }

    vec2 position;
    vec2 scale = { 0.5f ,0.5f };
};

// Damage Component
struct Damage
{
    float value = 0;
    DamageType type = DamageType::enemy;
};

// Type Components
struct Enemy {
    EnemyType type;
    float range = 0;
    float cooldown = -1.f;
    float secondCooldown = -1.f;
    bool movementRestricted = false;
};

struct Player {
    float leftCooldown = -1.f;
    float leftCooldownTotal = -1.f;
    float rightCooldown = -1.f;
    float rightCooldownTotal = -1.f;
    SpellQueue spell_queue;
};

struct Projectile {
    DamageType type;
    float range = 0;
    vec2 sourcePosition = vec2(0.0f, 0.0f);
    bool isActive = false;
};

struct SpellProjectile {
    SpellType type = SpellType::COUNT;
    int level = 1;
    bool isPostAttack = false;
    std::unordered_set<Entity> victims;
};

struct Interactable {
    InteractableType type;
};

struct SpellUnlock {
    SpellType type;
};

struct Decay {
    float timer = 0;
};

enum DebuffType {
    SLOW
};

struct Debuff {
    DebuffType type = SLOW;
    float timer = 0;
    float strength = 0;
};

// Timed Component
// struct Timed {
//     float timer = 0;
// }; // include temporary effects and counters

// Spell State Component
struct SpellState {
    //SpellType type = SpellType::COUNT;
    State state = State::CASTING;
    float timer = 0;

    // lightning vars
    bool isChild = false;

    // water barrier vars
    bool isBarrier = false;
    int barrier_level = 0;
    std::unordered_set<Entity> seen;
};

// Structure to store collision information
struct Collision
{
    // Note, the first object is stored in the ECS container.entities
    Entity other{}; // the second object involved in the collision
    explicit Collision(Entity& other) { this->other = other; };
};

// Structure to store information on being hit, setting same entity as attached as bypass for player immunity (TODO)
struct OnHit
{
    bool isAllImmune = false; // is immune to all damage
    bool invicibilityShader = false;
    bool isInvincible = false; // toggled to true when tracker is populated
    std::unordered_map<int, float> invuln_tracker;
};

// Structure to store information on being healed
struct OnHeal
{
    float heal_time = 0;
};

// Structure to store entities marked to die
struct Death
{
    float timer = 2;
};

// Structure to store what entities affect/damage other entities
struct Deadly
{
    bool to_player = false;
    bool to_enemy = false;
    bool to_projectile = false;
};

// Smooth position component to handle z-fighting
struct SmoothPosition {
    float render_y;
    static constexpr float HYSTERESIS = 5.0f;
    static constexpr float SMOOTHING_FACTOR = 0.1f;

    void update(const float actual_y) {
        const float diff = actual_y - render_y;
        if (std::abs(diff) > HYSTERESIS) {
            render_y += diff * SMOOTHING_FACTOR;
        }
    }
};

struct RenderRequest
{
    AssetId mesh = "";
    AssetId texture = "";
    AssetId shader = "";
    unsigned int type = BACK;
    SmoothPosition smooth_position;

};

enum class DebugType
{
    outline,
    fill
};
struct DebugRequest
{
    vec2 position = { 0, 0 };
    vec2 collider = { 0, 0 };
    float angle = 0.f;
    vec3 color = { 1.0f, 0.0f, 0.0f };
    DebugType type = DebugType::outline;
};

struct MeshCollider
{
    AssetId mesh = "";
};

struct Animation
{
    bool oneTime = false;

    float startFrame = 0.0f;
    float currentFrame;

    float frameTime = DEFAULT_LOOP_TIME;
    float elapsedTime = 0.f;

    int spriteCols = 4;
    int spriteRows = 1;
    int spriteCount = 4;

    int frameCount = 4;

    EntityState state = EntityState::IDLE;

    void initializeAtFrame(float frame) {
        startFrame = frame;
        currentFrame = startFrame;
    }

    void initializeAtRow(int row) {
        startFrame = row * spriteCols * 1.0;
        currentFrame = startFrame;
    }
};

struct Camera
{
    vec2 position = { 0, 0 };
};

struct GlobalOptions {
    bool tutorial = true;
    int showingTab = 0;
    bool pause = false;
    int fps = 0;
    bool showFps = false;
    bool loadingOldGame = false;

    // -- DEBUG options, will refactor --
    bool debugSpellSpawn = false;
};

extern GlobalOptions globalOptions;

struct EnemySpawnTimers {
    float knight = 0.0f;
    float archer = 60000.0f;
    float paladin = 120000.0f;
    float slasher = 180000.0f;
    float darklord = 0.0f; // 4.5 min

    std::unordered_map<std::string, float&> asMap() {
        return {
            {"knight", knight},
            {"archer", archer},
            {"paladin", paladin},
            {"slasher", slasher},
            {"darklord", darklord}
        };
    }
};
extern EnemySpawnTimers enemySpawnTimers;

enum TileType {
    GRASS1,
    GRASS2,
    GRASS3,
    GRASS4,
    GRASS5,
    CLAY1,
    CLAY2,
    CLAY3,
};


struct Tile {
    TileType type;
    vec2 position;
    vec2 scale;

};

struct Particle {
    // Do not change this order
    vec2 position;
    float size;
    vec3 color = { .5f, .5f, .5f };

    vec2 velocity;
    float lifetime = 1000;
};

// source: inclass simpleGL-3
struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2 Size;         // Size of glyph
    glm::ivec2 Bearing;      // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
    char character;          // The character represented by this glyph
};

struct Font {
    std::map<char, Character> m_ftCharacters;
    float size;
    GLuint vao = 0;
    GLuint vbo = 0;
};

struct VertexAttribute {
    GLint size;
    GLenum type;
    GLboolean normalized;
    const char* semanticName;
};

struct Mesh {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    GLuint instanceVBO = 0;
    size_t vertexCount = 0;
    size_t indexCount = 0;
    std::vector<float> vertices;
    std::vector<uint32_t> indices;
    std::vector<VertexAttribute> attributes;
};

struct Texture {
    GLuint handle = 0;
    glm::ivec2 dimensions{ 0, 0 };
};

struct Shader {
    GLuint program = 0;
    std::string vertexPath;
    std::string fragmentPath;
};

struct Material {
    AssetId shader;
    AssetId texture;
    glm::vec4 color{ 1.0f };
    // Add other material properties as needed
};

struct Transform2D {
    glm::vec2 position = glm::vec2(0.0f);
    float rotation = 0.0f;
    glm::vec2 scale = glm::vec2(1.0f);

    glm::mat3 getMatrix() const {
        glm::mat3 transform(1.0f);

        // Translation
        transform[2][0] = position.x;
        transform[2][1] = position.y;

        // Rotation
        float cosR = cos(rotation);
        float sinR = sin(rotation);
        glm::mat3 rotationMat(1.0f);
        rotationMat[0][0] = cosR;
        rotationMat[0][1] = sinR;
        rotationMat[1][0] = -sinR;
        rotationMat[1][1] = cosR;
        transform = transform * rotationMat;

        // Scale
        transform[0][0] *= scale.x;
        transform[1][1] *= scale.y;

        return transform;
    }
};

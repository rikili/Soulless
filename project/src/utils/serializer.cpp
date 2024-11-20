#include "utils/serializer.hpp"
#include "entities/ecs_registry.hpp"
#include "utils/enemy_factory.hpp"

Serializer::Serializer() {}

void Serializer::serialize() {
    json jsonData;

    json playerData;
    Entity& player_ent = registry.players.entities[0];
    Player& player = registry.players.get(player_ent);

    Health &health = registry.healths.get(player_ent);
    playerData["health"] = std::to_string(health.health);

    Motion &motion = registry.motions.get(player_ent);
    json motionData;
    motionData["position"] = { motion.position.x, motion.position.y };
    playerData["motion"] = motionData;

    SpellQueue& spell_queue = player.spell_queue;
    playerData["leftSpell"] = static_cast<int>(spell_queue.getLeftSpell());
    playerData["rightSpell"] = static_cast<int>(spell_queue.getRightSpell());
    playerData["spellQueue"] = json::array();
    for (const SpellType& spell : spell_queue.getQueue()) {
        playerData["spellQueue"].push_back(static_cast<int>(spell));
    }

    jsonData["player"] = playerData;

    jsonData["enemies"] = json::array();
    for (const Entity& enemy_entity : registry.enemies.entities) {
        Enemy &enemy = registry.enemies.get(enemy_entity);
        json enemyData;
        enemyData["type"] = static_cast<int>(enemy.type);
        enemyData["cooldown"] = enemy.cooldown;

        Health &health = registry.healths.get(enemy_entity);
        enemyData["health"] = std::to_string(health.health);

        Motion &motion = registry.motions.get(enemy_entity);
        json motionData;
        motionData["position"] = { motion.position.x, motion.position.y };
        motionData["velocity"] = { motion.velocity.x, motion.velocity.y };
        enemyData["motion"] = motionData;

        jsonData["enemies"].push_back(enemyData);
    }

    json timersData;

    for (const auto& pair : enemySpawnTimers.asMap()) {
        const std::string& key = pair.first;
        const float& value = pair.second;
        timersData[key] = value;
    }

    jsonData["enemySpawnTimers"] = timersData;

    std::ofstream file("data.json");
    if (file.is_open()) {
        file << jsonData.dump(4);
        file.close();
    } else {
        std::cerr << "Error saving game state" << std::endl;
    }
}

void Serializer::deserialize() {
    std::ifstream file("data.json");
    if (!file.is_open()) {
        std::cerr << "Error opening data.json for reading" << std::endl;
        return;
    }

    json jsonData;
    file >> jsonData;
    file.close();

    Entity& player_ent = registry.players.entities[0];
    Player& player = registry.players.get(player_ent);

    const auto playerData = jsonData["player"];

    glm::vec2 position = { playerData["motion"]["position"][0].get<float>(), 
                           playerData["motion"]["position"][1].get<float>() };

    Motion &motion = registry.motions.get(player_ent);
    motion.position = position;

    Health &health = registry.healths.get(player_ent);
    health.health = std::stof(playerData["health"].get<std::string>());

    SpellQueue& spell_queue = player.spell_queue;

    spell_queue.setLeftSpell(static_cast<SpellType>(playerData["leftSpell"].get<int>()));
    spell_queue.setRightSpell(static_cast<SpellType>(playerData["rightSpell"].get<int>()));

    const auto& spellQueueData = playerData["spellQueue"];
    
    int pos = 0;
    for (const auto& spellValue : spellQueueData) {
        if (pos < static_cast<int>(spell_queue.getQueue().size())) {
            SpellType newSpell = static_cast<SpellType>(spellValue.get<int>());
            spell_queue.replaceSpell(pos, newSpell);
        }
        pos++;
    }

    const auto& timersData = jsonData["enemySpawnTimers"];

    for (auto& pair : enemySpawnTimers.asMap()) {
        const std::string& key = pair.first;
        float& value = pair.second;
        if (timersData.contains(key)) {
            value = timersData[key].get<float>();
        }
    }

    for (const auto& enemyData : jsonData["enemies"]) {
        auto type = static_cast<EnemyType>(enemyData["type"].get<int>());
        glm::vec2 position = { enemyData["motion"]["position"][0].get<float>(), 
                               enemyData["motion"]["position"][1].get<float>() };
        glm::vec2 velocity = { enemyData["motion"]["velocity"][0].get<float>(), 
                               enemyData["motion"]["velocity"][1].get<float>() };

        float cooldown = enemyData["cooldown"].get<float>();
        float health = std::stof(enemyData["health"].get<std::string>());

        Entity enemyEntity;
        switch (type) {
            case EnemyType::ARCHER:
                enemyEntity = EnemyFactory::createArcher(registry, position, velocity, cooldown, health);
                break;
            case EnemyType::KNIGHT:
                enemyEntity = EnemyFactory::createKnight(registry, position, velocity, cooldown, health);
                break;
            case EnemyType::PALADIN:
                enemyEntity = EnemyFactory::createPaladin(registry, position, velocity, cooldown, health);
                break;
            case EnemyType::DARKLORD:
                enemyEntity = EnemyFactory::createDarkLord(registry, position, velocity, cooldown, health);
                break;
            default:
                std::cerr << "Unknown enemy type: " << static_cast<int>(type) << std::endl;
                continue;
        }
    }
}

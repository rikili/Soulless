// GameAssets.hpp
#pragma once

#include "graphics/asset_manager.hpp"
#include <unordered_map>
#include <string>
#include <chrono>

// Structure to hold all asset IDs for easy access
struct GameAssets {
    std::unordered_map<std::string, AssetId> meshes;
    std::unordered_map<std::string, AssetId> textures;
    std::unordered_map<std::string, AssetId> shaders;
    std::unordered_map<std::string, AssetId> materials;
    std::unordered_map<std::string, AssetId> fonts;
};

// Function to initialize all game assets
GameAssets initializeGameAssets(AssetManager& assetManager);

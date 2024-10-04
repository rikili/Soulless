#include "utils/game_assets.hpp"

#include "core/common.hpp"

GameAssets initializeGameAssets(AssetManager& assetManager) {
    GameAssets assets;

    // Basic shader -> white color
    assets.shaders["basic"] = assetManager.loadShader("basic", shader_path("basic") + ".vs.glsl", shader_path("basic") + ".fs.glsl");

    std::vector<float> vertices = {
        // positions        // texture coords
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
    };
    std::vector<uint32_t> indices = {
        0, 1, 2,
        2, 3, 0
    };
    AssetId boxMesh = assetManager.loadMesh("basic", vertices, indices);

    return assets;
}

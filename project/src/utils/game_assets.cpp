#include "utils/game_assets.hpp"

#include "core/common.hpp"

GameAssets initializeGameAssets(AssetManager& assetManager) {
    GameAssets assets;
    assets.shaders["basic"] = assetManager.loadShader("basic", shader_path("basic") + ".vs.glsl", shader_path("basic") + ".fs.glsl");
    assets.shaders["background"] = assetManager.loadShader("background", shader_path("background") + ".vs.glsl", shader_path("background") + ".fs.glsl");
    assets.shaders["sprite"] = assetManager.loadShader("sprite", shader_path("sprite") + ".vs.glsl", shader_path("sprite") + ".fs.glsl");

    const std::vector<float> vertices = {
        // positions        // colors           // texture coords
        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f
    };

    const std::vector<uint32_t> indices = {
        0, 1, 2,
        2, 3, 0
    };

    const std::vector<VertexAttribute> attributes = {
        {3, GL_FLOAT, GL_FALSE, "position"},
        {3, GL_FLOAT, GL_FALSE, "color"},
        {2, GL_FLOAT, GL_FALSE, "texCoord"}
    };
    AssetId meshId = assetManager.loadMesh("basic", vertices, indices, attributes);

    const std::vector<float> spriteVertices = {
        // positions        // texture coords
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f
    };

    const std::vector<VertexAttribute> spriteAttributes = {
        {3, GL_FLOAT, GL_FALSE, "position"},
        {2, GL_FLOAT, GL_FALSE, "texCoord"}
    };

    AssetId spriteMeshId = assetManager.loadMesh("sprite", spriteVertices, indices, spriteAttributes);

    AssetId mageTextureId = assetManager.loadTexture("mage", textures_path("mage") + ".png");
    assets.textures["mage"] = mageTextureId;

    AssetId fireballTextureId = assetManager.loadTexture("fireball", textures_path("fireball") + ".png");
    assets.textures["fireball"] = fireballTextureId;

    // Add a new mesh for the background (full screen quad)
    const std::vector<float> bgVertices = {
        // positions        // texture coords
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f
    };

    const std::vector<VertexAttribute> bgAttributes = {
        {3, GL_FLOAT, GL_FALSE, "position"},
        {2, GL_FLOAT, GL_FALSE, "texCoord"}
    };
    AssetId bgMeshId = assetManager.loadMesh("background", bgVertices, indices, bgAttributes);

    AssetId grassTextureId  = assetManager.loadBackgroundTexture("grass", textures_path("grass") + ".jpg");
    assets.textures["grass"] = grassTextureId;

    // Set texture parameters for repeating
    const Texture* grassTexture = assetManager.getTexture(grassTextureId);
    if (grassTexture) {
        glBindTexture(GL_TEXTURE_2D, grassTexture->handle);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    return assets;
}
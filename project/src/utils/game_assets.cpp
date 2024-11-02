#include "utils/game_assets.hpp"

#include "core/common.hpp"

GameAssets initializeGameAssets(AssetManager& assetManager) {
    GameAssets assets;
    assets.shaders["basic"] = assetManager.loadShader("basic", shader_path("basic") + ".vs.glsl", shader_path("basic") + ".fs.glsl");
    assets.shaders["background"] = assetManager.loadShader("background", shader_path("background") + ".vs.glsl", shader_path("background") + ".fs.glsl");
    assets.shaders["sprite"] = assetManager.loadShader("sprite", shader_path("sprite") + ".vs.glsl", shader_path("sprite") + ".fs.glsl");
    assets.shaders["animatedsprite"] = assetManager.loadShader("animatedsprite", shader_path("animatedsprite") + ".vs.glsl", shader_path("animatedsprite") + ".fs.glsl");
    assets.shaders["font"] = assetManager.loadShader("font", shader_path("font") + ".vs.glsl", shader_path("font") + ".fs.glsl");

    // fonts
    AssetId deutschFont = assetManager.loadFont("deutsch", font_path("deutsch") + ".ttf", 48.0f);
    assets.fonts["deutsch"] = deutschFont;

    AssetId kingFont = assetManager.loadFont("king", font_path("kingthings") + ".ttf", 148.0f);
    assets.fonts["king"] = kingFont;

    AssetId healthFont = assetManager.loadFont("healthFont", font_path("deutsch") + ".ttf", 20.0f);
    assets.fonts["healthFont"] = healthFont;

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

    // Player
    AssetId mageTextureId = assetManager.loadTexture("mage", textures_path("Idle-sheet") + ".png");
    assets.textures["mage"] = mageTextureId;

    // Enemies
    assets.textures["archer"] = assetManager.loadTexture("archer", textures_path("archer") + ".png");

    AssetId farmerTextureId = assetManager.loadTexture("farmer", textures_path("farmer") + ".png");
    assets.textures["farmer"] = farmerTextureId;

    // Projectiles
    AssetId fireballTextureId = assetManager.loadTexture("fireball", textures_path("fireball") + ".png");
    assets.textures["fireball"] = fireballTextureId;

    AssetId pitchforkTextureId = assetManager.loadTexture("pitchfork", textures_path("pitchfork") + ".png");
    assets.textures["pitchfork"] = pitchforkTextureId;

    // Background Objects
    AssetId treeTextureId = assetManager.loadTexture("tree", textures_path("tree1") + ".png");
    AssetId campfireTextureId = assetManager.loadTexture("campfire", textures_path("campfire-sheet") + ".png");
    assets.textures["campfire"] = campfireTextureId;
    assets.textures["tree"] = treeTextureId;

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

    AssetId grassTextureId  = assetManager.loadBackgroundTexture("grass1", textures_path("grass1") + ".png");
    assets.textures["grass1"] = grassTextureId;

    AssetId grassTextureId2  = assetManager.loadBackgroundTexture("grass2", textures_path("grass2") + ".png");
    assets.textures["grass2"] = grassTextureId2;

    AssetId grassTextureIdDark  = assetManager.loadBackgroundTexture("grass3", textures_path("grass3") + ".png");
    assets.textures["grass3"] = grassTextureIdDark;

    // 4
    AssetId grassTextureId4  = assetManager.loadBackgroundTexture("grass4", textures_path("grass4") + ".png");
    assets.textures["grass4"] = grassTextureId4;

    // 5
    AssetId grassTextureId5  = assetManager.loadBackgroundTexture("grass5", textures_path("grass5") + ".png");
    assets.textures["grass5"] = grassTextureId5;


    // Set texture parameters for repeating
    // const Texture* grassTexture = assetManager.getTexture(grassTextureId);
    // if (grassTexture) {
    //     glBindTexture(GL_TEXTURE_2D, grassTexture->handle);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //     glBindTexture(GL_TEXTURE_2D, 0);
    // }



    return assets;
}




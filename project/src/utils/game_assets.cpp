#include "utils/game_assets.hpp"

#include "core/common.hpp"

GameAssets initializeGameAssets(AssetManager& assetManager)
{
    GameAssets assets;
    assets.shaders["basic"] = assetManager.loadShader("basic", shader_path("basic") + ".vs.glsl", shader_path("basic") + ".fs.glsl");
    assets.shaders["debug"] = assetManager.loadShader("debug", shader_path("debug") + ".vs.glsl", shader_path("debug") + ".fs.glsl");
    assets.shaders["background"] = assetManager.loadShader("background", shader_path("background") + ".vs.glsl", shader_path("background") + ".fs.glsl");
    assets.shaders["sprite"] = assetManager.loadShader("sprite", shader_path("sprite") + ".vs.glsl", shader_path("sprite") + ".fs.glsl");
    assets.shaders["animatedsprite"] = assetManager.loadShader("animatedsprite", shader_path("animatedsprite") + ".vs.glsl", shader_path("animatedsprite") + ".fs.glsl");
    assets.shaders["healthbar"] = assetManager.loadShader("healthbar", shader_path("healthbar") + ".vs.glsl", shader_path("healthbar") + ".fs.glsl");
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
        0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f };

    const std::vector<uint32_t> indices = {
        0, 1, 2,
        2, 3, 0 };

    const std::vector<VertexAttribute> attributes = {
        {3, GL_FLOAT, GL_FALSE, "position"},
        {3, GL_FLOAT, GL_FALSE, "color"},
        {2, GL_FLOAT, GL_FALSE, "texCoord"} };
    AssetId meshId = assetManager.loadMesh("basic", vertices, indices, attributes);

    const std::vector<float> spriteVertices = {
        // positions        // texture coords
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f };

    const std::vector<VertexAttribute> spriteAttributes = {
        {3, GL_FLOAT, GL_FALSE, "position"},
        {2, GL_FLOAT, GL_FALSE, "texCoord"} };

    AssetId spriteMeshId = assetManager.loadMesh("sprite", spriteVertices, indices, spriteAttributes);

    const std::vector<float> mageCollisionVertices = {
        0.f, 1.f, 0.f,
        -.5333333f, 0.2666667f, 0.f,
        .9333333f, .0666667f, 0.f,
        -.4f, -0.333333f, 0.f,
        -0.5333333f, -1.f, 0.f,
        0.4666667f, -1.f, 0.f,
        0.7333333f, -1.f, 0.f,
        0.4f, -0.2f, 0.f,
        0.333333f, 0.8f, 0.f,
        0.9333333f, 0.9333333f, 0.f
    };

    const std::vector<uint32_t> mage_col_indices = {
        0, 1, 8,
        1, 2, 3,
        3, 4, 5,
        3, 5, 7,
        1, 3, 7,
        1, 7, 8,
        7, 8, 9,
        6, 7, 9
    };

    const std::vector<VertexAttribute> mageCollisionAttributes = {
        {3, GL_FLOAT, GL_FALSE, "position"}
    };

    AssetId mageCollisionMeshId = assetManager.loadMesh("mage_collider", mageCollisionVertices, mage_col_indices, mageCollisionAttributes);

    const std::vector<float> debug_vertices = {
        // positions        // colors
        -1.f, -1.f, 0.0f, 
         1.f, -1.f, 0.0f,
         1.f,  1.f, 0.0f, 
        -1.f,  1.f, 0.0f,
    };

    const std::vector<uint32_t> debug_indices = {
        0, 1, 2,
        2, 3, 0
    };

    const std::vector<VertexAttribute> debug_attributes = {
        {3, GL_FLOAT, GL_FALSE, "position"}
    };
    AssetId debugId = assetManager.loadMesh("debug", debug_vertices, debug_indices, debug_attributes);

    // Player
    AssetId mageTextureId = assetManager.loadTexture("mage", textures_path("Idle-sheet") + ".png");
    assets.textures["mage"] = mageTextureId;


    // Enemies
    AssetId farmerTextureId = assetManager.loadTexture("farmer", textures_path("farmer") + ".png");
    assets.textures["farmer"] = farmerTextureId;

    AssetId archerTextureId = assetManager.loadTexture("archer", textures_path("archer") + ".png");
    assets.textures["archer"] = archerTextureId;

    AssetId knightTextureId = assetManager.loadTexture("knight", textures_path("knight") + ".png");
    assets.textures["knight"] = knightTextureId;

    // Projectiles
    AssetId fireballTextureId = assetManager.loadTexture("fireball", textures_path("fireball") + ".png");
    assets.textures["fireball"] = fireballTextureId;

    AssetId pitchforkTextureId = assetManager.loadTexture("pitchfork", textures_path("pitchfork") + ".png");
    assets.textures["pitchfork"] = pitchforkTextureId;

    AssetId arrowTextureId = assetManager.loadTexture("arrow", textures_path("arrow") + ".png");
    assets.textures["arrow"] = arrowTextureId;

    // Used for knight's MELEE sword slash
    AssetId fillerTextureId = assetManager.loadTexture("filler", textures_path("filler") + ".png");
    assets.textures["filler"] = fillerTextureId;

    // Background Objects
    AssetId treeTextureId = assetManager.loadTexture("tree", textures_path("tree1") + ".png");
    AssetId campfireTextureId = assetManager.loadTexture("campfire", textures_path("campfire-sheet") + ".png");
    assets.textures["campfire"] = campfireTextureId;
    assets.textures["tree"] = treeTextureId;

    // UI
    AssetId healthBarId = assetManager.loadTexture("healthbar", textures_path("health") + ".png");
    assets.textures["healthbar"] = healthBarId;

    // Add a new mesh for the background (full screen quad)
    const std::vector<float> bgVertices = {
        // positions        // texture coords
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f };

    const std::vector<VertexAttribute> bgAttributes = {
        {3, GL_FLOAT, GL_FALSE, "position"},
        {2, GL_FLOAT, GL_FALSE, "texCoord"} };
    AssetId bgMeshId = assetManager.loadMesh("background", bgVertices, indices, bgAttributes);

    AssetId grassTextureId = assetManager.loadBackgroundTexture("grass", textures_path("grass") + ".jpg");
    assets.textures["grass"] = grassTextureId;

    // Set texture parameters for repeating
    const Texture* grassTexture = assetManager.getTexture(grassTextureId);
    if (grassTexture)
    {
        glBindTexture(GL_TEXTURE_2D, grassTexture->handle);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    return assets;
}

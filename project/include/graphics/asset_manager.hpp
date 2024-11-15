#pragma once

#include <glcorearb.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <memory>

#include <map>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "isystems/IAssetManager.hpp"

/**
 * AssetManager is a class that manages the assets of the game.
 * It is responsible for loading and storing textures and meshes and music.
 * It only loads the assets once and then stores them in memory.
 */
class AssetManager: public IAssetManager {
public:
    AssetManager();
    ~AssetManager();

    AssetId loadMesh(const std::string& name, const std::vector<float>& vertices, const std::vector<uint32_t>& indices, const std::vector<VertexAttribute>& attributes);
    AssetId loadTexture(const std::string& name, const std::string& path);
    AssetId loadBackgroundTexture(const std::string& name, const std::string& path);
    AssetId loadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);
    AssetId createMaterial(const std::string& name, const AssetId& shader, const AssetId& texture = "");
    AssetId loadFont(const std::string& name, const std::string& path, float size);
    Shader* getShader(const AssetId& name);
    Mesh* getMesh(const AssetId& name);
    Texture* getTexture(const AssetId& name);
    Font* getFont(const AssetId& name);

private:
    std::unordered_map<AssetId, std::shared_ptr<Mesh>> meshes;
    std::unordered_map<AssetId, std::shared_ptr<Texture>> textures;
    std::unordered_map<AssetId, std::shared_ptr<Shader>> shaders;
    std::unordered_map<AssetId, std::shared_ptr<Material>> materials;
    std::unordered_map<AssetId, std::shared_ptr<Font>> fonts;
};


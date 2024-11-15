// IAssetManager.hpp
#pragma once
#include "forward_types.hpp"
#include <vector>
#include <cstdint>

#include <map>


class IAssetManager {
public:
    virtual ~IAssetManager() = default;

    // Loading assets
    virtual AssetId loadMesh(
        const std::string& name,
        const std::vector<float>& vertices,
        const std::vector<uint32_t>& indices,
        const std::vector<VertexAttribute>& attributes) = 0;

    virtual AssetId loadTexture(
        const std::string& name,
        const std::string& path) = 0;

    virtual AssetId loadBackgroundTexture(
        const std::string& name,
        const std::string& path) = 0;

    virtual AssetId loadShader(
        const std::string& name,
        const std::string& vertexPath,
        const std::string& fragmentPath) = 0;

    virtual AssetId createMaterial(
        const std::string& name,
        const AssetId& shader,
        const AssetId& texture = "") = 0;

    virtual AssetId loadFont(
        const std::string& name,
        const std::string& path,
        float size) = 0;

    // Getting assets
    virtual Shader* getShader(const AssetId& name) = 0;
    virtual Mesh* getMesh(const AssetId& name) = 0;
    virtual Texture* getTexture(const AssetId& name) = 0;
    virtual Font* getFont(const AssetId& name) = 0;
};
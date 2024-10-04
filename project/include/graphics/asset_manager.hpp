#pragma once

#include <glcorearb.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

using AssetId = std::string;  // Using strings for more flexible identification

struct Mesh {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    size_t vertexCount = 0;
    size_t indexCount = 0;
    std::vector<float> vertices;
    std::vector<uint32_t> indices;
};
struct Texture {
    GLuint handle = 0;
    glm::ivec2 dimensions{0, 0};
};

struct Shader {
    GLuint program = 0;
    std::string vertexPath;
    std::string fragmentPath;
};

struct Material {
    AssetId shader;
    AssetId texture;
    glm::vec4 color{1.0f};
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

class Box {
public:
    Transform2D transform;
    glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);  // White by default

    Box(const glm::vec2& position, const glm::vec2& size, float rotation = 0.0f)
        : transform{position, rotation, size} {}
};


/**
 * AssetManager is a class that manages the assets of the game.
 * It is responsible for loading and storing textures and meshes and music.
 * It only loads the assets once and then stores them in memory.
 */
class AssetManager {
public:
    AssetManager();
    ~AssetManager();

    AssetId loadMesh(const std::string& name, const std::vector<float>& vertices, const std::vector<uint32_t>& indices = std::vector<uint32_t>());
    AssetId loadTexture(const std::string& name, const std::string& path);
    AssetId loadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);
    AssetId createMaterial(const std::string& name, const AssetId& shader, const AssetId& texture = "");
    Shader* getShader(const AssetId& name);
    Mesh* getMesh(const AssetId& name);

private:
    std::unordered_map<AssetId, std::shared_ptr<Mesh>> meshes;
    std::unordered_map<AssetId, std::shared_ptr<Texture>> textures;
    std::unordered_map<AssetId, std::shared_ptr<Shader>> shaders;
    std::unordered_map<AssetId, std::shared_ptr<Material>> materials;
};


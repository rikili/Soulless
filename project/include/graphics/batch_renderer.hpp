#pragma once

#include <vector>
#include <map>
#include <string>
#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>

#include "core/common.hpp"
#include "isystems/IRenderSystem.hpp"
#include "isystems/ISubRenderer.hpp"
#include "isystems/IAssetManager.hpp"

#include "entities/ecs_registry.hpp"

struct BatchVertex {
    glm::vec3 position;
    glm::vec2 texCoord;
};

class BatchRenderer : public ISubRenderer {
private:
    struct Batch {
        GLuint VAO = 0;
        GLuint VBO = 0;
        std::vector<BatchVertex> vertices;
        std::string texture;
        size_t vertexCount = 0;
    };

    struct TileData {
        glm::vec2 position;
        glm::vec2 scale;
        std::string texture;
    };

    std::map<std::string, Batch> batches;
    static const size_t MAX_BATCH_VERTICES = 20000;
    std::vector<TileData> permanentTiles;

    void createBatchBuffers(Batch& batch);
    bool validateShaderProgram(GLuint program);
    void cleanup();
    void addTileToBatch(const glm::vec2& position, const glm::vec2& scale, const std::string& texture);
    bool checkUniformLocation(GLint location, const char* uniformName);

public:
    BatchRenderer();
    virtual ~BatchRenderer() override;
    
    void addTile(const glm::vec2& position, const glm::vec2& scale, const std::string& texture);
    void clearAllTiles();
    virtual void render(IRenderSystem* renderer) override;
};

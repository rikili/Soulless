#include "graphics/batch_renderer.hpp"
#include <iostream>

BatchRenderer::BatchRenderer() {}

BatchRenderer::~BatchRenderer() {
    cleanup();
}

void BatchRenderer::cleanup() {
    for (auto& pair : batches) {
        if (pair.second.VAO != 0) {
            glDeleteVertexArrays(1, &pair.second.VAO);
        }
        if (pair.second.VBO != 0) {
            glDeleteBuffers(1, &pair.second.VBO);
        }
    }
    batches.clear();
    permanentTiles.clear();
}

void BatchRenderer::clearAllTiles() {
    permanentTiles.clear();
    for (auto& pair : batches) {
        pair.second.vertices.clear();
        pair.second.vertexCount = 0;
    }
}

bool BatchRenderer::validateShaderProgram(GLuint program) {
    if (!program) {
        std::cerr << "Invalid shader program handle" << std::endl;
        return false;
    }
    return true; 
}


void BatchRenderer::createBatchBuffers(Batch& batch) {
    glGenVertexArrays(1, &batch.VAO);
    glGenBuffers(1, &batch.VBO);
    
    glBindVertexArray(batch.VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, batch.VBO);
    glBufferData(GL_ARRAY_BUFFER, 
                 MAX_BATCH_VERTICES * sizeof(BatchVertex), 
                 nullptr, 
                 GL_DYNAMIC_DRAW);
    

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 
                         sizeof(BatchVertex), 
                         (void*)offsetof(BatchVertex, position));
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 
                         sizeof(BatchVertex), 
                         (void*)offsetof(BatchVertex, texCoord));
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    if (batch.VAO == 0) {
        std::cerr << "Failed to create VAO" << std::endl;
    }
    
    gl_has_errors();
}

void BatchRenderer::addTileToBatch(const glm::vec2& position, 
                                 const glm::vec2& scale, 
                                 const std::string& texture) {
    Batch& batch = batches[texture];
    
    if (batch.vertices.empty()) {
        batch.texture = texture;
        createBatchBuffers(batch);
    }

    if (batch.vertices.size() + 6 > MAX_BATCH_VERTICES) {
    
        std::cerr << "Warning: Maximum batch size exceeded for texture: " 
                  << texture << " it has " << batch.vertices.size()<< " vertices" << std::endl;
        return;
    }

    float halfWidth = scale.x * 32.0f;
    float halfHeight = scale.y * 32.0f;

    // Create 6 vertices for 2 triangles
    BatchVertex vertices[6];
    
    // First triangle
    vertices[0].position = glm::vec3(position.x - halfWidth, position.y - halfHeight, 0.0f);
    vertices[0].texCoord = glm::vec2(0.0f, 0.0f);
    
    vertices[1].position = glm::vec3(position.x + halfWidth, position.y - halfHeight, 0.0f);
    vertices[1].texCoord = glm::vec2(1.0f, 0.0f);
    
    vertices[2].position = glm::vec3(position.x + halfWidth, position.y + halfHeight, 0.0f);
    vertices[2].texCoord = glm::vec2(1.0f, 1.0f);

    // Second 
    vertices[3].position = glm::vec3(position.x - halfWidth, position.y - halfHeight, 0.0f);
    vertices[3].texCoord = glm::vec2(0.0f, 0.0f);
    
    vertices[4].position = glm::vec3(position.x + halfWidth, position.y + halfHeight, 0.0f);
    vertices[4].texCoord = glm::vec2(1.0f, 1.0f);
    
    vertices[5].position = glm::vec3(position.x - halfWidth, position.y + halfHeight, 0.0f);
    vertices[5].texCoord = glm::vec2(0.0f, 1.0f);

    for (const auto& vertex : vertices) {
        batch.vertices.push_back(vertex);
    }
    batch.vertexCount = batch.vertices.size();
}

void BatchRenderer::addTile(const glm::vec2& position, 
                          const glm::vec2& scale, 
                          const std::string& texture) {
    permanentTiles.push_back({position, scale, texture});
    addTileToBatch(position, scale, texture);
}


bool BatchRenderer::checkUniformLocation(GLint location, const char* uniformName) {
    if (location == -1) {
        std::cerr << "Failed to find uniform location: " << uniformName << std::endl;
        return false;
    }
    return true;
}

void BatchRenderer::render(IRenderSystem* renderer) {
    GLint last_program;
    glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
    
    GLint last_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    
    GLint last_array_buffer;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
    
    GLint last_vertex_array;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

    GLint last_blend_src, last_blend_dst;
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &last_blend_src);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &last_blend_dst);
    GLboolean blend_enabled = glIsEnabled(GL_BLEND);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (auto& pair : batches) {
        pair.second.vertices.clear();
        pair.second.vertexCount = 0;
    }

    for (const auto& tile : permanentTiles) {
        addTileToBatch(tile.position, tile.scale, tile.texture);
    }

    gl_has_errors();

    const glm::mat4& projectionMatrix = renderer->getProjectionMatrix();
    const glm::mat4& viewMatrix = renderer->getViewMatrix();
    
    glm::mat4 transformMatrix = glm::mat4(1.0f); 

    // Render all batches
    for (auto& pair : batches) {
        Batch& batch = pair.second;
        if (batch.vertices.empty()) continue;

        IAssetManager& asset_manager = renderer->getAssetManager();
        const Shader* shader = asset_manager.getShader("sprite");
        if (!validateShaderProgram(shader->program)) {
            continue;
        }

        glUseProgram(shader->program);
        gl_has_errors();

        glBindVertexArray(batch.VAO);
        gl_has_errors();

        GLint projectionLoc = glGetUniformLocation(shader->program, "projection");
        if (checkUniformLocation(projectionLoc, "projection")) {
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, 
                             glm::value_ptr(projectionMatrix));
        }

        GLint viewLoc = glGetUniformLocation(shader->program, "view");
        if (checkUniformLocation(viewLoc, "view")) {
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, 
                             glm::value_ptr(viewMatrix));
        }

        GLint transformLoc = glGetUniformLocation(shader->program, "transform");
        if (checkUniformLocation(transformLoc, "transform")) {
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, 
                             glm::value_ptr(transformMatrix));
        }

        const Texture* texture = asset_manager.getTexture(batch.texture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture->handle);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        GLint textureLoc = glGetUniformLocation(shader->program, "image");
        if (checkUniformLocation(textureLoc, "image")) {
            glUniform1i(textureLoc, 0);
        }

        gl_has_errors();

        glBindBuffer(GL_ARRAY_BUFFER, batch.VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
                       batch.vertices.size() * sizeof(BatchVertex),
                       batch.vertices.data());

        gl_has_errors();

        glDrawArrays(GL_TRIANGLES, 0, batch.vertexCount);
        gl_has_errors();

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);

        gl_has_errors();
    }

    // Restore previous OpenGL state
    glUseProgram(last_program);
    glBindTexture(GL_TEXTURE_2D, last_texture);
    glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
    glBindVertexArray(last_vertex_array);
    
    // Restore blend state
    if (!blend_enabled)
        glDisable(GL_BLEND);
    glBlendFunc(last_blend_src, last_blend_dst);
}
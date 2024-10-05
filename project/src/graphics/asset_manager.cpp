// AssetManager.cpp
#include "graphics/asset_manager.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#define STB_IMAGE_IMPLEMENTATION
#include <gl3w.h>

#include "stb_image.h"

AssetManager::AssetManager()
    : meshes(), textures(), shaders(), materials()
{
}

AssetManager::~AssetManager() {
    // Clean up OpenGL resources
    for (auto& pair : meshes) {
        glDeleteVertexArrays(1, &pair.second->vao);
        glDeleteBuffers(1, &pair.second->vbo);
        if (pair.second->ebo != 0) {
            glDeleteBuffers(1, &pair.second->ebo);
        }
    }
    for (const auto& pair : textures) {
        glDeleteTextures(1, &pair.second->handle);
    }
    for (auto& pair : shaders) {
        glDeleteProgram(pair.second->program);
    }
}

AssetId AssetManager::loadMesh(const std::string& name, const std::vector<float>& vertices, const std::vector<uint32_t>& indices, const std::vector<VertexAttribute>& attributes) {
    auto mesh = std::make_shared<Mesh>();

    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);

    glBindVertexArray(mesh->vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    GLsizei stride = 0;
    for (const auto& attr : attributes) {
        stride += attr.size * sizeof(float);
    }

    GLuint offset = 0;
    for (GLuint i = 0; i < attributes.size(); ++i) {
        const auto& attr = attributes[i];
        glVertexAttribPointer(i, attr.size, attr.type, attr.normalized, stride, (void*)(offset * sizeof(float)));
        glEnableVertexAttribArray(i);
        offset += attr.size;
    }

    if (!indices.empty()) {
        glGenBuffers(1, &mesh->ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
        mesh->indexCount = indices.size();
    }

    mesh->vertexCount = vertices.size() / (stride / sizeof(float));
    mesh->vertices = vertices;
    mesh->indices = indices;
    mesh->attributes = attributes;

    meshes[name] = std::move(mesh);
    return name;
}


AssetId AssetManager::loadTexture(const std::string& name, const std::string& path) {
    auto texture = std::make_shared<Texture>();

    int width, height, channels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (data) {
        glGenTextures(1, &texture->handle);
        glBindTexture(GL_TEXTURE_2D, texture->handle);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);

        texture->dimensions = glm::ivec2(width, height);
        textures[name] = std::move(texture);
        return name;
    } else {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return "";
    }
}

AssetId AssetManager::loadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) {
    auto shader = std::make_shared<Shader>();
    shader->vertexPath = vertexPath;
    shader->fragmentPath = fragmentPath;

    // Read vertex shader
    std::string vertexCode;
    std::ifstream vShaderFile(vertexPath);
    if (vShaderFile.is_open()) {
        std::stringstream vShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        vertexCode = vShaderStream.str();
        vShaderFile.close();
    } else {
        std::cerr << "Failed to open vertex shader file: " << vertexPath << std::endl;
        return "";
    }

    // Read fragment shader
    std::string fragmentCode;
    std::ifstream fShaderFile(fragmentPath);
    if (fShaderFile.is_open()) {
        std::stringstream fShaderStream;
        fShaderStream << fShaderFile.rdbuf();
        fragmentCode = fShaderStream.str();
        fShaderFile.close();
    } else {
        std::cerr << "Failed to open fragment shader file: " << fragmentPath << std::endl;
        return "";
    }

    // Compile shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vShaderCode = vertexCode.c_str();
    glShaderSource(vertexShader, 1, &vShaderCode, NULL);
    glCompileShader(vertexShader);
    // Check for compile errors...

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fShaderCode = fragmentCode.c_str();
    glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
    glCompileShader(fragmentShader);
    // Check for compile errors...

    // Link shaders
    shader->program = glCreateProgram();
    glAttachShader(shader->program, vertexShader);
    glAttachShader(shader->program, fragmentShader);
    glLinkProgram(shader->program);
    // Check for linking errors...

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    shaders[name] = std::move(shader);
    return name;
}

AssetId AssetManager::createMaterial(const std::string& name, const AssetId& shader, const AssetId& texture) {
    auto material = std::make_shared<Material>();
    material->shader = shader;
    material->texture = texture;

    materials[name] = std::move(material);
    return name;
}

Shader* AssetManager::getShader(const AssetId& name)  {
    const auto it = shaders.find(name);
    if (it != shaders.end()) {
        return it->second.get();
    }
    return nullptr;
}

 Mesh* AssetManager::getMesh(const AssetId& name)  {
     const auto it = meshes.find(name);
    if (it != meshes.end()) {
        return it->second.get();
    }
    return nullptr;
}


Texture* AssetManager::getTexture(const AssetId& name)  {
    auto it = textures.find(name);
    if (it != textures.end()) {
        return it->second.get();
    }
    return nullptr;
}

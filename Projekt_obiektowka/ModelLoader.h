#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <glad/glad.h>
#include <assimp/scene.h>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    GLuint VAO, VBO, EBO;
};

struct Node {
    glm::mat4 transform;
    std::vector<unsigned int> meshIndices;
    std::vector<Node> children;
};

// globalne kontenery
extern std::vector<Mesh> meshes;
extern Node rootNode;

// ³adowanie modelu z pliku
bool loadModel(const std::string& path);

// rysowanie ca³ego drzewa sceny
void drawNode(const Node& node, const glm::mat4& parentTransform, GLuint shaderProgram);

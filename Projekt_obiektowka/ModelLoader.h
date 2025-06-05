#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <glad/glad.h>
#include <assimp/scene.h>

// Struktura pojedynczego wierzcho³ka (pozycja i normalna)
struct Vertex {
    glm::vec3 position; // Pozycja wierzcho³ka
    glm::vec3 normal;   // Wektor normalny wierzcho³ka
	glm::vec2 texCoord; // Wspó³rzêdne tekstury (jeœli s¹ dostêpne, np. w modelach z teksturami)
};

// Struktura siatki (mesh) - przechowuje wierzcho³ki, indeksy i identyfikatory OpenGL
struct Mesh {
    std::vector<Vertex> vertices;      // Wierzcho³ki siatki
    std::vector<unsigned int> indices; // Indeksy do rysowania elementów
    GLuint VAO, VBO, EBO;              // Identyfikatory OpenGL: Vertex Array, Vertex Buffer, Element Buffer
	GLuint textureID = 0;              // Identyfikator tekstury (jeœli jest u¿ywana)
};

// Struktura wêz³a drzewa sceny (hierarchia modelu)
struct Node {
    glm::mat4 transform;                   // Macierz transformacji wêz³a
    std::vector<unsigned int> meshIndices; // Indeksy siatek przypisanych do tego wêz³a
    std::vector<Node> children;            // Dzieci (podwêz³y) w hierarchii
};

// Globalne kontenery na siatki i korzeñ drzewa sceny
extern std::vector<Mesh> meshes; // Wszystkie siatki modelu
extern Node rootNode;            // Korzeñ drzewa sceny

// £aduje model z pliku (np. .gltf, .obj) i buduje strukturê wêz³ów oraz siatek
bool LoadModel(const std::string& path);

// £adowanie tekstury z pliku przy u¿yciu stb_image
GLuint LoadTexture(const std::string& path);
GLuint LoadTextureFromMemory(const unsigned char* data, int size);

// Indeksy wêz³ów, które maj¹ byæ obracane oraz aktualny k¹t obrotu (do animacji)
extern std::vector<int> rotatingNodeIndices;
extern float rotationAngle;




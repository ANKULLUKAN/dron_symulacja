#include "floor.h"

// Wierzcho³ki pod³ogi
static float floorVertices[] = {
    -5.0f, 0.0f, -5.0f,
     5.0f, 0.0f, -5.0f,
     5.0f, 0.0f,  5.0f,
     5.0f, 0.0f,  5.0f,
    -5.0f, 0.0f,  5.0f,
    -5.0f, 0.0f, -5.0f,
};

// Konstruktor i destruktor klasy Floor
Floor::Floor() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);   
}

Floor::~Floor() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

// Funkcja rysuj¹ca pod³ogê
void Floor::Draw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection) {
    shader.Use();
    shader.SetMat4("view", view);
    shader.SetMat4("projection", projection);

    glBindVertexArray(VAO);

    float size = 10.0f; // rozmiar ca³ej pod³ogi
    float tile = size / gridSize;

    for (int x = 0; x < gridSize; ++x) {
        for (int z = 0; z < gridSize; ++z) {
            // Kolor szachownicy
            bool isBlack = (x + z) % 2 == 0;
            glm::vec4 color = isBlack ? glm::vec4(0.1f, 0.1f, 0.1f, 1.0f) : glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
            shader.SetVec4("objectColor", color);

            float x0 = -size / 2 + x * tile;
            float x1 = x0 + tile;
            float z0 = -size / 2 + z * tile;
            float z1 = z0 + tile;

            float quad[] = {
                x0, 0.0f, z0,
                x1, 0.0f, z0,
                x1, 0.0f, z1,
                x1, 0.0f, z1,
                x0, 0.0f, z1,
                x0, 0.0f, z0,
            };

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_DYNAMIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
            glEnableVertexAttribArray(0);

            glm::mat4 model = glm::mat4(1.0f);
            shader.SetMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }

    glBindVertexArray(0);
}
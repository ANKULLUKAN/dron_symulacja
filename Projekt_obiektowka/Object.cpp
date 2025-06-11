#include "Object.h"
#include <glm/gtc/matrix_transform.hpp>

Cube::Cube(float size, float mass)
    : position(0.0f, size / 2.0f, 0.0f), velocity(0.0f), mass(mass)
{
    setupMesh(size);
}

bool Cube::checkContactWithDrone(const glm::vec3 dronePosition) {
	const float distance = glm::length(position - dronePosition);
    constexpr float epsilon = 0.1f; // granica b³êdu (promieñ kontaktu)

    if (distance < epsilon) return true;
	return false;
}

Cube::~Cube() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void Cube::setupMesh(const float size) {
    float h = size / 2.0f;
    float vertices[] = {
        // Ty³
        -h, -h, -h,  h, -h, -h,  h,  h, -h,
         h,  h, -h, -h,  h, -h, -h, -h, -h,
         // Przód
         -h, -h,  h,  h, -h,  h,  h,  h,  h,
          h,  h,  h, -h,  h,  h, -h, -h,  h,
          // Lewa
          -h,  h,  h, -h,  h, -h, -h, -h, -h,
          -h, -h, -h, -h, -h,  h, -h,  h,  h,
          // Prawa
           h,  h,  h,  h,  h, -h,  h, -h, -h,
           h, -h, -h,  h, -h,  h,  h,  h,  h,
           // Dó³
           -h, -h, -h,  h, -h, -h,  h, -h,  h,
            h, -h,  h, -h, -h,  h, -h, -h, -h,
            // Góra
            -h,  h, -h,  h,  h, -h,  h,  h,  h,
             h,  h,  h, -h,  h,  h, -h,  h, -h
    };
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}


void Cube::Update(const float deltaTime, const glm::vec3 dronePosition, const bool contact) {

    if (contact) {
        position = dronePosition + glm::vec3(0.0f, -0.1f, 0.0f);
    }
    else {
        constexpr float gravity = -9.81f;
        velocity.y += gravity * deltaTime;

        if (position.y + velocity.y * deltaTime < 0.1f) {
            position.y = 0.1f;
            velocity.y = 0.0f;
        }
        else {
            position += velocity * deltaTime;
        }
    }

}

void Cube::Draw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection) const {
    shader.Use();
    shader.SetMat4("view", view);
    shader.SetMat4("projection", projection);
    shader.SetVec4("objectColor", glm::vec4(0.8f, 0.2f, 0.2f, 1.0f));
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    shader.SetMat4("model", model);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

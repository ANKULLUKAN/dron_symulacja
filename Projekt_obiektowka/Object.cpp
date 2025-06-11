#include "Object.h"
#include <glm/gtc/matrix_transform.hpp>

Cube::Cube(float size, float mass)
    :size(size), position(0.0f, size / 2.0f, 0.0f), velocity(0.0f), mass(mass)
{
    setupMesh(size);
}

bool Cube::checkContactWithDrone(const glm::vec3 dronePosition, bool& droneBroken) {
    if (attachedToDrone) {
        // Jeœli kostka jest podczepiona, nie sprawdzaj kolizji
        return false;
    }
	// sprawdzanie , czy dron jest w odpowiedniej pozycji do przyczepienia obiektu
	constexpr float epsilonXZ = 0.15f; // minimalna odleg³oœæ w p³aszczyŸnie XZ
	constexpr float minY = 0.1f; // minimalna odleg³oœæ w osi Y, aby dron móg³ siê przyczepiæ
	constexpr float maxY = 0.3f; //    maksymalna odleg³oœæ w osi Y, aby dron móg³ siê przyczepiæ
    float deltaY = dronePosition.y - position.y;
    bool canAttach = (deltaY > minY) && (deltaY < maxY) &&
        std::abs(dronePosition.x - position.x) < epsilonXZ &&
        std::abs(dronePosition.z - position.z) < epsilonXZ;

    // rozmiary drona do kolizji 
    float halfCube = size / 2.0f;
    float halfDroneX = 0.25f / 2.0f;
    float halfDroneY = 0.05f / 2.0f;
    float halfDroneZ = 0.25f / 2.0f;

    bool overlapX = std::abs(position.x - dronePosition.x) < (halfCube + halfDroneX);
    bool overlapY = std::abs(position.y - dronePosition.y) < (halfCube + halfDroneY);
    bool overlapZ = std::abs(position.z - dronePosition.z) < (halfCube + halfDroneZ);

    if (overlapX && overlapY && overlapZ) {
        droneBroken = true;
    }

    return canAttach;
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


void Cube::Update(const float deltaTime, const glm::vec3 dronePosition, const bool contact, const glm::vec3 velocity_of_drone) {
    if (contact) {
        attachedToDrone = true;
        
        glm::vec3 target = dronePosition + glm::vec3(0.0f, -0.2f, 0.0f);
        // Proste bujanie: swingOffset d¹¿y do zera (jakby lina siê prostowa³a)
        swingOffset *= 0.90f; // t³umienie bujania

        //efekt bujania na podstawie prêdkoœci drona
        swingOffset.x -= velocity_of_drone.x * 0.05f;
        swingOffset.z -= velocity_of_drone.z * 0.05f;

        // Ogranicz maksymalne wychylenie (d³ugoœæ liny)
        float maxSwing = 0.3f;
        if (glm::length(swingOffset) > maxSwing)
            swingOffset = glm::normalize(swingOffset) * maxSwing;

        position = target + swingOffset;
        velocity = velocity_of_drone;
    }
    else {
        constexpr float gravity = -0.81f; // ma³a sta³a grawitacji aby pokazac iz predkosc kostki jest dziedziczona
        velocity.y += gravity * deltaTime;

        float minY = size / 2.0f;
        if (position.y + velocity.y * deltaTime < minY) {
            position.y = minY;
            velocity.x = 0.0f;
            velocity.y = 0.0f;
            velocity.z = 0.0f;
        }
        else {
            position += velocity * deltaTime;
        }
        attachedToDrone = false;
    }
}

void Cube::Draw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection, float /*tiltAngleX*/, float /*tiltAngleY*/) const {
    shader.Use();
    shader.SetMat4("view", view);
    shader.SetMat4("projection", projection);
    shader.SetVec4("objectColor", glm::vec4(0.8f, 0.2f, 0.2f, 1.0f));

    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);

    if (attachedToDrone) {
        glm::vec2 swing2D(swingOffset.x, swingOffset.z);
        float swingLen = glm::length(swing2D);
        if (swingLen > 0.001f) {
            glm::vec3 swingDir = glm::normalize(glm::vec3(swingOffset.x, 0.0f, swingOffset.z));
            // Maksymalny k¹t odchylenia, np. 35 stopni
            float maxSwing = 0.3f; // taki sam jak w Update
            float maxAngle = glm::radians(45.0f);
            float swingAngle = glm::clamp(swingLen / maxSwing, 0.0f, 1.0f) * maxAngle;
            glm::vec3 axis = glm::cross(swingDir, glm::vec3(0, 1, 0));
            model = glm::rotate(model, swingAngle, axis);
        }
    }

    shader.SetMat4("model", model);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

#include "Object.h"
#include <glm/gtc/matrix_transform.hpp>

// Funkcja zwracaj¹ca masê kostki
float Cube::getCubeMass() {
	return mass;
}

// Funkcja sprawdzaj¹ca kolizjê kostki z dronem
bool Cube::checkContactWithDrone(const glm::vec3 dronePosition, bool& droneBroken) {

	if (attachedToDrone) return false; // Jeœli kostka jest podczepiona, nie sprawdzaj kolizji
	
    // Sprawdzenie, czy dron jest w odpowiedniej pozycji do przyczepienia obiektu
	constexpr float epsilonXZ = 0.15f; // minimalna odleg³oœæ w p³aszczyŸnie XZ
	constexpr float minY = 0.1f; // minimalna odleg³oœæ w osi Y, aby dron móg³ siê przyczepiæ
	constexpr float maxY = 0.3f; // maksymalna odleg³oœæ w osi Y, aby dron móg³ siê przyczepiæ

	const float deltaY = dronePosition.y - position.y; // ró¿nica wysokoœci miêdzy dronem a kostk¹
    const bool canAttach = (deltaY > minY) && (deltaY < maxY) &&
        std::abs(dronePosition.x - position.x) < epsilonXZ &&
        std::abs(dronePosition.z - position.z) < epsilonXZ;

	// Sprawdzenie kolizji z kostk¹
    const float halfCube = size / 2.0f;
    constexpr float halfDroneX = 0.25f / 2.0f;
    constexpr float halfDroneY = 0.05f / 2.0f;
    constexpr float halfDroneZ = 0.25f / 2.0f;

    bool overlapX = std::abs(dronePosition.x - position.x) < (halfCube + halfDroneX);
    bool overlapY = std::abs(dronePosition.y - position.y) < (halfCube + halfDroneY);
    bool overlapZ = std::abs(dronePosition.z - position.z) < (halfCube + halfDroneZ);

	// Jeœli dron jest w odpowiedniej pozycji i nastêpuje kolizja, oznacz drona jako uszkodzony
    if (overlapX && overlapY && overlapZ) {
        droneBroken = true;
    }

    return canAttach;
}

// Funkcja ustawiaj¹ca siatkê kostki
void Cube::setupMesh(const float size) {
    float h = size / 2.0f;
    float vertices[] = {
        // Ty³ (0, 0, -1)
        -h, -h, -h,  0, 0, -1,
         h, -h, -h,  0, 0, -1,
         h,  h, -h,  0, 0, -1,
         h,  h, -h,  0, 0, -1,
        -h,  h, -h,  0, 0, -1,
        -h, -h, -h,  0, 0, -1,

        // Przód (0, 0, 1)
        -h, -h,  h,  0, 0, 1,
         h, -h,  h,  0, 0, 1,
         h,  h,  h,  0, 0, 1,
         h,  h,  h,  0, 0, 1,
        -h,  h,  h,  0, 0, 1,
        -h, -h,  h,  0, 0, 1,

        // Lewa (-1, 0, 0)
        -h,  h,  h, -1, 0, 0,
        -h,  h, -h, -1, 0, 0,
        -h, -h, -h, -1, 0, 0,
        -h, -h, -h, -1, 0, 0,
        -h, -h,  h, -1, 0, 0,
        -h,  h,  h, -1, 0, 0,

        // Prawa (1, 0, 0)
         h,  h,  h,  1, 0, 0,
         h,  h, -h,  1, 0, 0,
         h, -h, -h,  1, 0, 0,
         h, -h, -h,  1, 0, 0,
         h, -h,  h,  1, 0, 0,
         h,  h,  h,  1, 0, 0,

         // Dó³ (0, -1, 0)
         -h, -h, -h,  0, -1, 0,
          h, -h, -h,  0, -1, 0,
          h, -h,  h,  0, -1, 0,
          h, -h,  h,  0, -1, 0,
         -h, -h,  h,  0, -1, 0,
         -h, -h, -h,  0, -1, 0,

         // Góra (0, 1, 0)
         -h,  h, -h,  0, 1, 0,
          h,  h, -h,  0, 1, 0,
          h,  h,  h,  0, 1, 0,
          h,  h,  h,  0, 1, 0,
         -h,  h,  h,  0, 1, 0,
         -h,  h, -h,  0, 1, 0
    };
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), static_cast<void*>(nullptr)); // pozycja
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float))); // normalne
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

// Aktualizacja fizyki kostki
void Cube::Update(const float deltaTime, const glm::vec3 dronePosition, const bool contact, const glm::vec3 droneVelocity) {

	const float minY = size / 2.0f; // minimalna wysokoœæ kostki nad pod³og¹
    constexpr float gravity = -0.1f; // ma³a sta³a grawitacji aby pokazac iz predkosc kostki jest dziedziczona

	if (contact) {
        attachedToDrone = true;
        
        glm::vec3 target = dronePosition + glm::vec3(0.0f, -0.2f, 0.0f);
        // Proste bujanie: swingOffset d¹¿y do zera (jakby lina siê prostowa³a)
        swingOffset *= 0.90f; // t³umienie bujania

        //efekt bujania na podstawie prêdkoœci drona
        swingOffset.x -= droneVelocity.x * 0.05f;
        swingOffset.z -= droneVelocity.z * 0.05f;

        // Ogranicz maksymalne wychylenie (d³ugoœæ liny)
        float maxSwing = 0.3f;
        if (glm::length(swingOffset) > maxSwing)
            swingOffset = glm::normalize(swingOffset) * maxSwing;

		position = target + swingOffset; // aktualizacja pozycji kostki
		velocity = droneVelocity; // dziedziczenie prêdkoœci drona

		// Sprawdzenie, czy kostka nie spad³a poni¿ej minimalnej wysokoœci
        if (position.y + velocity.y * deltaTime < minY) {
            position.y = minY;
            velocity.x = 0.0f;
            velocity.y = 0.0f;
            velocity.z = 0.0f;
        }
    }
    else {
		velocity.y += gravity * deltaTime; // Dodanie grawitacji do prêdkoœci kostki

		// Sprawdzenie, czy kostka nie spad³a poni¿ej minimalnej wysokoœci
        if (position.y + velocity.y * deltaTime < minY) {
            position.y = minY;
            velocity.x = 0.0f;
            velocity.y = 0.0f;
            velocity.z = 0.0f;
        }
        else {
			position += velocity * deltaTime; // Aktualizacja pozycji kostki na podstawie prêdkoœci
        }
        attachedToDrone = false;
    }
}

// Funkcja rysuj¹ca kostkê
void Cube::Draw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3 lightDir, const glm::vec3 cameraPos) const {

	shader.Use();
    shader.SetMat4("view", view);
    shader.SetMat4("projection", projection);
    shader.SetVec4("objectColor", glm::vec4(0.8f, 0.2f, 0.2f, 1.0f));
    shader.SetVec3("lightDir", lightDir);
    shader.SetVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.SetVec3("viewPos", cameraPos);

    glm::mat4 cubeModel = glm::translate(glm::mat4(1.0f), position);

	// Jeœli kostka jest przyczepiona do drona, dodaj bujanie
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
            cubeModel = glm::rotate(cubeModel, swingAngle, axis);
        }
    }

    shader.SetMat4("model", cubeModel);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}
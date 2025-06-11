#include "Drone.h"
#include "Object.h"

#include <glm/gtc/matrix_transform.hpp>
#include "ModelLoader.h"
#include "rotation.h"

// Wektor wierzcho³ków drona
void Drone::drawDrone(const Shader& texturedShader, const glm::mat4& projection, const glm::mat4& view, const glm::vec3 lightDir) {

    // --- Model drona ---
    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), position);

    texturedShader.Use();
    texturedShader.SetMat4("view", view);
    texturedShader.SetMat4("projection", projection);
    texturedShader.SetMat4("model", modelMatrix);
    texturedShader.SetVec3("lightDir", lightDir);
    texturedShader.SetVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    texturedShader.SetVec3("viewPos", getCameraPos());
	int nodeCounter = 0;
    renderer.drawNodeWithRotation(rootNode, modelMatrix, texturedShader.Id, nodeCounter,
        controller.tilt.x, controller.tilt.y, controller.thrust_y);
}

// Funkcja rzutuj¹ca punkt na pod³ogê w kierunku œwiat³a
glm::vec3 Drone::projectToFloor(const glm::vec3& point, const glm::vec3& lightDir) {
    // Rzutuj punkt w kierunku œwiat³a na p³aszczyznê y=0
    float t = -point.y / lightDir.y;
    return point + t * lightDir;
}

// Funkcja generuj¹ca siatkê cienia drona
void Drone::generateDroneShadowMesh(const glm::mat4& modelMatrix, const glm::vec3& lightDir) {
    shadowVertices.clear();
    for (const auto& v : droneVertices) {
        // Przekszta³æ wierzcho³ek do przestrzeni œwiata
        glm::vec3 worldV = glm::vec3(modelMatrix * glm::vec4(v, 1.0f));
        // Rzutuj na pod³ogê
        glm::vec3 shadowV = projectToFloor(worldV, lightDir);
        shadowVertices.push_back(shadowV);
    }
    // Indeksy cienia = indeksy modelu
    shadowIndices = droneIndices;
}

// Funkcja rysuj¹ca cieñ drona
void Drone::drawDroneShadow(const Shader& shader, const glm::mat4& projection, const glm::mat4& view, const glm::vec3& lightDir) {
    // Macierz modelu drona (pozycja, rotacja, skala)
    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), position);
    // Wygeneruj siatkê cienia
    generateDroneShadowMesh(modelMatrix, lightDir);

    // Wyznacz œrodek cienia
    glm::vec3 center = glm::vec3(position.x, 0.0f, position.z);


    // Przeskaluj cieñ wzglêdem œrodka
    float shadowScale = 0.1f; // np. 60% oryginalnego rozmiaru
    for (auto& v : shadowVertices) {
        v = center + shadowScale * (v - center);
    }

    // Zamieñ shadowVertices na tablicê floatów
    std::vector<float> shadowVerticesFlat;
    for (const auto& v : shadowVertices) {
        shadowVerticesFlat.push_back(v.x);
        shadowVerticesFlat.push_back(v.y + 0.01f); // lekko nad pod³og¹, by nie migota³o
        shadowVerticesFlat.push_back(v.z);
    }

    // Tworzenie VAO/VBO/IBO
    GLuint vao, vbo, ibo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, shadowVerticesFlat.size() * sizeof(float), shadowVerticesFlat.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr));
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, shadowIndices.size() * sizeof(unsigned int), shadowIndices.data(), GL_STATIC_DRAW);

    // Ustawienia shaderów
    shader.Use();
    shader.SetMat4("view", view);
    shader.SetMat4("projection", projection);
    shader.SetMat4("model", glm::mat4(1.0f)); // cieñ ju¿ jest w world space
    shader.SetVec4("objectColor", glm::vec4(0, 0, 0, 0.35f)); // pó³przezroczysty cieñ

    // Rysowanie cienia
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(shadowIndices.size()), GL_UNSIGNED_INT, nullptr);

    // Sprz¹tanie
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
}

// Funkcja aktualizuj¹ca masê drona
void Drone::updateMass(float mass) {
	controller.added_mass += mass;          
}

// Funkcja zwracaj¹ca pozycjê kamery drona
glm::vec3 Drone::getCameraPos() const {
    glm::vec3 cameraOffset(0.0f, 1.0f, 2.0f);
    return position + cameraOffset;
};

// Funkcja zwracaj¹ca pozycjê drona
glm::vec3 Drone::getDronePos() const {
    return position;
};

// Funkcja zwracaj¹ca prêdkoœæ drona
glm::vec3 Drone::getDroneVelocity() const {
	return velocity;
}

// Funkcja zwracaj¹ca masê drona
float Drone::getDroneMass() const {
	return drone_mass;
}

// Funkcja zwracaj¹ca przechylenie drona
glm::vec2 Drone::getDroneTilt() const {
    return controller.tilt;
}

// Funkcja zwracaj¹ca prêdkoœæ skrzyde³ drona
float Drone::getWingsSpeed() const {
	return renderer.wings_speed; 
}

// Aktualizacja fizyki drona na podstawie wejœcia z klawiatury
void Drone::updatePhysics(glm::vec2 tiltInput, float verticalInput, bool& droneBroken, float deltaTime) {
	controller.UpdatePhysics(position, velocity, tiltInput, verticalInput, deltaTime, droneBroken); // aktualizacja fizyki drona
	drone_mass = controller.whole_mass; // aktualizacja masy drona+klocka
}

// Funkcja resetuj¹ca pozycjê drona do domyœlnej
void Drone::resetDronePosition() {
    position = glm::vec3(0.0f, 1.0f, 0.0f);
    velocity = glm::vec3(0.0f);
    controller.tilt = glm::vec2(0.0f);
};
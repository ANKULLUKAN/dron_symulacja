#include "Drone.h"

#include <glm/gtc/matrix_transform.hpp>
#include "ModelLoader.h"
#include "rotation.h"

void Drone::drawDrone(const glm::mat4& projection, const glm::mat4& view, const glm::vec3 lightDir) {

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
        controller.tilt.x, controller.tilt.y, velocity.y);
}

glm::vec3 Drone::projectToFloor(const glm::vec3& point, const glm::vec3& lightDir) {
    // Rzutuj punkt w kierunku œwiat³a na p³aszczyznê y=0
    float t = -point.y / lightDir.y;
    return point + t * lightDir;
}

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

void Drone::drawDroneShadow(const Shader& shader, const glm::mat4& projection, const glm::mat4& view, const glm::vec3& lightDir) {
    // Macierz modelu drona (pozycja, rotacja, skala)
    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), position);
    // Wygeneruj siatkê cienia
    generateDroneShadowMesh(modelMatrix, lightDir);

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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
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
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(shadowIndices.size()), GL_UNSIGNED_INT, 0);

    // Sprz¹tanie
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
}

void Drone::addMass(float mass) {
    controller.mass += mass;
};

glm::vec3 Drone::getCameraPos() {
    glm::vec3 cameraOffset(0.0f, 1.0f, 2.0f);
    return position + cameraOffset;
};

glm::vec3 Drone::getDronePos() {
    return position;
};

float Drone::getWingsSpeed() {
	return renderer.wings_speed; 
}

void Drone::updatePhysics(glm::vec2 tiltInput, float verticalInput, bool& droneBroken) {
    controller.UpdatePhysics(position, velocity, tiltInput, verticalInput, 1/60.0f, droneBroken);
}

void Drone::resetDronePosition() {
    position = glm::vec3(0.0f, 1.0f, 0.0f);
    velocity = glm::vec3(0.0f);
    controller.tilt = glm::vec2(0.0f);
};
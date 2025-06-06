#pragma once

#include <glad/glad.h>
#include "Shader.h"
#include "Drone.h"

class Cube {
public:
    Cube(float size, float mass);
    ~Cube();

    bool contactWithDrone(glm::vec3 dron_position);
    void Update(float deltaTime, glm::vec3 dron_position);
    void Draw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection);


    glm::vec3 contactPoint;
    glm::vec3 position;
    glm::vec3 velocity;
    float mass;
  

private:
    void setupMesh(float size);
    unsigned int VAO, VBO;
    bool attachedToDrone = false;
};
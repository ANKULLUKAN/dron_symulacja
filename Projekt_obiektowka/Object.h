#pragma once

#include <glad/glad.h>
#include "Shader.h"
#include "Drone.h"

class Cube {
public:
    Cube(float size, float mass);
    ~Cube();

    bool checkContactWithDrone(glm::vec3 dronePosition);
    void Update(float deltaTime, glm::vec3 dronePosition, bool contact);
    void Draw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection) const;

	bool attachedToDrone = false;
    glm::vec3 position;
    glm::vec3 velocity;
    float mass;

private:
    void setupMesh(float size);
    unsigned int VAO, VBO;
    
};
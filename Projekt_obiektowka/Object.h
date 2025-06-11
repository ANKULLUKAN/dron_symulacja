#pragma once

#include <glad/glad.h>
#include "Shader.h"
#include "Drone.h"

class Cube {
public:
    Cube(float size, float mass);
    ~Cube();

    bool checkContactWithDrone(glm::vec3 dronePosition, bool& droneBroken);

    void Update(float deltaTime, glm::vec3 dronePosition, bool contact, const glm::vec3 velocity_of_drone);

    void Draw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection, float tiltAngleX, float tiltAngleY) const;

	bool attachedToDrone = false;
    glm::vec3 position;
    glm::vec3 velocity;
    float mass;
    float size;
    
private:
    glm::vec3 swingOffset = glm::vec3(0.0f); // przesuniêcie wzglêdem drona
    void setupMesh(float size);
    unsigned int VAO, VBO;
    
};
#include "Drone.h"

#include <glm/gtc/matrix_transform.hpp>
#include "ModelLoader.h"
#include "rotation.h"



void Drone::drawDrone(const glm::mat4& projection, const glm::mat4& view) {

    // --- Model drona ---
    glm::vec3 lightDir = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));
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
#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "ModelLoader.h" 

class NodeRenderer {
public:
    NodeRenderer(std::vector<Mesh>& meshes);

    float wings_speed;

    void drawNodeWithRotation(const Node& node, const glm::mat4& parentTransform, GLuint shaderProgram,
        int& nodeCounter, float tiltAngleX, float tiltAngleY, float velocity_y);
private:
    void rotateWings(int nodeCounter, int targetCounter, const glm::vec3& pivot, float& wingsAngle, float speed, glm::mat4& localTransform);
    std::vector<Mesh>& meshes;
    float wingsAngle = 0.0f;
};

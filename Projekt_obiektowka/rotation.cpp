#include "rotation.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

NodeRenderer::NodeRenderer(std::vector<Mesh>& meshes)
    : meshes(meshes)
{
}

void NodeRenderer::rotateWings(int nodeCounter, int targetCounter, const glm::vec3& pivot, float& wingsAngle, float speed, glm::mat4& localTransform) {
    if (nodeCounter == targetCounter) {
        if (speed < 0.01f) speed = 0.01f;
        wingsAngle += speed;
        if (wingsAngle > glm::two_pi<float>()) wingsAngle -= glm::two_pi<float>();

        glm::mat4 toPivot = glm::translate(glm::mat4(1.0f), pivot);
        glm::mat4 fromPivot = glm::translate(glm::mat4(1.0f), -pivot);
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), wingsAngle, glm::vec3(0, 1, 0));
        localTransform = toPivot * rotation * fromPivot * localTransform;
    }
}

void NodeRenderer::drawNodeWithRotation(const Node& node, const glm::mat4& parentTransform, GLuint shaderProgram,
    int& nodeCounter, float tiltAngleX, float tiltAngleY, float velocity_y) {
    glm::mat4 localTransform = node.transform;
    const float baseSpeed = 0.05f;
    float speed = baseSpeed + 0.3f * velocity_y;

    rotateWings(nodeCounter, 127, glm::vec3(-8.4f, 4.2f, 8.4f), wingsAngle, speed, localTransform);
    rotateWings(nodeCounter, 130, glm::vec3(-8.5f, 4.2f, -8.0f), wingsAngle, speed, localTransform);
    rotateWings(nodeCounter, 133, glm::vec3(+8.5f, 4.2f, -8.0f), wingsAngle, speed, localTransform);
    rotateWings(nodeCounter, 136, glm::vec3(+8.5f, 4.2f, 8.0f), wingsAngle, speed, localTransform);

    glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), tiltAngleX, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), tiltAngleY, glm::vec3(0.0f, 0.0f, -1.0f));
    glm::mat4 globalTransform = parentTransform * rotationY * rotationX * localTransform;

    for (unsigned int i : node.meshIndices) {
        const Mesh& mesh = meshes[i];
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(globalTransform));
        glBindVertexArray(mesh.VAO);

        if (mesh.textureID) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mesh.textureID);
            glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
        }

        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
    }

    nodeCounter++;
    for (const Node& child : node.children) {
        drawNodeWithRotation(child, globalTransform, shaderProgram, nodeCounter, tiltAngleX, tiltAngleY, velocity_y);
    }
    wings_speed = speed;
}
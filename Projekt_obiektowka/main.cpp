#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "Shader.h"
#include "ModelLoader.h"


struct PhysicsBox {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 size;
    float mass;

    PhysicsBox(glm::vec3 pos, glm::vec3 sz, float m)
        : position(pos), size(sz), mass(m), velocity(0.0f) {
    }
};


float yaw = 0.0f, pitch = 0.0f;
float lastX = 400, lastY = 300;
bool firstMouse = true;
bool leftMousePressed = false;
float radius = 5.0f;

void scroll_callback(GLFWwindow*, double, double yoffset) {
    radius -= yoffset;
    if (radius < 1.0f) radius = 1.0f;
    if (radius > 20.0f) radius = 20.0f;
}

void mouse_button_callback(GLFWwindow*, int button, int action, int) {
    if (button == GLFW_MOUSE_BUTTON_LEFT)
        leftMousePressed = (action == GLFW_PRESS);
}

void cursor_position_callback(GLFWwindow*, double xpos, double ypos) {
    if (!leftMousePressed) {
        firstMouse = true;
        return;
    }

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.2f;
    yaw += xoffset * sensitivity;
    pitch += yoffset * sensitivity;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
}


const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec4 objectColor;
void main() {
    FragColor = objectColor;
}
)";


float cubeVertices[] = {
    -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f
};
float floorVertices[] = {      
    -5.0f, 0.0f, -5.0f,
     5.0f, 0.0f, -5.0f,
     5.0f, 0.0f,  5.0f,

     5.0f, 0.0f,  5.0f,
    -5.0f, 0.0f,  5.0f,
    -5.0f, 0.0f, -5.0f,
};


int nodeCounter = 0;
int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Dron z fizyka", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glEnable(GL_DEPTH_TEST);
    if (!loadModel("../model/result.gltf")) return -1;

    Shader shader(vertexShaderSource, fragmentShaderSource);

    PhysicsBox droneBox(glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(1.0f, 0.3f, 1.0f), 1.0f);

    unsigned int floorVAO, floorVBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);

    glBindVertexArray(floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    float deltaTime = 0.016f; 
    float rotationAngle = 0.0f;
    

    while (!glfwWindowShouldClose(window)) {
        int counter = 0;
        glfwPollEvents();

        glm::vec3 gravity(0.0f, -9.81f, 0.0f);

		const float hoverThrust = 9.81f; 
        const float ascendDelta = 0.2f; 
        const float descendDelta = 0.2f;

        glm::vec3 thrust(0.0f);

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            thrust.y = hoverThrust + ascendDelta;
        }
        else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) {
            thrust.y = hoverThrust - descendDelta;
        }
        else {
            thrust.y = hoverThrust;
        }
        

        glm::vec3 inputDir(0.0f);
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) inputDir.z -= 1.0f;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) inputDir.z += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) inputDir.x -= 1.0f;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) inputDir.x += 1.0f;

        float maxSpeed = 0.5f;
        if (glm::length(inputDir) > 0.0f) {
            inputDir = glm::normalize(inputDir);
        }

        // przysppieszenie pod sziftem :))
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
			maxSpeed *= 2.0f; 
		}
        glm::vec3 targetVelocity = inputDir * maxSpeed;
        droneBox.velocity.x = glm::mix(droneBox.velocity.x, targetVelocity.x, 0.1f);
        droneBox.velocity.z = glm::mix(droneBox.velocity.z, targetVelocity.z, 0.1f);

        droneBox.velocity += (gravity + thrust) * deltaTime;

        float damping = 0.99f;
        droneBox.velocity *= damping;

        droneBox.position += droneBox.velocity * deltaTime;

        if (droneBox.position.y < 0.0f) {
            droneBox.position.y = 0.0f;
            droneBox.velocity.y = 0.0f;
        }

        float maxVerticalSpeed = 0.5f;
        if (droneBox.velocity.y > maxVerticalSpeed) droneBox.velocity.y = maxVerticalSpeed;
		if (droneBox.velocity.y < -maxVerticalSpeed) droneBox.velocity.y = -maxVerticalSpeed;

        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::vec3 cameraOffset(0.0f, 1.0f, 2.0f);
        glm::vec3 cameraPos = droneBox.position + cameraOffset;
        glm::mat4 view = glm::lookAt(cameraPos, droneBox.position, glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.f / 600.f, 0.1f, 100.0f);

        // Draw floor
        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setVec4("objectColor", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)); // gray
        glm::mat4 floorModel = glm::mat4(1.0f);
        shader.setMat4("model", floorModel);
        glBindVertexArray(floorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Draw drone
        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setVec4("objectColor", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); // blue

        // takie podstawowe obracanie wzgledem wysokosci im wieksza tym szybciej :p
        rotationAngle += 1.0f + * &droneBox.position.y;


        glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), droneBox.position);
        shader.setMat4("model", modelMatrix);
        
        drawNodeWithRotation(rootNode, modelMatrix, shader.ID, nodeCounter, rotationAngle);

       
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

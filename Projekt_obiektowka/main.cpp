#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "Shader.h"
#include "ModelLoader.h"

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
void main() {
    FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}
)";

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Dron", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    if (!loadModel("E:/projektyCpp/Projekt_obiektowka/x64/Debug/model/result.gltf")) return -1;

    Shader shader(vertexShaderSource, fragmentShaderSource);
    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClearColor(0.4f, 0.2f, 0.6f, 0.5f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float camX = radius * cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        float camY = radius * sin(glm::radians(pitch));
        float camZ = radius * sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        glm::vec3 cameraPos = glm::vec3(camX, camY, camZ);

        glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.f / 600.f, 0.1f, 100.0f);

        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        drawNode(rootNode, glm::mat4(1.0f), shader.ID);

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

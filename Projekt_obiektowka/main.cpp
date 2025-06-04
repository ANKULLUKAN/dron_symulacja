#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <iostream>
#include <numbers>
#include <windows.h>

#include "Shader.h"
#include "ModelLoader.h"
#include "DroneController.h"


// Struktura opisująca pudełko fizyczne (pozycja, prędkość, rozmiar, masa)
struct PhysicsBox {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 size;
    float mass;

    PhysicsBox(glm::vec3 pos, glm::vec3 sz, float m)
        : position(pos), velocity(0.0f), size(sz), mass(m) {
    }
};

// --- Zmienne globalne do obsługi celu ---
glm::vec3 droneTarget(0.0f);
bool hasTarget = false;
PhysicsBox* g_droneBox = nullptr;
glm::mat4 g_view, g_projection;
GLFWwindow* g_window = nullptr;

// Wierzchołki sześcianu
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

// Wierzchołki podłogi (prostokąt 10x10)
float floorVertices[] = {
    -5.0f, 0.0f, -5.0f,
     5.0f, 0.0f, -5.0f,
     5.0f, 0.0f,  5.0f,

     5.0f, 0.0f,  5.0f,
    -5.0f, 0.0f,  5.0f,
    -5.0f, 0.0f, -5.0f,
};

// Wierzchołki elipsy (cień drona)
constexpr int ellipseSegments = 40;
float ellipseVertices[(ellipseSegments + 2) * 3]; // środek + segmenty + powrót do pierwszego
void generateEllipseVertices(float rx, float rz) {
    ellipseVertices[0] = 0.0f; // środek x
    ellipseVertices[1] = 0.0f; // środek y
    ellipseVertices[2] = 0.0f; // środek z
    for (int i = 0; i <= ellipseSegments; ++i) {
        float angle = 2.0f * std::numbers::pi_v<float> *i / ellipseSegments;
        ellipseVertices[3 * (i + 1) + 0] = rx * std::cos(angle);
        ellipseVertices[3 * (i + 1) + 1] = 0.0f;
        ellipseVertices[3 * (i + 1) + 2] = rz * std::sin(angle);
    }
}

int main() {

    // Inicjalizacja GLFW i OpenGL
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dron", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Załaduj model drona
    if (!LoadModel("../model/result.gltf")) return -1;

	// Inicjalizacja shadera
    Shader colorShader("shader/color.vert", "shader/color.frag");
    Shader texturedShader("shader/texture.vert", "shader/texture.frag");

    PhysicsBox droneBox(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.3f, 1.0f), 1.0f);
    g_droneBox = &droneBox;
    g_window = window;

    // Podłoga
    unsigned int floorVAO, floorVBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);
    glBindVertexArray(floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr));
    glEnableVertexAttribArray(0);

	// Cień drona (elipsa)
    generateEllipseVertices(0.15f, 0.15f); // promienie elipsy
    unsigned int ellipseVAO, ellipseVBO;
    glGenVertexArrays(1, &ellipseVAO);
    glGenBuffers(1, &ellipseVBO);
    glBindVertexArray(ellipseVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ellipseVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ellipseVertices), ellipseVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr));
    glEnableVertexAttribArray(0);

    DroneController droneController(0.2f, 0.02f, 0.2f, 1.0f);

    bool droneBroken = false;

    while (!glfwWindowShouldClose(window)) {

        // --- Aktualizacja tytułu okna z pozycją drona ---
        char title[128];
        snprintf(title, sizeof(title), "Dron - Pozycja: X=%.2f Y=%.2f Z=%.2f",
            droneBox.position.x, droneBox.position.y, droneBox.position.z);
        glfwSetWindowTitle(window, title);

        int nodeCounter = 0;
        glfwPollEvents();

        glm::vec3 cameraOffset(0.0f, 1.0f, 2.0f);
        glm::vec3 cameraPos = droneBox.position + cameraOffset;
        glm::mat4 view = glm::lookAt(cameraPos, droneBox.position, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.f / 600.f, 0.1f, 100.0f);

        g_view = view;
        g_projection = projection;

        glm::vec2 tiltInput(0.0f);
        float verticalInput = 0.0f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) tiltInput.x -= 1.0f; // pitch w przód
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) tiltInput.x += 1.0f; // pitch w tył
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) tiltInput.y -= 1.0f; // roll w prawo
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) tiltInput.y += 1.0f; // roll w lewo
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) verticalInput += 1.0f; // w górę
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) verticalInput -= 1.0f; // w dół

        droneController.UpdatePhysics(droneBox.position, droneBox.velocity, tiltInput, verticalInput, 1/60.f, droneBroken);

        if (droneBroken) {
            static bool messageShown = false;
            while (1 > 0) {
                glfwPollEvents();
                if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
                    droneBox.position = glm::vec3(0.0f, 1.0f, 0.0f);
                    droneBox.velocity = glm::vec3(0.0f);
                    droneController.tilt = glm::vec2(0.0f);
                    droneBroken = false;
					messageShown = false; // resetuj komunikat
                    break; // wyjdź z pętli, aby kontynuować grę
                }
                if (!messageShown) {
                    MessageBoxA(nullptr, "Kolizja z podłogą! Wciśnij R, aby zresetować drona.", "Kolizja", MB_OK | MB_ICONWARNING);
                    messageShown = true; // pokaż komunikat tylko raz
                }
            }
        }
      
        // --- Renderowanie sceny ---
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Podłoga
        colorShader.Use();
        colorShader.SetMat4("view", view);
        colorShader.SetMat4("projection", projection);
        colorShader.SetVec4("objectColor", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
        glm::mat4 floorModel = glm::mat4(1.0f);
        colorShader.SetMat4("model", floorModel);
        glBindVertexArray(floorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), droneBox.position);

        // --- Cień drona jako elipsa ---
        colorShader.Use();
        colorShader.SetMat4("view", view);
        colorShader.SetMat4("projection", projection);
        colorShader.SetVec4("objectColor", glm::vec4(0.0f, 0.0f, 0.0f, 0.35f)); // półprzezroczysty cień

        // Model elipsy przesunięty pod drona (na podłogę)
        glm::mat4 shadowEllipseModel = glm::translate(glm::mat4(1.0f), glm::vec3(droneBox.position.x, 0.01f, droneBox.position.z));
        // Opcjonalnie: skalowanie cienia w zależności od wysokości drona
        float scaleY = 1.0f - glm::clamp(droneBox.position.y / 10.0f, 0.0f, 0.7f);
        shadowEllipseModel = glm::scale(shadowEllipseModel, glm::vec3(1.0f, 1.0f, scaleY));
        colorShader.SetMat4("model", shadowEllipseModel);

        glBindVertexArray(ellipseVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, ellipseSegments + 2);

        // --- Model drona ---
        texturedShader.Use();
        texturedShader.SetMat4("view", view);
        texturedShader.SetMat4("projection", projection);
        texturedShader.SetMat4("model", modelMatrix);
        texturedShader.SetVec3("lightDir", glm::vec3(-1.0f, -1.0f, -1.0f));
        texturedShader.SetVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
        texturedShader.SetVec3("viewPos", cameraPos);

        drawNodeWithRotation(rootNode, modelMatrix, texturedShader.Id, nodeCounter, 
            droneController.tilt.x, droneController.tilt.y);

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
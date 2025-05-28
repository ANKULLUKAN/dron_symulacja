#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "ModelLoader.h"

// Struktura opisująca pudełko fizyczne (pozycja, prędkość, rozmiar, masa)
struct PhysicsBox {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 size;
    float mass;

    PhysicsBox(glm::vec3 pos, glm::vec3 sz, float m)
        : position(pos), size(sz), mass(m), velocity(0.0f) {
    }
};

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

int main() {
    // Inicjalizacja GLFW i OpenGL
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Dron", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glEnable(GL_DEPTH_TEST);

    // Wczytanie modelu drona (GLTF)
    if (!loadModel("../model/result.gltf")) return -1;

    // Tworzenie shadera
    Shader shader{};

    // Inicjalizacja obiektu fizycznego drona
    PhysicsBox droneBox(glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(1.0f, 0.3f, 1.0f), 1.0f);

    // Tworzenie VAO i VBO dla podłogi
    unsigned int floorVAO, floorVBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);

    glBindVertexArray(floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    float rotationAngle = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        int nodeCounter = 0;
        glfwPollEvents();

        // --- Prosta fizyka drona: identyczna w każdej osi ---
        const float accel = 0.05f;         // Przyspieszenie w każdej osi
        const float damping = 0.98f;        // Tłumienie
        const float maxSpeed = 0.05f;       // Maksymalna prędkość w każdej osi
        const float stopThreshold = 0.005f; // Próg uznania prędkości za "zatrzymaną"

        // Odczyt wejścia
        glm::vec3 input(0.0f);
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) input.z -= 0.1f;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) input.z += 0.1f;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) input.x -= 0.1f;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) input.x += 0.1f;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) input.y += 0.1f;
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) input.y -= 0.1f;

        // Ujednolicone sterowanie i blokada zmiany kierunku
        for (int i = 0; i < 3; ++i) {
            if (std::abs(input[i]) > 0.0f) {
                // Jeśli prędkość jest bliska zera, zerujemy ją i pozwalamy ruszyć w nowym kierunku
                if (std::abs(droneBox.velocity[i]) < stopThreshold) {
                    droneBox.velocity[i] = 0.0f;
                    droneBox.velocity[i] += input[i] * accel;
                }
                // Jeśli kierunek wejścia zgadza się z kierunkiem prędkości, przyspieszamy normalnie
                else if ((input[i] > 0.0f && droneBox.velocity[i] > 0.0f) ||
                    (input[i] < 0.0f && droneBox.velocity[i] < 0.0f)) {
                    droneBox.velocity[i] += input[i] * accel;
                }
                // Jeśli kierunek wejścia przeciwny do prędkości, nie przyspieszaj (pozwól działać tłumieniu)
                // else: nic nie robimy, aż prędkość wyhamuje do zera
            }
        }

        // Tłumienie
        droneBox.velocity *= damping;

        // Ograniczenie maksymalnej prędkości w każdej osi
        for (int i = 0; i < 3; ++i) {
            if (droneBox.velocity[i] > maxSpeed) droneBox.velocity[i] = maxSpeed;
            if (droneBox.velocity[i] < -maxSpeed) droneBox.velocity[i] = -maxSpeed;
        }

        // Aktualizacja pozycji
        droneBox.position += droneBox.velocity;

        // Ograniczenie ruchu w dół (podłoga)
        if (droneBox.position.y < 0.0f) {
            droneBox.position.y = 0.0f;
            droneBox.velocity.y = 0.0f;
        }


        // --- Renderowanie sceny ---
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Ustawienie kamery śledzącej drona z góry pod kątem
        glm::vec3 cameraOffset(0.0f, 1.0f, 2.0f);
        glm::vec3 cameraPos = droneBox.position + cameraOffset;
        glm::mat4 view = glm::lookAt(cameraPos, droneBox.position, glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.f / 600.f, 0.1f, 100.0f);

        // Rysowanie podłogi (szary kolor)
        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setVec4("objectColor", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
        glm::mat4 floorModel = glm::mat4(1.0f);
        shader.setMat4("model", floorModel);
        glBindVertexArray(floorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Rysowanie drona (czerwony kolor)
        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setVec4("objectColor", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

        glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), droneBox.position);
        shader.setMat4("model", modelMatrix);

        // Rysowanie modelu drona z rotacją (jeśli jest obsługiwana)
        drawNodeWithRotation(rootNode, modelMatrix, shader.ID, nodeCounter, rotationAngle);

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
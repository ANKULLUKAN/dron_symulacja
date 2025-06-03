#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <numbers>
#include "Shader.h"
#include "ModelLoader.h"
#include <windows.h>

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

// --- Funkcja rzutująca kliknięcie na podłogę (y=0) ---
glm::vec3 screenToWorld(float mouseX, float mouseY, int width, int height, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& camPos) {
    float x = (2.0f * mouseX) / width - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / height;
    glm::vec4 ray_clip = glm::vec4(x, y, -1.0f, 1.0f);

    glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);

    glm::vec3 ray_world = glm::vec3(glm::inverse(view) * ray_eye);
    ray_world = glm::normalize(ray_world);

    // Przecięcie promienia z płaszczyzną y=0
    float t = -camPos.y / ray_world.y;
    glm::vec3 intersection = camPos + t * ray_world;
    return intersection;
}

// --- Callback myszy ---
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        int width, height;
        glfwGetWindowSize(window, &width, &height);
       
        glm::vec3 cameraOffset(0.0f, 1.0f, 2.0f);
        glm::vec3 camPos = g_droneBox->position + cameraOffset;

        // Użyj aktualnych macierzy
        glm::vec3 target = screenToWorld(static_cast<float>(xpos), static_cast<float>(ypos), width, height, g_view, g_projection, camPos);
        target.y = 0.0f; // Lądujemy na podłodze
        droneTarget = target;
        hasTarget = true;
    }
}

std::vector<float> generateShadowVertices(float radiusX, float radiusZ, int segments = 32) {
    std::vector<float> vertices;
    vertices.push_back(0.0f); 
    vertices.push_back(0.0f); 
    vertices.push_back(0.0f); 
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * std::numbers::pi_v<float> * i / segments;
        float x = radiusX * cos(angle);
        float z = radiusZ * sin(angle);
        vertices.push_back(x);
        vertices.push_back(0.0f);
        vertices.push_back(z);
    }
    return vertices;
}


bool droneBroken = false;
int main() {

    // Inicjalizacja GLFW i OpenGL
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Dron", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
    glEnable(GL_DEPTH_TEST);

	// Załaduj model drona
    if (!LoadModel("../model/result.gltf")) return -1;

	// Inicjalizacja shadera
    Shader colorShader("shader/color.vert", "shader/color.frag");
    Shader texturedShader("shader/texture.vert", "shader/texture.frag");

    PhysicsBox droneBox(glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(1.0f, 0.3f, 1.0f), 1.0f);
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

    // Cień
    std::vector<float> shadowVertices = generateShadowVertices(0.5f, 0.25f, 32); // elipsa pod dronem
    unsigned int shadowVAO, shadowVBO;
    glGenVertexArrays(1, &shadowVAO);
    glGenBuffers(1, &shadowVBO);
    glBindVertexArray(shadowVAO);
    glBindBuffer(GL_ARRAY_BUFFER, shadowVBO);
    glBufferData(GL_ARRAY_BUFFER, shadowVertices.size() * sizeof(float), shadowVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr));
    glEnableVertexAttribArray(0);

    glfwSetMouseButtonCallback(window, mouse_button_callback);

    while (!glfwWindowShouldClose(window)) {

		// --- Obsługa wejścia i aktualizacja stanu drona ---
        int nodeCounter = 0;
        glfwPollEvents();

        // --- Prosta fizyka drona: identyczna w każdej osi ---
        constexpr float accel = 0.05f;
        constexpr float damping = 0.99f;
        constexpr float maxSpeed = 0.05f;
        constexpr float stopThreshold = 0.005f;
        constexpr float maxTiltAngle = 10.0f;

        // Ustawienie kamery śledzącej drona z góry pod kątem
        glm::vec3 cameraOffset(0.0f, 1.0f, 2.0f);
        glm::vec3 cameraPos = droneBox.position + cameraOffset;
        glm::mat4 view = glm::lookAt(cameraPos, droneBox.position, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.f / 600.f, 0.1f, 100.0f);

        // Zapamiętaj macierze do rzutowania kliknięcia
        g_view = view;
        g_projection = projection;

        // Odczyt wejścia
        glm::vec3 input(0.0f);
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) input.z -= 0.1f;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) input.z += 0.1f;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) input.x -= 0.1f;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) input.x += 0.1f;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) input.y += 0.1f;
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) input.y -= 0.1f;

        
        // W głównej pętli:
        if (droneBroken) {
            static bool messageShown = false;
            if (!messageShown) {
                MessageBoxA(nullptr, "Press R to restart.", "Collision!", MB_OK);
                messageShown = true;
            }
            // Czekaj na wciśnięcie R
            if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
                droneBox.position = glm::vec3(0.0f, 5.0f, 0.0f);
                droneBox.velocity = glm::vec3(0.0f);
                droneBroken = false;
                hasTarget = false;
                messageShown = false;
            }
            // Pomijaj resztę logiki, dopóki nie zrestartujesz
            glfwSwapBuffers(window);
            continue;
        }


        // Sterowanie do miejsca kliknięcia
        if (hasTarget) {
            glm::vec3 toTarget = droneTarget - droneBox.position;
            toTarget.y = 0.0f; // Lataj tylko po podłodze
            float dist = glm::length(toTarget);
            if (dist > 0.1f) {
                glm::vec3 dir = glm::normalize(toTarget);
                input.x = dir.x * 0.1f;
                input.z = dir.z * 0.1f;
            }
            else {
                hasTarget = false;
            }
        }

        // Ujednolicone sterowanie i blokada zmiany kierunku
        for (int i = 0; i < 3; ++i) {
            if (std::abs(input[i]) > 0.0f) {
	            
	            if (std::abs(droneBox.velocity[i]) < stopThreshold) {
                    droneBox.velocity[i] = 0.0f;
                    droneBox.velocity[i] += input[i] * accel;
                }
                else if ((input[i] > 0.0f && droneBox.velocity[i] > 0.0f) ||
                    (input[i] < 0.0f && droneBox.velocity[i] < 0.0f)) {
                    droneBox.velocity[i] += input[i] * accel;
                }
            }
        }

        droneBox.velocity *= damping;

        for (int i = 0; i < 3; ++i) {
            if (droneBox.velocity[i] > maxSpeed) droneBox.velocity[i] = maxSpeed;
            if (droneBox.velocity[i] < -maxSpeed) droneBox.velocity[i] = -maxSpeed;
        }

        droneBox.position += droneBox.velocity;

        if (droneBox.position.y < 0.0f) {
            droneBox.position.y = 0.0f;
            droneBox.velocity.y = 0.0f;
            if (!droneBroken) {
                droneBroken = true;             
            }
        }

        // --- Renderowanie sceny ---
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Tworzenie podłogi 
        colorShader.use();
        colorShader.setMat4("view", view);
        colorShader.setMat4("projection", projection);
        colorShader.setVec4("objectColor", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
        glm::mat4 floorModel = glm::mat4(1.0f);
        colorShader.setMat4("model", floorModel);

        glBindVertexArray(floorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Tworzenie modelu
        texturedShader.use();
        texturedShader.setMat4("view", view);
        texturedShader.setMat4("projection", projection);
        glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), droneBox.position);
        texturedShader.setMat4("model", modelMatrix);
        texturedShader.setVec3("lightDir", glm::vec3(-1.0f, -1.0f, -1.0f)); // kierunek światła
        texturedShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));  // kolor światła
        texturedShader.setVec3("viewPos", cameraPos); // pozycja kamery

		// Kąty w zależności od prędkości trzeba cos z ta prędkości ogarnąć, bo on nie przyspiesza, ale od razu pędzi z pełną prędkością
        float tiltAngleX = glm::clamp(droneBox.velocity.z / maxSpeed, -1.0f, 1.0f) * maxTiltAngle;
        float tiltAngleY = glm::clamp(droneBox.velocity.x / maxSpeed, -1.0f, 1.0f) * maxTiltAngle;

        // Funkcja rysująca
        drawNodeWithRotation(rootNode, modelMatrix, texturedShader.ID, nodeCounter, tiltAngleX, tiltAngleY);
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
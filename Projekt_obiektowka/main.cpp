#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <windows.h>

#include "Drone.h"
#include "Shader.h"
#include "ModelLoader.h"


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

constexpr int ellipseSegments = 40;
float ellipseVertices[(ellipseSegments + 2) * 3];
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

    Drone drone(glm::vec3(0.0f, 1.0f, 0.0f));

    // Podłoga
    unsigned int floorVAO, floorVBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);
    glBindVertexArray(floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr));
    glEnableVertexAttribArray(0);

    // Cień drona
    generateEllipseVertices(0.2f, 0.1f); // promienie elipsy (dostosuj do rozmiaru drona)
    unsigned int ellipseVAO, ellipseVBO;
    glGenVertexArrays(1, &ellipseVAO);
    glGenBuffers(1, &ellipseVBO);
    glBindVertexArray(ellipseVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ellipseVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ellipseVertices), ellipseVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr));
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    bool droneBroken = false;

    while (!glfwWindowShouldClose(window)) {

        // --- Aktualizacja tytułu okna z pozycją drona ---
        char title[128];
        snprintf(title, sizeof(title), "Dron - Pozycja: X=%.2f Y=%.2f Z=%.2f",
            drone.getDronePos().x, drone.getDronePos().y, drone.getDronePos().z);
        glfwSetWindowTitle(window, title);

        glfwPollEvents();

        glm::mat4 view = glm::lookAt(drone.getCameraPos(), drone.getDronePos(), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.f / 600.f, 0.1f, 100.0f);

        glm::vec2 tiltInput(0.0f);
        float verticalInput = 0.0f;

        // Obsługa pada (GLFW_JOYSTICK_1)
        if (glfwJoystickPresent(GLFW_JOYSTICK_1) && glfwJoystickIsGamepad(GLFW_JOYSTICK_1)) {
            GLFWgamepadstate state;
            if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state)) {
                // Lewy drążek: pitch (oś Y), roll (oś X)
                tiltInput.x = state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]; // pitch: przód/tył
                tiltInput.y = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];  // roll: lewo/prawo

                // DEADZONE
                const float deadzone = 0.15f;
                if (std::abs(tiltInput.x) < deadzone) tiltInput.x = 0.0f;
                if (std::abs(tiltInput.y) < deadzone) tiltInput.y = 0.0f;


                // Spusty: w górę (RT), w dół (LT)
                float up = (state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] + 1.0f) / 2.0f;   // 0..1
                float down = (state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] + 1.0f) / 2.0f;  // 0..1
                verticalInput = up - down; // RT podnosi, LT opuszcza
            }
        }
        
        // Obsługa klawiatury
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) tiltInput.x -= 1.0f;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) tiltInput.x += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) tiltInput.y -= 1.0f;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) tiltInput.y += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) verticalInput += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) verticalInput -= 1.0f;

        drone.updatePhysics(tiltInput, verticalInput, droneBroken);

        if (droneBroken) {
            static bool messageShown = false;
            while (1 > 0) {
                glfwPollEvents();
                if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
                    drone.resetDronePosition();
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

		// --- Podłoga ---
        glm::mat4 floorModel = glm::mat4(1.0f);

        colorShader.Use();
        colorShader.SetMat4("view", view);
        colorShader.SetMat4("projection", projection);
        colorShader.SetVec4("objectColor", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
        colorShader.SetMat4("model", floorModel);
        glBindVertexArray(floorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

    	// --- Cień drona (elipsa) ---
        glm::mat4 shadowEllipseModel = glm::translate(glm::mat4(1.0f), glm::vec3(drone.getDronePos().x, 0.01f, drone.getDronePos().z));
        colorShader.Use();
        colorShader.SetMat4("view", view);
        colorShader.SetMat4("projection", projection);
        colorShader.SetVec4("objectColor", glm::vec4(0.0f, 0.0f, 0.0f, 0.35f)); // półprzezroczysty cień
        colorShader.SetMat4("model", shadowEllipseModel);
        glBindVertexArray(ellipseVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, ellipseSegments + 2);

		drone.drawDrone(projection, view);

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
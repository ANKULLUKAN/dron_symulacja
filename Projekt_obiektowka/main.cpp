#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <windows.h>

#include "Drone.h"
#include "Shader.h"
#include "ModelLoader.h"
#include "floor.h"
#include "Object.h"


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
    
	// Inicjalizacja drona
    Drone drone(glm::vec3(0.0f, 1.0f, 0.0f));
    bool droneBroken = false;

	// Inicjalizacja kostki
    Cube cube(0.1f, 5.0f);
    bool attached = false;

	// Inicjalizacja podłogi
    Floor floor;

	// Ustawienie kierunku światła
    glm::vec3 lightDir = glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f));

	float deltaTime = 1/60.0f;

    while (!glfwWindowShouldClose(window)) {

        // --- Aktualizacja tytułu okna z pozycją drona ---
        char title[128];
        snprintf(title, sizeof(title), "Pozycja: X=%.2f Y=%.2f Z=%.2f Predkosc skrzydel(obr/s): %.2f Masa: %.2f",
            drone.getDronePos().x, drone.getDronePos().y, drone.getDronePos().z, drone.getWingsSpeed() * 150, drone.drone_mass);
        glfwSetWindowTitle(window, title);

        glfwPollEvents();

        glm::vec2 tiltInput(0.0f);
        float verticalInput = 0.0f;

    	// Obsługa klawiatury
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) tiltInput.x -= 1.0f;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) tiltInput.x += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) tiltInput.y -= 1.0f;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) tiltInput.y += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) verticalInput += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) verticalInput -= 1.0f;

		// Przełączanie kontaktu z kostką i aktualizacja masy drona
        static bool prevEPressed = false;
        bool ePressed = (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS);
        if (ePressed && !prevEPressed) {
            if (cube.checkContactWithDrone(drone.getDronePos()) || attached) {
                attached = !attached; // przełącz
                if (attached) {
                    drone.updateMass(cube.mass); // przyczepienie
                }
                else {
                    drone.updateMass(-cube.mass); // odczepienie
                }
            }
        }
        prevEPressed = ePressed;

		// --- Aktualizacja fizyki drona i kostki---
        drone.updatePhysics(tiltInput, verticalInput, droneBroken, deltaTime);
        cube.Update(deltaTime, drone.getDronePos(), attached); // Aktualizacja fizyki kostki;

		// --- Sprawdzenie kolizji z podłogą ---
        if (droneBroken) {
            static bool messageShown = false;
            while (1 > 0) {
                glfwPollEvents();
                if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
                    drone.resetDronePosition();
                    droneBroken = false;
					messageShown = false; // resetuj komunikat
                    break; // wyjdź z pętli, aby kontynuować
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

		// --- Ustawienia shadera ---
        glm::mat4 view = glm::lookAt(drone.getCameraPos(), drone.getDronePos(), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.f / 600.f, 0.1f, 100.0f);

		// --- Rysowanie podłogi ---
        floor.Draw(colorShader, view, projection);

		// --- Rysowanie drona i jego cienia ---
        drone.drawDroneShadow(colorShader, projection, view, lightDir);
		drone.drawDrone(texturedShader, projection, view, lightDir);

		// --- Rysowanie kostki ---
		
		cube.Draw(colorShader, view, projection);

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
#pragma once

#include "DroneController.h"
#include "rotation.h"
#include "Shader.h"


class Drone {

public:
	// Konstruktor: inicjalizuje shader, pozycjê i kontroler drona
	Drone(const glm::vec3& startPosition)
		:texturedShader(Shader("shader/texture.vert", "shader/texture.frag")), position(startPosition),
	velocity(0.0f), controller(0.1f, 0.02f, 0.2f, 1.0f), renderer(NodeRenderer(meshes)) {
	}

	// Funkcja rysuj¹ca drona
	void drawDrone(const glm::mat4& projection, const glm::mat4& view);

	void addMass(float mass);

	// Funkcje pomocnicze do pobierania pozycji kamery i drona oraz prêdkoœci skrzyde³
	glm::vec3 getCameraPos();
	glm::vec3 getDronePos();
	float getWingsSpeed();

	// Aktualizuje fizykê drona na podstawie wejœcia z kontrolera
	void updatePhysics(glm::vec2 tiltInput, float verticalInput, bool& droneBroken);

	// Resetuje pozycjê drona do domyœlnej
	void resetDronePosition();

private:
	Shader texturedShader;
	glm::vec3 position; // Pozycja drona
	glm::vec3 velocity; // Prêdkoœæ drona
	DroneController controller; // Kontroler drona
	NodeRenderer renderer; // Renderer wêz³ów (do animacji skrzyde³ek i innych elementów)
};

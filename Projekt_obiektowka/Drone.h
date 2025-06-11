#pragma once

#include "DroneController.h"
#include "rotation.h"
#include "Shader.h"
#include "ModelLoader.h"


class Drone {

public:
	// Konstruktor: inicjalizuje shader, pozycjê i kontroler drona
	Drone(const glm::vec3& startPosition)
		:position(startPosition),
	velocity(0.0f), controller(0.1f, 0.02f, 0.2f, 1.0f), renderer(NodeRenderer(meshes)) {

		if (!meshes.empty()) {
			droneVertices.clear();
			for (const auto& vertex : meshes[0].vertices) {
				droneVertices.push_back(vertex.position);
			}
			droneIndices = meshes[0].indices;
		}
	}

	// Funkcja rysuj¹ca drona
	void drawDrone(const Shader& texturedShader, const glm::mat4& projection, const glm::mat4& view, const glm::vec3 lightDir);

	// Funkcja rysuj¹ca cieñ drona (elipsa)
	void drawDroneShadow(const Shader& shader, const glm::mat4& projection, const glm::mat4& view) const;
	glm::vec3 projectToFloor(const glm::vec3& point, const glm::vec3& lightDir);
	void generateDroneShadowMesh(const glm::mat4& modelMatrix, const glm::vec3& lightDir);
	void drawDroneShadow(const Shader& shader, const glm::mat4& projection,
		const glm::mat4& view, const glm::vec3& lightDir);

	// Funkcja aktualizuj¹ca masê do drona
	void updateMass(float mass);

	// Funkcje pomocnicze do pobierania pozycji kamery i drona oraz prêdkoœci skrzyde³
	glm::vec3 getCameraPos();
	glm::vec3 getDronePos();
	float getWingsSpeed();

	// Aktualizuje fizykê drona na podstawie wejœcia z kontrolera
	void updatePhysics(glm::vec2 tiltInput, float verticalInput, bool& droneBroken, float deltaTime);

	// Resetuje pozycjê drona do domyœlnej
	void resetDronePosition();

	float drone_mass;

private:
	glm::vec3 position; // Pozycja drona
	glm::vec3 velocity; // Prêdkoœæ drona
	DroneController controller; // Kontroler drona
	NodeRenderer renderer; // Renderer wêz³ów (do animacji skrzyde³ek i innych elementów)

	std::vector<glm::vec3> droneVertices;
	std::vector<unsigned int> droneIndices;
	std::vector<glm::vec3> shadowVertices;
	std::vector<unsigned int> shadowIndices;
};

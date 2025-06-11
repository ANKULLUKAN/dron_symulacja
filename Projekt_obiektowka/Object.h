#pragma once

#include <glad/glad.h>
#include "Shader.h"
#include "Drone.h"

class Cube {

public:
	// Konstruktor
	Cube(const float size, const float mass)
		:position(0.0f, size / 2.0f, 0.0f), velocity(0.0f), mass(mass), size(size)
	{
		setupMesh(size);
	}

	// Destruktor
    ~Cube() {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	};

	// Funkcje do sprawdzania kolizji z dronem
    bool checkContactWithDrone(glm::vec3 dronePosition, bool& droneBroken);

	// Aktualizacja fizyki kostki
    void Update(float deltaTime, glm::vec3 dronePosition, bool contact, const glm::vec3 droneVelocity);

	// Funkcja rysuj¹ca kostkê
    void Draw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3 lightDir, const glm::vec3 cameraPos) const;

	// Funkcja zwracaj¹ca masê kostki
    float getCubeMass();

private:
    glm::vec3 swingOffset = glm::vec3(0.0f); // przesuniêcie wzglêdem drona
	void setupMesh(float size); // ustawia siatkê kostki
	unsigned int VAO, VBO; // identyfikatory OpenGL dla VAO i VBO

	glm::vec3 position; // pozycja kostki
	glm::vec3 velocity; // prêdkoœæ kostki
	float mass; // masa kostki
	float size; // rozmiar kostki
	bool attachedToDrone = false; // czy kostka jest przyczepiona do drona
};
#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Shader.h"

class Floor {

public:

	// Konstruktor inicjalizuj¹cy VAO i VBO dla pod³ogi
	Floor(int size, int gridSize)
		:gridSize(gridSize), size(size)
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
	}

	// Destruktor zwalniaj¹cy zasoby OpenGL
	~Floor() {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
    }

	// Funkcja rysuj¹ca pod³ogê
	void Draw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3 lightDir, const glm::vec3 cameraPos) const;

private:
	unsigned int VAO, VBO; // identyfikatory OpenGL dla VAO i VBO
	int gridSize; // rozmiar siatki pod³ogi (10x10)
	int size; // rozmiar ca³ej pod³ogi
};

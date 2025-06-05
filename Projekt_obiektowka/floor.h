#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Shader.h"

class Floor {

public:
    Floor();
    ~Floor();
    void Draw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection);

private:
    unsigned int VAO, VBO;
	int gridSize = 10; 
};

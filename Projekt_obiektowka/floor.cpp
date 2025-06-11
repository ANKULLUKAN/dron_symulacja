#include "floor.h"

// Wierzcho³ki pod³ogi
static float floorVertices[] = {
    -5.0f, 0.0f, -5.0f,
     5.0f, 0.0f, -5.0f,
     5.0f, 0.0f,  5.0f,
     5.0f, 0.0f,  5.0f,
    -5.0f, 0.0f,  5.0f,
    -5.0f, 0.0f, -5.0f,
};

// Funkcja rysuj¹ca pod³ogê
void Floor::Draw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3 lightDir, const glm::vec3 cameraPos) const {

	shader.Use();
    shader.SetMat4("view", view);
    shader.SetMat4("projection", projection);
    shader.SetVec3("lightDir", lightDir);
    shader.SetVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.SetVec3("viewPos", cameraPos);

    glBindVertexArray(VAO);

	const float tile = size / gridSize; // rozmiar pojedynczej p³ytki

    for (int x = 0; x < gridSize; ++x) {
        for (int z = 0; z < gridSize; ++z) {

        	// Kolor szachownicy
	        const bool isBlack = (x + z) % 2 == 0;
            glm::vec4 color = isBlack ? glm::vec4(0.1f, 0.1f, 0.1f, 1.0f) : glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
            shader.SetVec4("objectColor", color);

	        const float x0 = -size / 2 + x * tile;
	        const float x1 = x0 + tile;
	        const float z0 = -size / 2 + z * tile;
	        const float z1 = z0 + tile;

			// Wspó³rzêdne wierzcho³ków kwadratu
	        const float quad[] = {
			    x0, 0.0f, z0,  0.0f, 1.0f, 0.0f,
			    x1, 0.0f, z0,  0.0f, 1.0f, 0.0f,
			    x1, 0.0f, z1,  0.0f, 1.0f, 0.0f,
			    x1, 0.0f, z1,  0.0f, 1.0f, 0.0f,
			    x0, 0.0f, z1,  0.0f, 1.0f, 0.0f,
			    x0, 0.0f, z0,  0.0f, 1.0f, 0.0f,
            };

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_DYNAMIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), static_cast<void*>(nullptr)); // pozycja
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float))); // normalne
            glEnableVertexAttribArray(1);

            glm::mat4 model = glm::mat4(1.0f);
            shader.SetMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }

    glBindVertexArray(0);
}
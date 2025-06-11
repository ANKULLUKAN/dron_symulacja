#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "ModelLoader.h" 

// Klasa odpowiedzialna za rysowanie wêz³ów z uwzglêdnieniem obrotu skrzyde³
class NodeRenderer {
public:
	// Konstruktor przyjmuj¹cy referencjê do wektora siatek
	NodeRenderer(std::vector<Mesh>& meshes)
		: meshes(meshes){}

	float wings_speed; // Przechowuje prêdkoœæ skrzyde³

	// Funkcja rysuj¹ca wêze³ z uwzglêdnieniem obrotu skrzyde³
    void drawNodeWithRotation(const Node& node, const glm::mat4& parentTransform, GLuint shaderProgram,
        int& nodeCounter, float tiltAngleX, float tiltAngleY, float velocity_y);
private:
	void rotateWings(int nodeCounter, int targetCounter, const glm::vec3& pivot, 
		float& wingsAngle, float speed, glm::mat4& localTransform); // Funkcja obracaj¹ca skrzyd³a wêz³a
	std::vector<Mesh>& meshes; // Referencja do wektora siatek, które bêd¹ rysowane
	float wingsAngle = 0.0f; // Przechowuje k¹t obrotu skrzyde³
};

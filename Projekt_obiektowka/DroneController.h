#pragma once
#include <glm/glm.hpp>
#include <numbers>

// Klasa do obs³ugi sterowania dronem z p³ynnym przyspieszaniem i zwalnianiem
class DroneController {
public:
	DroneController(float thrustStrength, float verticalThrustStrength, float dragCoeff, float mass)
		: thrustStrength(thrustStrength), verticalThrustStrength(verticalThrustStrength), dragCoeff(dragCoeff), mass(mass) {
	}
    // Aktualny k¹t pochylenia (pitch, roll)
    glm::vec2 tilt = glm::vec2(0.0f); // [pitch, roll]

    float maxTilt = 10.0f * (std::numbers::pi_v<float> / 180.0f); // w radianach

    // Aktualizuje tilt w kierunku tiltTarget
    void updateTilt(const glm::vec2& tiltInput, float deltaTime);

    // G³ówna funkcja fizyki drona
    void updatePhysics(
        glm::vec3& position,
        glm::vec3& velocity,
        const glm::vec2& tiltInput, // wejœcie: ¿¹dany pitch/roll
        float verticalInput,
        float deltaTime
    );

private:
    // Parametry fizyczne
    float mass;           // masa drona
    float thrustStrength; // si³a ci¹gu (dostosuj do swoich potrzeb)
	float verticalThrustStrength; // si³a ci¹gu pionowego (np. do unoszenia siê w górê)
    float dragCoeff;     // opór powietrza
};

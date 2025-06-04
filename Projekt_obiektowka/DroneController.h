#pragma once
#include <glm/glm.hpp>
#include <numbers>

// Klasa do obs³ugi sterowania dronem z p³ynnym przyspieszaniem i zwalnianiem
class DroneController {
public:
	DroneController(const float thrustStrength, const float verticalThrustStrength, const float dragCoefficient, const float mass)
		: mass(mass), thrustStrength(thrustStrength), verticalThrustStrength(verticalThrustStrength), dragCoefficient(dragCoefficient) {
	}

    // Aktualny k¹t pochylenia (pitch, roll)
    glm::vec2 tilt = glm::vec2(0.0f); // [pitch, roll]

    // G³ówna funkcja fizyki drona
    void UpdatePhysics(
        glm::vec3& position,
        glm::vec3& velocity,
        const glm::vec2& tiltInput, // wejœcie: ¿¹dany pitch/roll
        float verticalInput,
        float deltaTime,
        bool& collidedWithGround
    );

private:
    // Parametry fizyczne
    float mass;           // masa drona
	float thrustStrength; // si³a ci¹gu w poziomie (np. do poruszania siê w przód/ty³)
	float verticalThrustStrength; // si³a ci¹gu pionowego (np. do unoszenia siê w górê)
    float dragCoefficient;     // opór powietrza
	float maxTilt = 15.0f * (std::numbers::pi_v<float> / 180.0f); // maksymalny k¹t pochylenia w radianach (10 stopni)

    // Aktualizuje tilt w kierunku tiltTarget
    void UpdateTilt(const glm::vec2& tiltInput, float deltaTime);
};

#pragma once

#include <glm/glm.hpp>
#include <numbers>

// Klasa do obs³ugi sterowania dronem z p³ynnym przyspieszaniem i zwalnianiem
class DroneController {
public:
	// Konstruktor z parametrami fizycznymi drona
	DroneController(const float thrustStrength, const float verticalThrustStrength, const float dragCoefficient, const float mass)
		: mass(mass), thrustStrength(thrustStrength), verticalThrustStrength(verticalThrustStrength), dragCoefficient(dragCoefficient) {
	}

	// Wektor przechowuj¹cy aktualny tilt drona (pochylenie) w formacie [pitch, roll]
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
	
	float thrust_y; // si³a ci¹gu w pionie (np. do unoszenia siê w górê)
	float added_mass; // masa dodatkowych obiektów dodanych do drona (np. ³adunek)
	float whole_mass; // ca³kowita masa drona z dodan¹ mas¹ obiektów
private:
    // Parametry fizyczne
    float mass;       // masa drona
	float thrustStrength; // si³a ci¹gu w poziomie (np. do poruszania siê w przód/ty³)
	float verticalThrustStrength; // si³a ci¹gu pionowego (np. do unoszenia siê w górê)
    float dragCoefficient;     // opór powietrza
	float maxTilt = 15.0f * (std::numbers::pi_v<float> / 180.0f); // maksymalny k¹t pochylenia w radianach (10 stopni)

    // Aktualizuje tilt w kierunku tiltTarget
    void UpdateTilt(const glm::vec2& tiltInput, float deltaTime);
};

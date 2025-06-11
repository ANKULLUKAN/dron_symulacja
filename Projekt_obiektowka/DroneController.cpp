#include "DroneController.h"

#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>
#include "Object.h"

// Funkcja aktualizuj¹ca tilt drona w zale¿noœci od wejœcia
void DroneController::UpdateTilt(const glm::vec2& tiltInput, const float deltaTime) {
	// Dla ka¿dej osi: jeœli trzymasz klawisz, zwiêkszaj/zmniejszaj tilt, jeœli nie - wracaj do zera
    for (int i = 0; i < 2; ++i) {
	    constexpr float tiltSpeed = 0.07f;
	    if (std::abs(tiltInput[i]) > 0.01f) {
            tilt[i] += tiltInput[i] * tiltSpeed * deltaTime;
        }
        else {
            // Powrót do zera, gdy nie trzymasz klawisza
            if (tilt[i] > 0.0f) {
                tilt[i] -= tiltSpeed * deltaTime;
                if (tilt[i] < 0.0f) tilt[i] = 0.0f;
            }
            else if (tilt[i] < 0.0f) {
                tilt[i] += tiltSpeed * deltaTime;
                if (tilt[i] > 0.0f) tilt[i] = 0.0f;
            }
        }
        // Ogranicz tilt do maxTilt
        if (tilt[i] > maxTilt) tilt[i] = maxTilt;
        if (tilt[i] < -maxTilt) tilt[i] = -maxTilt;
    }
   
}

// G³ówna funkcja fizyki drona
void DroneController::UpdatePhysics(   
    glm::vec3& position,
    glm::vec3& velocity,
    const glm::vec2& tiltInput,
    const float verticalInput,
    const float deltaTime,
    bool& collidedWithGround) {

	// Oblicz ca³kowit¹ masê drona z dodan¹ mas¹ obiektów
    whole_mass = mass + added_mass;

    // Aktualizuj tilt (pochylenie)
    UpdateTilt(tiltInput, deltaTime);

    // Oblicz si³ê ci¹gu w XZ na podstawie tilt
    glm::vec3 thrust(
        std::sin(tilt.y) * thrustStrength, // roll -> X
        verticalInput * verticalThrustStrength,    // pionowo
        std::sin(tilt.x) * thrustStrength  // pitch -> Z
    );
	thrust_y = thrust.y;

    // Opór powietrza (drag)
    const glm::vec3 drag = -velocity * dragCoefficient;

    // Suma si³
    const glm::vec3 force = thrust + drag;

    // Aktualizacja prêdkoœci (F = m*a)
    velocity += (force / whole_mass) * deltaTime;

    // Aktualizacja pozycji
    position += velocity * deltaTime;

    // Minimalny próg prêdkoœci - zatrzymaj drona ca³kowicie, gdy jest bardzo wolny
    if (glm::length(velocity) < 0.0000001f) {
        velocity = glm::vec3(0.0f);
    }

    // Kolizja z pod³og¹ (y = 0)
    if (position.y < 0.0f) {
        position.y = 0.0f;
        velocity.y = 0.0f;
        collidedWithGround = true;
    }
}









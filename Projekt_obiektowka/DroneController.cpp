#include "DroneController.h"

#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>
#include "Object.h"

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



void DroneController::UpdatePhysics(
    glm::vec3& position,
    glm::vec3& velocity,
    const glm::vec2& tiltInput,
    const float verticalInput,
    const float deltaTime,
    bool& collidedWithGround
) {
    
    whole_mass = mass + added_mass;
    

	 // ca³kowita masa drona z dodan¹ mas¹ obiektów

    // 2. Aktualizuj tilt (pochylenie) w kierunku tiltTarget
    UpdateTilt(tiltInput, deltaTime);

    // 3. Oblicz si³ê ci¹gu w XZ na podstawie tilt
    glm::vec3 thrust(
        std::sin(tilt.y) * thrustStrength, // roll -> X
        verticalInput * verticalThrustStrength,    // pionowo
        std::sin(tilt.x) * thrustStrength  // pitch -> Z
    );
	thrusty = thrust.y;

    // 4. Opór powietrza (drag)
    const glm::vec3 drag = -velocity * dragCoefficient;

    // 5. Suma si³
    const glm::vec3 force = thrust + drag;

    // 6. Aktualizacja prêdkoœci (F = m*a)
    velocity += (force / whole_mass) * deltaTime;

    // 7. Aktualizacja pozycji
    position += velocity * deltaTime;

    // 8. Minimalny próg prêdkoœci - zatrzymaj drona ca³kowicie, gdy jest bardzo wolny
    if (glm::length(velocity) < 0.0000001f) {
        velocity = glm::vec3(0.0f);
    }

    // 9. Kolizja z pod³og¹ (y = 0)
    if (position.y < 0.0f) {
        position.y = 0.0f;
        velocity.y = 0.0f;
        collidedWithGround = true;
    }

    
}









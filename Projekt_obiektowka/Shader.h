#pragma once
#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <string>
#include <glm/glm.hpp>

// Klasa Shader - zarz¹dza programem shaderów OpenGL (kompilacja, u¿ycie, ustawianie uniformów)
class Shader {
public:
    GLuint Id; // Identyfikator programu shaderów OpenGL

    // Konstruktor: kompiluje i linkuje shadery z podanych Ÿróde³ (vertex + fragment)
    Shader(const std::string& vertexPath, const std::string& fragmentPath);

    // Destruktor: usuwa program shaderów z OpenGL
    ~Shader();

    // Aktywuje (ustawia jako bie¿¹cy) program shaderów
    void Use() const;

    // Ustawia uniform mat4 o podanej nazwie (np. macierz modelu, widoku, projekcji)
    void SetMat4(const std::string& name, const glm::mat4& mat) const;

    // Ustawia uniform vec4 o podanej nazwie (np. kolor obiektu)
    void SetVec4(const std::string& name, const glm::vec4& value) const;

	// Ustawia uniform vec3 o podanej nazwie (np. wektor normalny, kolor œwiat³a)
    void SetVec3(const std::string& name, const glm::vec3& value) const;

private:
    // Kompiluje pojedynczy shader (vertex lub fragment) z podanego Ÿród³a
    static GLuint CompileShader(GLenum type, const char* source);
    static std::string LoadShaderSource(const std::string& filename);
};

#endif

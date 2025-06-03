#pragma once
#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <string>
#include <glm/glm.hpp>

// Klasa Shader - zarz¹dza programem shaderów OpenGL (kompilacja, u¿ycie, ustawianie uniformów)
class Shader {
public:
    GLuint ID; // Identyfikator programu shaderów OpenGL

    // Konstruktor: kompiluje i linkuje shadery z podanych Ÿróde³ (vertex + fragment)
    Shader(const std::string& vertexPath, const std::string& fragmentPath);

    // Destruktor: usuwa program shaderów z OpenGL
    ~Shader();

    // Aktywuje (ustawia jako bie¿¹cy) program shaderów
    void use() const;

    // Ustawia uniform mat4 o podanej nazwie (np. macierz modelu, widoku, projekcji)
    void setMat4(const std::string& name, const glm::mat4& mat) const;

    // Ustawia uniform vec4 o podanej nazwie (np. kolor obiektu)
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;


private:
    // Kompiluje pojedynczy shader (vertex lub fragment) z podanego Ÿród³a
    GLuint compileShader(GLenum type, const char* source);
    std::string loadShaderSource(const std::string& filename);
};

#endif

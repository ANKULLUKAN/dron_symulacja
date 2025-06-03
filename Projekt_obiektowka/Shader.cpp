#include "Shader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>

// Konstruktor: kompiluje shadery, linkuje program i sprawdza b³êdy
Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    
    std::string vertexCode = loadShaderSource(vertexPath);
    std::string fragmentCode = loadShaderSource(fragmentPath);
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vShaderCode);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fShaderCode);

    ID = glCreateProgram();
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragmentShader);
    glLinkProgram(ID);

    // Sprawdzenie poprawnoœci linkowania programu
    int success;
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(ID, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Usuwanie niepotrzebnych ju¿ shaderów (s¹ w programie)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

// Destruktor: usuwa program shaderów z OpenGL
Shader::~Shader() {
    glDeleteProgram(ID);
}

// Ustawia ten program shaderów jako aktywny
void Shader::use() const {
    glUseProgram(ID);
}

// Ustawia uniform mat4 o podanej nazwie (np. macierz modelu, widoku, projekcji)
void Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

// Ustawia uniform vec4 o podanej nazwie (np. kolor obiektu)
void Shader::setVec4(const std::string& name, const glm::vec4& value) const {
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
}

// Kompiluje pojedynczy shader (vertex lub fragment) i sprawdza b³êdy kompilacji
GLuint Shader::compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    // Sprawdzenie poprawnoœci kompilacji shadera
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    return shader;
}

std::string Shader::loadShaderSource(const std::string& filename) {
    std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

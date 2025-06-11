#include "Shader.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>

// Konstruktor: kompiluje shadery, linkuje program i sprawdza b³êdy
Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    
    std::string vertexCode = LoadShaderSource(vertexPath);
    std::string fragmentCode = LoadShaderSource(fragmentPath);
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vShaderCode);
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fShaderCode);

    Id = glCreateProgram();
    glAttachShader(Id, vertexShader);
    glAttachShader(Id, fragmentShader);
    glLinkProgram(Id);

    // Sprawdzenie poprawnoœci linkowania programu
    int success;
    glGetProgramiv(Id, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(Id, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << '\n';
    }

    // Usuwanie niepotrzebnych ju¿ shaderów (s¹ w programie)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

// Destruktor: usuwa program shaderów z OpenGL
Shader::~Shader() {
    glDeleteProgram(Id);
}

// Ustawia ten program shaderów jako aktywny
void Shader::Use() const {
    glUseProgram(Id);
}

// Ustawia uniform mat4 o podanej nazwie (np. macierz modelu, widoku, projekcji)
void Shader::SetMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(glGetUniformLocation(Id, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

// Ustawia uniform vec4 o podanej nazwie (np. kolor obiektu)
void Shader::SetVec4(const std::string& name, const glm::vec4& value) const {
    glUniform4fv(glGetUniformLocation(Id, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::SetVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(Id, name.c_str()), 1, glm::value_ptr(value));
}

// Kompiluje pojedynczy shader (vertex lub fragment) i sprawdza b³êdy kompilacji
GLuint Shader::CompileShader(const GLenum type, const char* source) {
	const GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    // Sprawdzenie poprawnoœci kompilacji shadera
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << '\n';
    }

    return shader;
}

// £aduje Ÿród³o shadera z pliku i zwraca jako string
std::string Shader::LoadShaderSource(const std::string& filename) {
	const std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

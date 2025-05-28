#include "Shader.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

// Prosty shader wierzcho³ków
const char* vertexSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

// Prosty shader fragmentów z kolorem przekazywanym przez uniform
const char* fragmentSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec4 objectColor;
void main() {
    FragColor = objectColor;
}
)";

// Konstruktor: kompiluje shadery, linkuje program i sprawdza b³êdy
Shader::Shader() {
    // Kompilacja vertex shadera
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    // Kompilacja fragment shadera
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    // Tworzenie programu shaderów i do³¹czanie shaderów
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
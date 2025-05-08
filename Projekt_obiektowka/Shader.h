#pragma once
#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <string>
#include <glm/glm.hpp>

class Shader {
public:
    GLuint ID;

    Shader(const char* vertexSource, const char* fragmentSource);
    ~Shader();

    void use() const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;

private:
    GLuint compileShader(GLenum type, const char* source);
};

#endif

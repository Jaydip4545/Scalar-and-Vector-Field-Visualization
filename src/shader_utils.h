#ifndef SHADER_UTILS_H
#define SHADER_UTILS_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <GL/glew.h>

// Function to load shader source code from a file
std::string loadShaderSource(const std::string& filepath) {
    std::ifstream shaderFile(filepath);
    if (!shaderFile.is_open()) {
        std::cerr << "Error: Could not open shader file: " << filepath << std::endl;
        return "";
    }
    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    shaderFile.close();
    return shaderStream.str();
}

// Function to compile a shader and check for errors
GLuint compileShader(GLuint type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int result;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> message(length);
        glGetShaderInfoLog(shader, length, &length, &message[0]);
        std::cerr << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
        std::cerr << &message[0] << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

// Function to create a shader program
GLuint createShaderProgram(const std::string& vertexShaderPath, const std::string& fragmentShaderPath) {
    std::string vertexShaderSource = loadShaderSource(vertexShaderPath);
    std::string fragmentShaderSource = loadShaderSource(fragmentShaderPath);

    GLuint program = glCreateProgram();
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

#endif // SHADER_UTILS_H

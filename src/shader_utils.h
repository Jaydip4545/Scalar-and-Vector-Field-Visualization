#ifndef SHADER_UTILS_H
#define SHADER_UTILS_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <GL/glew.h>

inline std::string loadShaderSource(const std::string& filepath) {
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

inline GLuint compileShader(GLuint type, const std::string& source, const std::string& type_str) {
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
        std::cerr << "Failed to compile " << type_str << " shader!" << std::endl;
        std::cerr << &message[0] << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

inline bool checkLinkStatus(GLuint program) {
    int result;
    glGetProgramiv(program, GL_LINK_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> message(length);
        glGetProgramInfoLog(program, length, &length, &message[0]);
        std::cerr << "Failed to link shader program!" << std::endl;
        std::cerr << &message[0] << std::endl;
        return false;
    }
    return true;
}

inline GLuint createShaderProgram(const std::string& vs_path, const std::string& fs_path) {
    std::string vs_src = loadShaderSource(vs_path);
    std::string fs_src = loadShaderSource(fs_path);
    GLuint program = glCreateProgram();
    GLuint vs = compileShader(GL_VERTEX_SHADER, vs_src, "vertex");
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fs_src, "fragment");
    if (vs == 0 || fs == 0) return 0;
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    if (!checkLinkStatus(program)) return 0;
    glValidateProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

inline GLuint createShaderProgram(const std::string& vs_path, const std::string& gs_path, const std::string& fs_path) {
    std::string vs_src = loadShaderSource(vs_path);
    std::string gs_src = loadShaderSource(gs_path);
    std::string fs_src = loadShaderSource(fs_path);
    GLuint program = glCreateProgram();
    GLuint vs = compileShader(GL_VERTEX_SHADER, vs_src, "vertex");
    GLuint gs = compileShader(GL_GEOMETRY_SHADER, gs_src, "geometry");
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fs_src, "fragment");
    if (vs == 0 || gs == 0 || fs == 0) return 0;
    glAttachShader(program, vs);
    glAttachShader(program, gs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    if (!checkLinkStatus(program)) return 0;
    glValidateProgram(program);
    glDeleteShader(vs);
    glDeleteShader(gs);
    glDeleteShader(fs);
    return program;
}
#endif // SHADER_UTILS_H

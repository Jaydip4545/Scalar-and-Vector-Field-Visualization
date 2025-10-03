#version 330 core
#extension GL_ARB_shader_storage_buffer_object : require

// The input is a single integer ID for the cube
layout (location = 0) in uint cubeID;

// Pass the ID directly to the geometry shader
flat out uint g_cubeID;

void main() {
    g_cubeID = cubeID;
}

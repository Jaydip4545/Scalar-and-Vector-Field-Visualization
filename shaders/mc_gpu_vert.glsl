#version 330 core

layout (location = 0) in uint cubeID;

flat out uint g_cubeID;

void main() {
    g_cubeID = cubeID;
}

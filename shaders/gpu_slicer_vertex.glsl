#version 330 core
layout (location = 0) in vec3 aPos;
// We accept aTexCoord to match the VAO, but we don't use it.
layout (location = 1) in vec2 aTexCoord;

uniform mat4 mvp;

out vec3 TexCoord3D; 

void main()
{
    gl_Position = mvp * vec4(aPos, 1.0);
    // The vertex position in the [0,1] cube is our 3D texture coordinate
    TexCoord3D = aPos; 
}

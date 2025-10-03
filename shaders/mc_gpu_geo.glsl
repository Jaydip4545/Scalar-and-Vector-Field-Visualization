#version 330 core
layout(points) in;
layout(triangle_strip, max_vertices = 15) out;

flat in uint g_cubeID[];

// UNIFORMS
uniform mat4 mvp;
uniform ivec3 dataDimensions;
uniform float isovalue;
uniform uint totalCubes;

// TEXTURES
uniform sampler3D volumeTexture;
uniform isampler1D edgeTable;
uniform isampler2D triTable;

// OUTPUT TO FRAGMENT SHADER
out vec3 f_color;

// Helper to interpolate vertex positions
vec3 vertexInterp(float isoval, vec3 p1, vec3 p2, float val1, float val2) {
    if (abs(isoval - val1) < 1e-6) return p1;
    if (abs(isoval - val2) < 1e-6) return p2;
    if (abs(val1 - val2) < 1e-6) return p1;
    float mu = (isoval - val1) / (val2 - val1);
    return p1 + mu * (p2 - p1);
}

// *** THE FIX: A lookup array to match the C++ vertex order ***
const ivec3 corner_offsets[8] = ivec3[8](
    ivec3(0, 0, 0), // i = 0
    ivec3(1, 0, 0), // i = 1
    ivec3(1, 1, 0), // i = 2
    ivec3(0, 1, 0), // i = 3
    ivec3(0, 0, 1), // i = 4
    ivec3(1, 0, 1), // i = 5
    ivec3(1, 1, 1), // i = 6
    ivec3(0, 1, 1)  // i = 7
);

void main() {
    int id = int(g_cubeID[0]);
    ivec3 dims_no_border = dataDimensions - 1;

    int x = id % dims_no_border.x;
    int y = (id / dims_no_border.x) % dims_no_border.y;
    int z = id / (dims_no_border.x * dims_no_border.y);
    ivec3 cubePos = ivec3(x, y, z);

    vec3 cornerPos[8];
    float cornerVal[8];
    int cubeindex = 0;
    for (int i = 0; i < 8; i++) {
        // Use the lookup array instead of bitwise logic
        ivec3 offset = corner_offsets[i];
        ivec3 current_pos = cubePos + offset;
        
        cornerVal[i] = texelFetch(volumeTexture, current_pos, 0).r;
        cornerPos[i] = vec3(current_pos);

        if (cornerVal[i] < isovalue) {
            cubeindex |= (1 << i);
        }
    }

    int edges = texelFetch(edgeTable, cubeindex, 0).r;
    if (edges == 0) return;

    vec3 vertlist[12];
    if ((edges & 1)    != 0) vertlist[0] = vertexInterp(isovalue, cornerPos[0], cornerPos[1], cornerVal[0], cornerVal[1]);
    if ((edges & 2)    != 0) vertlist[1] = vertexInterp(isovalue, cornerPos[1], cornerPos[2], cornerVal[1], cornerVal[2]);
    if ((edges & 4)    != 0) vertlist[2] = vertexInterp(isovalue, cornerPos[2], cornerPos[3], cornerVal[2], cornerVal[3]);
    if ((edges & 8)    != 0) vertlist[3] = vertexInterp(isovalue, cornerPos[3], cornerPos[0], cornerVal[3], cornerVal[0]);
    if ((edges & 16)   != 0) vertlist[4] = vertexInterp(isovalue, cornerPos[4], cornerPos[5], cornerVal[4], cornerVal[5]);
    if ((edges & 32)   != 0) vertlist[5] = vertexInterp(isovalue, cornerPos[5], cornerPos[6], cornerVal[5], cornerVal[6]);
    if ((edges & 64)   != 0) vertlist[6] = vertexInterp(isovalue, cornerPos[6], cornerPos[7], cornerVal[6], cornerVal[7]);
    if ((edges & 128)  != 0) vertlist[7] = vertexInterp(isovalue, cornerPos[7], cornerPos[4], cornerVal[7], cornerVal[4]);
    if ((edges & 256)  != 0) vertlist[8] = vertexInterp(isovalue, cornerPos[0], cornerPos[4], cornerVal[0], cornerVal[4]);
    if ((edges & 512)  != 0) vertlist[9] = vertexInterp(isovalue, cornerPos[1], cornerPos[5], cornerVal[1], cornerVal[5]);
    if ((edges & 1024) != 0) vertlist[10] = vertexInterp(isovalue, cornerPos[2], cornerPos[6], cornerVal[2], cornerVal[6]);
    if ((edges & 2048) != 0) vertlist[11] = vertexInterp(isovalue, cornerPos[3], cornerPos[7], cornerVal[3], cornerVal[7]);

    float progress = float(id) / float(totalCubes);
    f_color = vec3(progress, 1.0 - progress, 0.0);

    for (int i = 0; texelFetch(triTable, ivec2(i, cubeindex), 0).r != -1; i += 3) {
        vec3 v1_grid = vertlist[texelFetch(triTable, ivec2(i,     cubeindex), 0).r];
        vec3 v2_grid = vertlist[texelFetch(triTable, ivec2(i + 1, cubeindex), 0).r];
        vec3 v3_grid = vertlist[texelFetch(triTable, ivec2(i + 2, cubeindex), 0).r];

        gl_Position = mvp * vec4(v1_grid / vec3(dims_no_border), 1.0); EmitVertex();
        gl_Position = mvp * vec4(v2_grid / vec3(dims_no_border), 1.0); EmitVertex();
        gl_Position = mvp * vec4(v3_grid / vec3(dims_no_border), 1.0); EmitVertex();
        EndPrimitive();
    }
}

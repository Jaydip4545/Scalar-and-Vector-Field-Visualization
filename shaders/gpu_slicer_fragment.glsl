#version 330 core
out vec4 FragColor;

in vec2 TexCoord; // The 2D coordinate from the vertex shader

uniform sampler3D volumeTexture;
uniform sampler1D colormapTexture;
uniform float minScalar;
uniform float maxScalar;

// New uniforms to tell the shader where the slice is
uniform float sliceNorm;
uniform int slicingAxis; // 0=Z, 1=Y, 2=X

void main()
{
    // 1. Construct the 3D texture coordinate based on the slicing axis
    vec3 texCoord3D;
    if (slicingAxis == 0) { // Z-Slice
        texCoord3D = vec3(TexCoord.x, TexCoord.y, sliceNorm);
    } else if (slicingAxis == 1) { // Y-Slice
        texCoord3D = vec3(TexCoord.x, sliceNorm, TexCoord.y);
    } else { // X-Slice
        texCoord3D = vec3(sliceNorm, TexCoord.x, TexCoord.y);
    }

    // 2. Sample the volume with the newly constructed coordinate
    float scalarValue = texture(volumeTexture, texCoord3D).r;
    
    // 3. Normalize and get color (same as before)
    float normalizedScalar = 0.0;
    if (maxScalar > minScalar) {
        normalizedScalar = (scalarValue - minScalar) / (maxScalar - minScalar);
    }
    FragColor = texture(colormapTexture, normalizedScalar);
}

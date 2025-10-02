#version 330 core
out vec4 FragColor;

// The interpolated 3D position from the vertex shader
in vec3 TexCoord3D; 

// Our two main data sources on the GPU
uniform sampler3D volumeTexture;
uniform sampler1D colormapTexture;

// Min/max values for normalization
uniform float minScalar;
uniform float maxScalar;

void main()
{
    // 1. Sample the 3D volume at the given coordinate to get the scalar value.
    // The '.r' gets the red channel, which is where we stored our single scalar value.
    float scalarValue = texture(volumeTexture, TexCoord3D).r;
    
    // 2. Normalize the scalar value to the [0, 1] range.
    float normalizedScalar = (scalarValue - minScalar) / (maxScalar - minScalar);
    
    // 3. Use the normalized value as a coordinate to look up the color in our 1D colormap.
    FragColor = texture(colormapTexture, normalizedScalar);
}

#ifndef MARCHING_CUBES_H
#define MARCHING_CUBES_H

#include <vector>
#include <glm/glm.hpp>

// A struct to hold a single vertex's data (position and color)
struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
};

class MarchingCubes {
public:

	// The two essential lookup tables for the algorithm
    static const int edgeTable[256];
    static const int triTable[256][16];
    	// Main function to generate the isosurface mesh
    std::vector<Vertex> generateSurface(const std::vector<float>& scalars, 
                                        glm::ivec3 dims, 
                                        float isovalue);

private:
    // Helper function to calculate a vertex's position along an edge
    glm::vec3 vertexInterp(float isovalue, glm::vec3 p1, glm::vec3 p2, float val1, float val2);

    
};

#endif // MARCHING_CUBES_H

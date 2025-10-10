#ifndef VTK_PARSER_H
#define VTK_PARSER_H

#include <string>
#include <vector>
#include <map>
#include <glm/glm.hpp>

class VtkParser {
public:
    VtkParser(const std::string& filepath);

    // Main function to parse the file
    bool read();

    // Accessors
    const glm::ivec3& getDimensions() const { return dimensions; }
    std::string getFirstFieldName() const;
    
    
    // Get a specific scalar field by name
    bool getScalarField(const std::string& fieldName, std::vector<float>& scalars) const;

    // Get a value using trilinear interpolation from a given field
    float getValue(const std::vector<float>& scalars, const glm::vec3& coord) const;
    
    const glm::vec3& getOrigin() const { return origin; }
    const glm::vec3& getSpacing() const { return spacing; }

private:
    std::string filepath;
    glm::ivec3 dimensions;
    glm::vec3 spacing;
    glm::vec3 origin;
    
    // Store multiple named scalar fields
    std::map<std::string, std::vector<float>> scalarFields;
};

#endif // VTK_PARSER_H

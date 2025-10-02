#include "vtk_parser.h"
#include <iostream>
#include <fstream>
#include <sstream>

VtkParser::VtkParser(const std::string& path) 
    : filepath(path), dimensions(0), spacing(1.0f), origin(0.0f) {}

bool VtkParser::read() {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open VTK file: " << filepath << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string keyword;
        ss >> keyword;

        if (keyword == "DIMENSIONS") {
            ss >> dimensions.x >> dimensions.y >> dimensions.z;
        } else if (keyword == "SPACING") {
            ss >> spacing.x >> spacing.y >> spacing.z;
        } else if (keyword == "ORIGIN") {
            ss >> origin.x >> origin.y >> origin.z;
        } else if (keyword == "POINT_DATA") {
            long long totalPoints;
            ss >> totalPoints;

            // Check consistency with dimensions
            if (totalPoints != (long long)dimensions.x * dimensions.y * dimensions.z) {
                std::cerr << "Parser Error: POINT_DATA count (" << totalPoints 
                          << ") does not match dimensions count (" 
                          << (long long)dimensions.x * dimensions.y * dimensions.z << ")." << std::endl;
                return false;
            }

            // Next line should be FIELD
            if (std::getline(file, line)) {
                std::stringstream field_ss(line);
                std::string field_keyword, field_data_keyword;
                int num_fields;
                field_ss >> field_keyword >> field_data_keyword >> num_fields;

                if (field_keyword != "FIELD") continue;

                // Loop to read each named field
                for (int i = 0; i < num_fields; ++i) {
                    if (std::getline(file, line)) {
                        std::stringstream header_ss(line);
                        std::string field_name, data_type;
                        int num_components;
                        long long num_tuples;
                        
                        header_ss >> field_name >> num_components >> num_tuples >> data_type;

                        if (num_tuples != totalPoints) {
                            std::cerr << "Parser Error: Field '" << field_name << "' tuple count mismatch." << std::endl;
                            return false;
                        }

                        // Read the actual data for this field
                        std::vector<float> data(totalPoints);
                        for (long long j = 0; j < totalPoints; ++j) {
                            double val;
                            if (!(file >> val)) {
                                std::cerr << "Parser Error: Failed reading value #" << j << " for field '" << field_name << "'." << std::endl;
                                return false;
                            }
                            data[j] = static_cast<float>(val);
                        }
                        scalarFields[field_name] = data;
                        std::cout << "Successfully read field: " << field_name << std::endl;
                    }
                }
                return true; // Finished parsing all fields
            }
        }
    }
    return false; // Reached end of file without finding POINT_DATA
}

bool VtkParser::getScalarField(const std::string& fieldName, std::vector<float>& scalars) const {
    auto it = scalarFields.find(fieldName);
    if (it != scalarFields.end()) {
        scalars = it->second;
        return true;
    }
    std::cerr << "Error: Field '" << fieldName << "' not found in VTK file." << std::endl;
    return false;
}

float VtkParser::getValue(const std::vector<float>& scalars, const glm::vec3& coord) const {
    float x = glm::clamp(coord.x, 0.0f, (float)dimensions.x - 1.001f);
    float y = glm::clamp(coord.y, 0.0f, (float)dimensions.y - 1.001f);
    float z = glm::clamp(coord.z, 0.0f, (float)dimensions.z - 1.001f);

    int x0 = (int)x, y0 = (int)y, z0 = (int)z;
    int x1 = x0 + 1, y1 = y0 + 1, z1 = z0 + 1;

    float xd = x - x0, yd = y - y0, zd = z - z0;

    auto get_val = [&](int i, int j, int k) {
        return scalars[k * dimensions.x * dimensions.y + j * dimensions.x + i];
    };

    float c00 = get_val(x0, y0, z0) * (1 - xd) + get_val(x1, y0, z0) * xd;
    float c01 = get_val(x0, y0, z1) * (1 - xd) + get_val(x1, y0, z1) * xd;
    float c10 = get_val(x0, y1, z0) * (1 - xd) + get_val(x1, y1, z0) * xd;
    float c11 = get_val(x0, y1, z1) * (1 - xd) + get_val(x1, y1, z1) * xd;

    float c0 = c00 * (1 - yd) + c10 * yd;
    float c1 = c01 * (1 - yd) + c11 * yd;

    return c0 * (1 - zd) + c1 * zd;
}

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "vtk_parser.h"
#include "shader_utils.h"
#include "marching_cubes.h"

// --- Globals & Callbacks ---
Camera camera(800, 600);
int slicingAxis = 0; // 0=Z, 1=Y, 2=X
bool showIsosurface = false;
bool useGpuSlicing = true; // Start with the GPU version by default
GLuint sliceTexture; 
bool useGpuMarchingCubes = false;

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    cam->mouseButtonCallback(window, button, action, mods);
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    cam->cursorPosCallback(window, xpos, ypos);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    cam->scrollCallback(window, xoffset, yoffset);
}
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if(key == GLFW_KEY_C)
        {
		slicingAxis = (slicingAxis + 1) % 3; // Cycle through 0, 1, 2
		if (slicingAxis == 0) std::cout << "Slicing along Z-axis" << std::endl;
		if (slicingAxis == 1) std::cout << "Slicing along Y-axis" << std::endl;
		if (slicingAxis == 2) std::cout << "Slicing along X-axis" << std::endl;
        }
        if (key == GLFW_KEY_G) {
            useGpuSlicing = !useGpuSlicing;
            if (useGpuSlicing) {
                std::cout << "Switched to GPU Slicing" << std::endl;
            } else {
                std::cout << "Switched to CPU Slicing" << std::endl;
            }
        }
        if (key == GLFW_KEY_M) {
            showIsosurface = !showIsosurface;
            std::cout << "Switched to " << (showIsosurface ? "Isosurface View" : "Slicer View") << std::endl;
        }
        if (key == GLFW_KEY_H) {
            useGpuMarchingCubes = !useGpuMarchingCubes;
            std::cout << "Switched to " << (useGpuMarchingCubes ? "useGpuMarchingCubes " : "useCpuMarchingCubes") << std::endl;
        }
    }
}



void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

const std::vector<glm::vec3> viridis_colormap = {
    {0.267004f, 0.004874f, 0.329415f}, {0.268565f, 0.009605f, 0.335427f}, {0.270126f, 0.014625f, 0.341379f}, {0.271687f, 0.019942f, 0.347269f}, {0.273248f, 0.025563f, 0.353093f}, {0.274809f, 0.031497f, 0.35885f}, {0.27637f, 0.037749f, 0.364537f}, {0.277931f, 0.0441f, 0.370154f}, {0.279492f, 0.0503f, 0.375702f}, {0.281053f, 0.056399f, 0.38118f}, {0.282614f, 0.062408f, 0.386589f}, {0.284175f, 0.068337f, 0.391929f}, {0.285736f, 0.074196f, 0.397199f}, {0.287297f, 0.079994f, 0.402401f}, {0.288858f, 0.085741f, 0.407534f}, {0.290419f, 0.091448f, 0.4126f}, {0.29198f, 0.097123f, 0.417598f}, {0.293541f, 0.102776f, 0.422529f}, {0.295102f, 0.108415f, 0.427394f}, {0.296663f, 0.114048f, 0.432192f}, {0.298224f, 0.11968f, 0.436924f}, {0.299785f, 0.125319f, 0.441589f}, {0.301346f, 0.13097f, 0.446188f}, {0.302907f, 0.136639f, 0.45072f}, {0.304468f, 0.142331f, 0.455186f}, {0.306029f, 0.148051f, 0.459585f}, {0.30759f, 0.153803f, 0.463918f}, {0.309151f, 0.159591f, 0.468184f}, {0.310712f, 0.165419f, 0.472384f}, {0.312273f, 0.17129f, 0.476517f}, {0.313834f, 0.177207f, 0.480584f}, {0.315395f, 0.183172f, 0.484585f}, {0.316956f, 0.189185f, 0.488519f}, {0.318517f, 0.195244f, 0.492387f}, {0.320078f, 0.201347f, 0.496188f}, {0.321639f, 0.207491f, 0.499923f}, {0.3232f, 0.213674f, 0.503592f}, {0.324761f, 0.219894f, 0.507194f}, {0.326322f, 0.226149f, 0.51073f}, {0.327883f, 0.232435f, 0.5142f}, {0.329444f, 0.238752f, 0.517604f}, {0.331005f, 0.245096f, 0.520942f}, {0.332566f, 0.251466f, 0.524214f}, {0.334127f, 0.25786f, 0.52742f}, {0.335688f, 0.264276f, 0.53056f}, {0.337249f, 0.270711f, 0.533635f}, {0.33881f, 0.277165f, 0.536645f}, {0.340371f, 0.283634f, 0.539589f}, {0.341932f, 0.290118f, 0.542468f}, {0.343493f, 0.296614f, 0.545283f}, {0.345054f, 0.303121f, 0.548033f}, {0.346615f, 0.309637f, 0.550718f}, {0.348176f, 0.31616f, 0.553339f}, {0.349737f, 0.32269f, 0.555897f}, {0.351298f, 0.329225f, 0.55839f}, {0.352859f, 0.335763f, 0.56082f}, {0.35442f, 0.342304f, 0.563186f}, {0.355981f, 0.348846f, 0.565489f}, {0.357542f, 0.355388f, 0.567728f}, {0.359103f, 0.361929f, 0.569904f}, {0.360664f, 0.368469f, 0.572017f}, {0.362225f, 0.375005f, 0.574067f}, {0.363786f, 0.381538f, 0.576054f}, {0.365347f, 0.388066f, 0.577979f}, {0.366908f, 0.394589f, 0.57984f}, {0.368469f, 0.401105f, 0.581639f}, {0.37003f, 0.407614f, 0.583376f}, {0.371591f, 0.414115f, 0.58505f}, {0.373152f, 0.420608f, 0.586662f}, {0.374713f, 0.427091f, 0.588212f}, {0.376274f, 0.433564f, 0.5897f}, {0.377835f, 0.440026f, 0.591127f}, {0.379396f, 0.446478f, 0.592492f}, {0.380957f, 0.452918f, 0.593796f}, {0.382518f, 0.459346f, 0.595039f}, {0.384079f, 0.465762f, 0.596221f}, {0.38564f, 0.472165f, 0.597343f}, {0.387201f, 0.478555f, 0.598404f}, {0.388762f, 0.484932f, 0.599404f}, {0.390323f, 0.491295f, 0.600344f}, {0.391884f, 0.497645f, 0.601224f}, {0.393445f, 0.50398f, 0.602044f}, {0.395006f, 0.5103f, 0.602804f}, {0.396567f, 0.516606f, 0.603505f}, {0.398128f, 0.522896f, 0.604147f}, {0.399689f, 0.529171f, 0.604729f}, {0.40125f, 0.53543f, 0.605252f}, {0.402811f, 0.541673f, 0.605716f}, {0.404372f, 0.5479f, 0.606121f}, {0.405933f, 0.55411f, 0.606467f}, {0.407494f, 0.560304f, 0.606754f}, {0.409055f, 0.566481f, 0.606983f}, {0.410616f, 0.572641f, 0.607153f}, {0.412177f, 0.578784f, 0.607264f}, {0.413738f, 0.58491f, 0.607316f}, {0.415299f, 0.591018f, 0.60731f}, {0.41686f, 0.597109f, 0.607245f}, {0.418421f, 0.603183f, 0.607122f}, {0.419982f, 0.609239f, 0.60694f}, {0.421543f, 0.615277f, 0.606699f}, {0.423104f, 0.621298f, 0.606401f}, {0.424665f, 0.6273f, 0.606045f}, {0.426226f, 0.633285f, 0.605631f}, {0.427787f, 0.639252f, 0.605159f}, {0.429348f, 0.6452f, 0.604629f}, {0.430909f, 0.65113f, 0.604042f}, {0.43247f, 0.657041f, 0.603397f}, {0.434031f, 0.662933f, 0.602695f}, {0.435592f, 0.668806f, 0.599728f}, {0.437153f, 0.67466f, 0.59392f}, {0.438714f, 0.680494f, 0.588019f}, {0.440275f, 0.686309f, 0.582026f}, {0.441836f, 0.692105f, 0.57594f}, {0.443397f, 0.697881f, 0.569762f}, {0.444958f, 0.703638f, 0.563493f}, {0.446519f, 0.709375f, 0.557133f}, {0.44808f, 0.715093f, 0.550682f}, {0.449641f, 0.720791f, 0.544141f}, {0.451202f, 0.72647f, 0.53751f}, {0.452763f, 0.732129f, 0.53079f}, {0.454324f, 0.737768f, 0.523981f}, {0.455885f, 0.743388f, 0.517083f}, {0.457446f, 0.748987f, 0.510097f}, {0.459007f, 0.754567f, 0.503024f}, {0.460568f, 0.760127f, 0.495863f}, {0.462129f, 0.765668f, 0.488616f}, {0.46369f, 0.771188f, 0.481283f}, {0.465251f, 0.776689f, 0.473865f}, {0.466812f, 0.78217f, 0.466362f}, {0.468373f, 0.78763f, 0.458774f}, {0.469934f, 0.79307f, 0.451103f}, {0.471495f, 0.79849f, 0.443348f}, {0.473056f, 0.803888f, 0.43551f}, {0.474617f, 0.809267f, 0.42759f}, {0.476178f, 0.814624f, 0.419589f}, {0.477739f, 0.81996f, 0.411508f}, {0.4793f, 0.825275f, 0.403348f}, {0.480861f, 0.830569f, 0.395109f}, {0.482422f, 0.835843f, 0.386793f}, {0.483983f, 0.841095f, 0.3784f}, {0.485544f, 0.846327f, 0.369931f}, {0.487105f, 0.851538f, 0.361387f}, {0.488666f, 0.856728f, 0.352769f}, {0.490227f, 0.861898f, 0.344078f}, {0.491788f, 0.867047f, 0.335314f}, {0.493349f, 0.872175f, 0.326478f}, {0.49491f, 0.877283f, 0.317571f}, {0.496471f, 0.88237f, 0.308593f}, {0.498032f, 0.887436f, 0.299546f}, {0.499593f, 0.892482f, 0.29043f}, {0.501154f, 0.897507f, 0.281246f}, {0.502715f, 0.902511f, 0.271994f}, {0.504276f, 0.907495f, 0.262676f}, {0.505837f, 0.912458f, 0.253293f}, {0.507398f, 0.917399f, 0.243846f}, {0.508959f, 0.92232f, 0.234336f}, {0.51052f, 0.927219f, 0.224764f}, {0.512081f, 0.932098f, 0.215132f}, {0.513642f, 0.936955f, 0.20544f}, {0.515203f, 0.941791f, 0.19569f}, {0.516764f, 0.946606f, 0.185883f}, {0.518325f, 0.951399f, 0.17602f}, {0.519886f, 0.956171f, 0.166102f}, {0.521447f, 0.960922f, 0.156129f}, {0.523008f, 0.965651f, 0.146104f}, {0.524569f, 0.970359f, 0.136028f}, {0.52613f, 0.975046f, 0.125902f}, {0.527691f, 0.979712f, 0.115728f}, {0.529252f, 0.984357f, 0.105506f}, {0.530813f, 0.988981f, 0.095238f}, {0.532374f, 0.993584f, 0.084924f}, {0.561498f, 0.992055f, 0.085822f}, {0.590622f, 0.990526f, 0.086719f}, {0.619746f, 0.988997f, 0.087617f}, {0.64887f, 0.987468f, 0.088514f}, {0.677994f, 0.985939f, 0.089412f}, {0.707118f, 0.98441f, 0.09031f}, {0.736242f, 0.982881f, 0.091207f}, {0.765366f, 0.981352f, 0.092105f}, {0.79449f, 0.979823f, 0.093002f}, {0.823614f, 0.978294f, 0.0939f}, {0.852738f, 0.976765f, 0.094798f}, {0.881862f, 0.975236f, 0.095695f}, {0.910986f, 0.973707f, 0.096593f}, {0.94011f, 0.972178f, 0.09749f}, {0.969234f, 0.970649f, 0.098388f}, {0.993248f, 0.906157f, 0.143936f}
};
// --- Colormap Function ---
glm::vec3 getColor(float value, float min_val, float max_val) {
    // Define the key colors
    glm::vec3 color_blue(0.0f, 0.0f, 1.0f);   // Coolest
    glm::vec3 color_white(1.0f, 1.0f, 1.0f);  // Midpoint
    glm::vec3 color_red(1.0f, 0.0f, 0.0f);     // Hottest

    // 1. Normalize the value to the range [0, 1]
    float normalized = (value - min_val) / (max_val - min_val);
    normalized = glm::clamp(normalized, 0.0f, 1.0f);

    // 2. Interpolate based on which half the value is in
    if (normalized < 0.5f) {
        // Interpolate from blue to white (for the lower half)
        // We need to remap the [0, 0.5] range to [0, 1] for glm::mix
        return glm::mix(color_blue, color_white, normalized * 2.0f);
    } else {
        // Interpolate from white to red (for the upper half)
        // We need to remap the [0.5, 1] range to [0, 1] for glm::mix
        return glm::mix(color_white, color_red, (normalized - 0.5f) * 2.0f);
    }
}

int main(int argc, char* argv[]) {



	
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_vtk_file> [optional_field_name]" << std::endl;
        std::cerr << "Example: " << argv[0] << " resources/redseaT.vtk TEMP" << std::endl;
        return 1;
    }

    std::string vtk_filepath = argv[1];

    // --- GLFW/GLEW Initialization ---
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Visualizer", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) return -1;
    
    // --- Set Callbacks ---
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetWindowUserPointer(window, &camera);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);

    // --- Load Data ---
    VtkParser parser(vtk_filepath);
    if (!parser.read()) return -1;
    std::vector<float> scalars;
    std::string fieldName;
    
    if (argc > 2) {
        // A field name was provided as the second argument
        fieldName = argv[2];
        std::cout << "Attempting to visualize user-specified field: " << fieldName << std::endl;
    } else {
        // No field name provided, get the first one from the file
        fieldName = parser.getFirstFieldName();
        if (fieldName.empty()) {
            std::cerr << "Error: No scalar fields found in the VTK file." << std::endl;
            return -1;
        }
        std::cout << "No field specified. Visualizing first available field: " << fieldName << std::endl;
    }
    
    
    
    if (!parser.getScalarField(fieldName, scalars)) return -1;
    
    glm::ivec3 dims = parser.getDimensions();
    glm::vec3 spacing = parser.getSpacing();
    glm::vec3 size = glm::vec3(dims - glm::ivec3(1)) * spacing;
    float min_scalar = *std::min_element(scalars.begin(), scalars.end());
    float max_scalar = *std::max_element(scalars.begin(), scalars.end());

    // --- Auto-fit Camera (now using the true size) ---

    float radius = glm::length(size) * 0.5f;

    float fov_radians = glm::radians(45.0f);

    float distance = radius / tan(fov_radians / 2.0f);

    camera.setZoom(distance * 1.5f); // Use new distance 
    
    // --- Shaders ---
    GLuint textureShader = createShaderProgram("shaders/texture_vertex.glsl", "shaders/texture_fragment.glsl");
    GLuint flatColorShader = createShaderProgram("shaders/flat_color_vertex.glsl", "shaders/flat_color_fragment.glsl");
    GLuint gpuSlicerShader = createShaderProgram("shaders/gpu_slicer_vertex.glsl", "shaders/gpu_slicer_fragment.glsl");

    // --- Geometry ---
    float box_vertices[] = {0,0,0, 1,0,0, 1,0,0, 1,1,0, 1,1,0, 0,1,0, 0,1,0, 0,0,0, 0,0,1, 1,0,1, 1,0,1, 1,1,1, 1,1,1, 0,1,1, 0,1,1, 0,0,1, 0,0,0, 0,0,1, 1,0,0, 1,0,1, 1,1,0, 1,1,1, 0,1,0, 0,1,1};
    GLuint boxVAO, boxVBO;
    glGenVertexArrays(1, &boxVAO); glGenBuffers(1, &boxVBO);
    glBindVertexArray(boxVAO); glBindBuffer(GL_ARRAY_BUFFER, boxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(box_vertices), box_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    float axis_vertices[] = {0,0,0, 1,0,0, 0,0,0, 0,1,0, 0,0,0, 0,0,1};
    GLuint axisVAO_g, axisVBO_g; // Use unique names to avoid bug
    glGenVertexArrays(1, &axisVAO_g); glGenBuffers(1, &axisVBO_g);
    glBindVertexArray(axisVAO_g); glBindBuffer(GL_ARRAY_BUFFER, axisVBO_g);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axis_vertices), axis_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    float quad_vertices[] = {0,0,0, 0,0, 1,0,0, 1,0, 1,1,0, 1,1, 0,1,0, 0,1};
    unsigned int quad_indices[] = {0, 1, 2, 2, 3, 0};
    GLuint quadVAO, quadVBO, quadEBO;
    glGenVertexArrays(1, &quadVAO); glGenBuffers(1, &quadVBO); glGenBuffers(1, &quadEBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO); glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO); glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_indices), quad_indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
    
    // --- CPU Slicing Resources ---
    glGenTextures(1, &sliceTexture);
    glBindTexture(GL_TEXTURE_2D, sliceTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    std::vector<unsigned char> textureData;

    // --- GPU Slicing Resources ---
    GLuint volumeTexture;
    glGenTextures(1, &volumeTexture);
    glBindTexture(GL_TEXTURE_3D, volumeTexture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, dims.x, dims.y, dims.z, 0, GL_RED, GL_FLOAT, scalars.data());
    
    GLuint colormapTexture;
    glGenTextures(1, &colormapTexture);
    glBindTexture(GL_TEXTURE_1D, colormapTexture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    std::vector<glm::vec3> colormap_data;
    for(int i = 0; i < 256; ++i) colormap_data.push_back(getColor((float)i, 0.0f, 255.0f));
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, colormap_data.size(), 0, GL_RGB, GL_FLOAT, colormap_data.data());
    
    
     // --- Marching Cubes Setup ---
    MarchingCubes mc;
    GLuint isoVAO, isoVBO;
    glGenVertexArrays(1, &isoVAO);
    glGenBuffers(1, &isoVBO);
    glBindVertexArray(isoVAO);
    glBindBuffer(GL_ARRAY_BUFFER, isoVBO);
    // Setup vertex attributes for our Vertex struct (pos and color)
    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(0);
    // Color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);

    // Load the new shader for the isosurface
    GLuint vertexColorShader = createShaderProgram("shaders/mc_cpu_vert.glsl", "shaders/mc_cpu_frag.glsl");
    
    
    
    //GPU MC setup
    	 
    GLuint mcGpuShader = createShaderProgram("shaders/mc_gpu_vert.glsl", "shaders/mc_gpu_geo.glsl", "shaders/mc_gpu_frag.glsl");
    
    GLuint edgeTableTexture, triTableTexture;
    glGenTextures(1, &edgeTableTexture);
    glBindTexture(GL_TEXTURE_1D, edgeTableTexture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_R32I, 256, 0, GL_RED_INTEGER, GL_INT, &MarchingCubes::edgeTable[0]);

    glGenTextures(1, &triTableTexture);
    glBindTexture(GL_TEXTURE_2D, triTableTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, 16, 256, 0, GL_RED_INTEGER, GL_INT, &MarchingCubes::triTable[0][0]);

    // Create a VBO containing the ID of every cube for the GPU to process
    GLuint numCubes = (dims.x - 1) * (dims.y - 1) * (dims.z - 1);
    std::vector<unsigned int> cubeIDs(numCubes);
    for (unsigned int i = 0; i < numCubes; ++i) {
        cubeIDs[i] = i;
    }
    
    GLuint mcGpuVAO, mcGpuVBO;
    glGenVertexArrays(1, &mcGpuVAO);
    glGenBuffers(1, &mcGpuVBO);
    glBindVertexArray(mcGpuVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mcGpuVBO);
    glBufferData(GL_ARRAY_BUFFER, cubeIDs.size() * sizeof(unsigned int), cubeIDs.data(), GL_STATIC_DRAW);
    
    // Use glVertexAttribIPointer for integer vertex attributes (the 'I' is important)
    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, 0, (void*)0);
    
    
    
    
    

    // --- Main Loop ---
    glEnable(GL_DEPTH_TEST);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    double lastTime = glfwGetTime();
    int frameCount = 0;

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- Common Calculations ---
        float slice_norm = (sin(glfwGetTime() * 0.5f) * 0.5f + 0.5f);
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        if (height == 0) height = 1;
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 2000.0f);
        glm::mat4 model = glm::translate(glm::mat4(1.0f), -size / 2.0f) * glm::scale(glm::mat4(1.0f), size);

        glm::mat4 slice_pos_model = glm::mat4(1.0f), slice_rotation_model = glm::mat4(1.0f);
        switch(slicingAxis){
            case 0: slice_pos_model = glm::translate(slice_pos_model, glm::vec3(0.5f, 0.5f, slice_norm)); break;
            case 1: slice_rotation_model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); slice_pos_model = glm::translate(slice_pos_model, glm::vec3(0.5f, slice_norm, 0.5f)); break;
            case 2: slice_rotation_model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); slice_pos_model = glm::translate(slice_pos_model, glm::vec3(slice_norm, 0.5f, 0.5f)); break;
        }
        glm::mat4 quad_center_model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, -0.5f, 0.0f));
        glm::mat4 slice_mvp = projection * view * model * slice_pos_model * slice_rotation_model * quad_center_model;
        
        // --- Draw Bounding Box & Axis ---
        glUseProgram(flatColorShader);
        glm::mat4 box_mvp = projection * view * model;
        glUniformMatrix4fv(glGetUniformLocation(flatColorShader, "mvp"), 1, GL_FALSE, glm::value_ptr(box_mvp));
        glUniform3f(glGetUniformLocation(flatColorShader, "ourColor"), 1.0f, 1.0f, 1.0f);
        glBindVertexArray(boxVAO);
        glDrawArrays(GL_LINES, 0, 24);
        
        glBindVertexArray(axisVAO_g);
        glm::mat4 axis_model = glm::scale(model, glm::vec3(1.1f));
        glm::mat4 axis_mvp = projection * view * axis_model;
        glUniformMatrix4fv(glGetUniformLocation(flatColorShader, "mvp"), 1, GL_FALSE, glm::value_ptr(axis_mvp));
        glUniform3f(glGetUniformLocation(flatColorShader, "ourColor"), 1.0f, 0.0f, 0.0f); glDrawArrays(GL_LINES, 0, 2);
        glUniform3f(glGetUniformLocation(flatColorShader, "ourColor"), 0.0f, 1.0f, 0.0f); glDrawArrays(GL_LINES, 2, 2);
        glUniform3f(glGetUniformLocation(flatColorShader, "ourColor"), 0.0f, 0.0f, 1.0f); glDrawArrays(GL_LINES, 4, 2);
	if (showIsosurface) {
        
        // Animate the isovalue from min to max and back
        float isovalue_norm = (sin(glfwGetTime() * 0.5f) * 0.5f + 0.5f);
        float isovalue = min_scalar + isovalue_norm * (max_scalar - min_scalar);
        
        
        
        if (useGpuMarchingCubes) {
            // --- GPU ISOSURFACE RENDER PATH ---
            glUseProgram(mcGpuShader);
            glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_3D, volumeTexture);
            glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_1D, edgeTableTexture);
            glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, triTableTexture);
            glUniformMatrix4fv(glGetUniformLocation(mcGpuShader, "mvp"), 1, GL_FALSE, glm::value_ptr(box_mvp));
            glUniform1i(glGetUniformLocation(mcGpuShader, "volumeTexture"), 0);
            glUniform1i(glGetUniformLocation(mcGpuShader, "edgeTable"), 1);
            glUniform1i(glGetUniformLocation(mcGpuShader, "triTable"), 2);
            glUniform1f(glGetUniformLocation(mcGpuShader, "isovalue"), isovalue);
            glUniform3iv(glGetUniformLocation(mcGpuShader, "dataDimensions"), 1, glm::value_ptr(dims));
            glUniform1ui(glGetUniformLocation(mcGpuShader, "totalCubes"), numCubes);
            glBindVertexArray(mcGpuVAO);
            glDrawArrays(GL_POINTS, 0, numCubes);
        }
        else
        {
        	
        	// Generate the mesh for the current isovalue
        std::vector<Vertex> iso_vertices = mc.generateSurface(scalars, dims, isovalue);
        // *** ADD THIS DEBUG LINE ***
        std::cout << "Isovalue: " << isovalue << "  |  Generated Vertices: " << iso_vertices.size() << "\r";

		if (!iso_vertices.empty()) {
            
            // *** THE CORRECT ORDER ***

            // 1. Bind the VAO for the isosurface FIRST.
            // This activates the entire "preset" for our isosurface mesh.
            glBindVertexArray(isoVAO);

            // 2. Bind the VBO associated with this VAO.
            glBindBuffer(GL_ARRAY_BUFFER, isoVBO);

            // 3. Now, upload the data to the currently bound VBO.
            glBufferData(GL_ARRAY_BUFFER, iso_vertices.size() * sizeof(Vertex), iso_vertices.data(), GL_DYNAMIC_DRAW);

            // 4. Use the correct shader and set its uniforms.
            glUseProgram(vertexColorShader);
            glm::mat4 iso_mvp = projection * view * model;
            glUniformMatrix4fv(glGetUniformLocation(vertexColorShader, "mvp"), 1, GL_FALSE, glm::value_ptr(iso_mvp));
            
            // 5. Draw the mesh. The correct VAO is already bound.
            glDrawArrays(GL_TRIANGLES, 0, iso_vertices.size());
       	 }
        
        
        
        
        }
        
        
        
        
        
        
        
        

    	} 
    	else
    	{
    
    		// --- Conditional Rendering ---
        if (useGpuSlicing) {
            // --- GPU RENDER PATH ---
            glUseProgram(gpuSlicerShader);
            glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_3D, volumeTexture);
            glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_1D, colormapTexture);
            
            // Set all uniforms
            glUniform1i(glGetUniformLocation(gpuSlicerShader, "volumeTexture"), 0);
            glUniform1i(glGetUniformLocation(gpuSlicerShader, "colormapTexture"), 1);
            glUniform1f(glGetUniformLocation(gpuSlicerShader, "minScalar"), min_scalar);
            glUniform1f(glGetUniformLocation(gpuSlicerShader, "maxScalar"), max_scalar);
            
            // Send the two NEW uniforms
            glUniform1f(glGetUniformLocation(gpuSlicerShader, "sliceNorm"), slice_norm);
            glUniform1i(glGetUniformLocation(gpuSlicerShader, "slicingAxis"), slicingAxis);

            // Send the MVP matrix (this is now the same for both shaders)
            glUniformMatrix4fv(glGetUniformLocation(gpuSlicerShader, "mvp"), 1, GL_FALSE, glm::value_ptr(slice_mvp));
        } else {
            int texWidth, texHeight;
            switch(slicingAxis){
                case 0: texWidth = dims.x; texHeight = dims.y; textureData.resize(texWidth * texHeight * 3); for(int y=0;y<texHeight;++y)for(int x=0;x<texWidth;++x){ float v = parser.getValue(scalars,glm::vec3(x,y,slice_norm*(dims.z-1.f))); glm::vec3 c=getColor(v,min_scalar,max_scalar); int i=(y*texWidth+x)*3; textureData[i]=c.r*255; textureData[i+1]=c.g*255; textureData[i+2]=c.b*255; } break;
                case 1: texWidth = dims.x; texHeight = dims.z; textureData.resize(texWidth * texHeight * 3); for(int z=0;z<texHeight;++z)for(int x=0;x<texWidth;++x){ float v = parser.getValue(scalars,glm::vec3(x,slice_norm*(dims.y-1.f),z)); glm::vec3 c=getColor(v,min_scalar,max_scalar); int i=(z*texWidth+x)*3; textureData[i]=c.r*255; textureData[i+1]=c.g*255; textureData[i+2]=c.b*255; } break;
                case 2: texWidth = dims.y; texHeight = dims.z; textureData.resize(texWidth * texHeight * 3); for(int z=0;z<texHeight;++z)for(int y=0;y<texWidth;++y){ float v = parser.getValue(scalars,glm::vec3(slice_norm*(dims.x-1.f),y,z)); glm::vec3 c=getColor(v,min_scalar,max_scalar); int i=(z*texWidth+y)*3; textureData[i]=c.r*255; textureData[i+1]=c.g*255; textureData[i+2]=c.b*255; } break;
            }
	    glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, sliceTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData.data());
            glUseProgram(textureShader);
            glUniform1i(glGetUniformLocation(textureShader, "ourTexture"), 0);
            glUniformMatrix4fv(glGetUniformLocation(textureShader, "mvp"), 1, GL_FALSE, glm::value_ptr(slice_mvp));
        }
        
        glBindVertexArray(quadVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    
    
    
    	}
        

        // --- FPS Counter ---
        double currentTime = glfwGetTime();
        frameCount++;
        if (currentTime - lastTime >= 1.0) {
            std::stringstream ss;
            
            if(showIsosurface)
            {
            	ss << "Field Visualizer | " << (useGpuMarchingCubes ? "GPU" : "CPU") << " | FPS: " << frameCount;
            }
            else
            {
            	ss << "Field Visualizer | " << (useGpuSlicing ? "GPU" : "CPU") << " | FPS: " << frameCount;
            }
            glfwSetWindowTitle(window, ss.str().c_str());
            frameCount = 0;								
            lastTime = currentTime;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // --- Cleanup ---
    glDeleteVertexArrays(1, &isoVAO);
    glDeleteBuffers(1, &isoVBO);
    glDeleteProgram(vertexColorShader);
    glDeleteVertexArrays(1, &boxVAO); glDeleteBuffers(1, &boxVBO);
    glDeleteVertexArrays(1, &axisVAO_g); glDeleteBuffers(1, &axisVBO_g);
    glDeleteVertexArrays(1, &quadVAO); glDeleteBuffers(1, &quadVBO); glDeleteBuffers(1, &quadEBO);
    glDeleteProgram(textureShader); glDeleteProgram(flatColorShader); glDeleteProgram(gpuSlicerShader);
    glDeleteTextures(1, &sliceTexture); glDeleteTextures(1, &volumeTexture); glDeleteTextures(1, &colormapTexture);
    glDeleteProgram(mcGpuShader);
    glDeleteTextures(1, &edgeTableTexture);
    glDeleteTextures(1, &triTableTexture);
    glDeleteVertexArrays(1, &mcGpuVAO);
    glDeleteBuffers(1, &mcGpuVBO);
    
    
    glfwTerminate();
    return 0;
}

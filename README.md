# A Scientific Visualization Toolkit

Field Visualizer is a real-time visualization tool for exploring 3D scalar field data. It is built with modern OpenGL and C++, featuring high-performance, GPU-accelerated implementations of common scientific visualization techniques.

---

## Features

### Visualization Techniques
* **Scalar Field Slicing**
    * Renders an animated, axis-parallel slicing plane that sweeps through the volumetric data.
    * Features a high-performance **GPU-based** implementation using 3D textures for the volume and a 1D texture for the colormap.
    * Includes a **CPU-based** implementation for direct performance comparison.
    * Allows for dynamic cycling between X, Y, and Z slicing axes.

* **Isosurface Extraction**
    * Extracts and renders 3D isosurfaces using the Marching Cubes algorithm.
    * Animates the surface by smoothly varying the isovalue across the dataset's entire scalar range.
    * Features a high-performance **GPU-based** implementation that offloads the entire algorithm to a **Geometry Shader**.
    * Includes a **CPU-based** implementation for performance and correctness comparison.

### General Features
* **Arcball Camera:** Intuitive mouse-based rotation and zoom for easy 3D navigation.
* **Resizable Window:** The viewport and projection matrix update automatically to prevent distortion.
* **Live Performance Metrics:** A real-time FPS counter is displayed in the window title for performance analysis.
* **Axis Gizmo:** A colored axis indicator (Red=X, Green=Y, Blue=Z) provides a clear spatial frame of reference.

---

## Prerequisites

This project is developed for a Linux environment (tested on Ubuntu/Debian). You will need the following dependencies:

* A C++ compiler (`g++`) and `make`
* Git
* GLFW (for windowing)
* GLEW (for OpenGL extensions)
* GLM (for math)

### Installation on Ubuntu/Debian

Open a terminal and run this single command to install all required packages:

```bash
sudo apt update
sudo apt install build-essential git libglfw3-dev libglew-dev libglm-dev
```

---

## How to Clone, Build, and Run

1.  **Clone the repository:**
    ```bash
    git clone [https://github.com/YourUsername/Voxel-Scope.git](https://github.com/YourUsername/Voxel-Scope.git)
    ```
    *(Replace `YourUsername/Voxel-Scope.git` with your actual repository URL)*

2.  **Navigate into the project directory:**
    ```bash
    cd Voxel-Scope
    ```

3.  **Run the script to compile and execute the program:**
    ```bash
    ./run
    ```
    *(This script handles cleaning, compiling, and running the executable.)*

---

## Controls

* **Left Mouse + Drag:** Rotate the camera.
* **Scroll Wheel:** Zoom in and out.
* **'M' Key:** Toggle between **Slicer View** and **Isosurface View**.
* **'C' Key:** (In Slicer View) Cycle the slicing axis (X, Y, Z).
* **'G' Key:** (In Slicer View) Toggle between CPU and GPU slicing methods.
* **'H' Key:** (In Isosurface View) Toggle between CPU and GPU Marching Cubes.

---

## Project Structure

```
.
├── bin/
│   └── Visualizer
├── obj/
│   └── *.o
├── resources/
│   └── redseaT.vtk
├── shaders/
│   ├── ...
├── src/
│   ├── ...
├── makefile
└── run
```

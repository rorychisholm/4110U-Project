## README

### Final Project
**CSCI 4110 Project - Bee Simulation**

### Description
The full report is in the docs folder. This project simulates the behavior of a bee colony within a dynamic 3D environment. The simulation includes realistic terrain generation, hive interactions, and environmental changes. The bees collect pollen from flowers scattered across the terrain and return it to their hive, dynamically growing the colony as more pollen is gathered.

### Features
- **Terrain Generation**: Uses the Diamond-Square algorithm to generate realistic terrains, loaded from files or generated randomly if none are available.
- **Swarm Behavior**: Simulated bee behavior with cohesive movement, obstacle avoidance, and dynamic pollen collection.
- **Interactive Environment**: Dynamic addition and removal of flowers, reflecting environmental changes.
- **Camera Controls**: User-controlled camera for exploring the 3D environment.
- **Object Rendering**: Fully modeled objects like bees, flowers, and the hive, with real-time animations.

### Dependencies
- **Libraries**:
  - OpenGL (GLFW, GLEW)
  - GLM (OpenGL Mathematics)
  - TinyOBJLoader (for loading .obj files)
- **Other Requirements**:
  - A modern C++ compiler supporting C++11 or later.
  - OpenGL-compatible GPU.

### Files
- **main.cpp**: Entry point for the application, handles initialization, simulation loop, and rendering logic.
- **LandMass.h**: Generates and renders the terrain using a fractal algorithm.
- **Member.h**: Defines the behavior and movement of individual bees.
- **Object.h**: Base class for all 3D objects, including their rendering and transformation.
- **Camera.h**: Provides functionality for camera movement and orientation.
- **EcoObj.h**: Represents environmental objects like flowers and hives.

## **Usage**
- **Build the Project**:
  - Select the `Debug` or `Release` configuration.
  - Click `Build > Build Solution` or press `Ctrl + Shift + B`.

- **Run the Executable**:
  - After building, navigate to the `(SolutionDir)\(Configuration)\` folder to find the `.exe` file, and drag it back to the Solution Directory.
  - Or, Click on the premade `.exe` file in the main Solution Directory.
  - Alternatively, press `F5` in Visual Studio to run the program directly.

### Controls
- **W/S**: Move forward/backward.
- **A/D**: Move left/right.
- **Arrow Keys**: Adjust camera pitch and yaw.
- **ESC**: Exit the simulation.
# An A*-Based Rescue Robot Simulation

<img width="800" height="425" alt="RescueX-IntelligentEmergencyResponseSimulation gif" src="https://github.com/user-attachments/assets/7212b886-de76-4a2e-a4da-e262a4bbcff6" />



RescueX is a 2D rescue simulation developed using
C++ and OpenGL/FreeGLUT for the Computer Graphics Laboratory.

The simulation places a rescue robot inside an environment
containing obstacles, fire zones, and stranded survivors.

The robot can operate in two modes:
- Automatic mode using A* pathfinding
- Manual mode controlled by the player

The objective is to safely rescue all survivors and return
them to the rescue base while managing the robot's battery
and avoiding fire zones.

## FEATURES

- Interactive 2D rescue environment
- A* pathfinding for automatic navigation
- Manual robot control
- Automatic/Manual mode switching
- Fire zones with animated fire graphics
- Static walls
- Survivor pickup and rescue mechanism
- Robot battery management
- Fire collision detection
- Mission success and failure states
- Interactive main menu
- Controls and Help interfaces
- Fullscreen support
- Professional HUD displaying mission information

## CONTROLS

| Key | Action |
|-----|--------|
| M | Toggle Automatic / Manual mode |
| W / ↑ | Move Up |
| S / ↓ | Move Down |
| A / ← | Move Left |
| D / → | Move Right |
| P | Pause |
| R | Restart Mission |
| F11 | Toggle Fullscreen |
| ESC | Exit / Return |

## PATHFINDING

RescueX uses the A* (A-star) pathfinding algorithm to
calculate a safe route between the robot and its target.

The algorithm considers:

- Current position
- Target position
- Obstacles
- Traversable areas

The robot recalculates its route when necessary and can
switch between automatic A* navigation and manual control.

## USED TECHNOLOGIES

- C++
- OpenGL
- FreeGLUT
- GLUT
- A* Pathfinding
- 2D Computer Graphics
- Windows / MSYS2 MinGW environment

## Dependencies

- C++ compiler (G++)
- FreeGLUT
- OpenGL
- GLU
- Windows / MSYS2 MinGW environment

### LIBRARIES REQUIRED
- freeglut
- opengl32
- glu32

## BUILD

Clone the repository:

git clone https://github.com/Abdullah-Tawsif/Rescue-Simulation-OpenGL-project.git

#### Navigate to the project:

cd Rescue-Simulation-OpenGL-project

#### Compile:

g++ src/main.cpp -o RescueX.exe -lfreeglut -lopengl32 -lglu32

## How to Run

#### After compilation, run:

./RescueX.exe

The application will open with the RescueX main menu.


## SCREENSHOTS

### MAIN MENU --
<img width="650" height="315" alt="image" src="https://github.com/user-attachments/assets/f6b4c018-8a94-4de3-be11-680404d92970" />

### GAME PLAY --
<img width="650" height="315" alt="image" src="https://github.com/user-attachments/assets/494beb01-1afa-4231-bb31-e64781420247" />

### CONTROLS --
<img width="650" height="315" alt="image" src="https://github.com/user-attachments/assets/e9569d9a-c54a-4e52-ac88-19904ffeae4e" />

### MISSION FAILED --
<img width="650" height="315" alt="image" src="https://github.com/user-attachments/assets/538f1838-4245-41cd-b254-c79fabf6818b" />

### MISSION COMPLETE --

<img width="650" height="315" alt="image" src="https://github.com/user-attachments/assets/e296610f-8f12-4217-be28-49342f9cd110" />


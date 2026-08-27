# RescueX – AI Rescue Simulation

<img width="1246" height="687" alt="image" src="https://github.com/user-attachments/assets/1206bac2-a1f7-47c7-b37d-897afd2d6819" />


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

### LIBRARIES REQUIRED
- freeglut
- opengl32
- glu32

## BUILD

Clone the repository:

git clone <YOUR_GITHUB_REPOSITORY_URL>

Navigate to the project:

cd RescueX

Compile:

g++ src/main.cpp -o RescueX.exe -lfreeglut -lopengl32 -lglu32


## How to Run

After compilation, run:

./RescueX.exe

The application will open with the RescueX main menu.

Select:

START MISSION

to begin the simulation.

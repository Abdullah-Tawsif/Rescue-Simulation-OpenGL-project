#include <GL/freeglut.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"

using namespace std;

// ============================================================
// WINDOW / MAP
// ============================================================

const int MAP_WIDTH  = 1261;
const int MAP_HEIGHT = 700;

const int GRID_SIZE = 30;

// ============================================================
// COLORS
// ============================================================

struct Color
{
    float r, g, b;
};

// ============================================================
// RECTANGLE
// ============================================================

struct Rect
{
    float x;
    float y;
    float w;
    float h;
};

// ============================================================
// SURVIVOR
// ============================================================

struct Survivor
{
    float x;
    float y;
    bool rescued;
};

// ============================================================
// FIRE ZONE
// ============================================================

struct FireZone
{
    float x;
    float y;
    float w;
    float h;
};

// ============================================================
// FIRE TEXTURES
// ============================================================

GLuint fireTextures[5];

int currentFireFrame = 0;

// ============================================================
// ROBOT
// ============================================================

float robotX = 85;
float robotY = 360;

// ============================================================
// SURVIVORS
// Positions taken approximately from the supplied image
// ============================================================

vector<Survivor> survivors =
{
    { 515,  55, false },
    { 765,  55, false },
    { 515, 180, false },
    { 565, 495, false },
    { 745, 500, false }
};

// ============================================================
// FIRE ZONES
//
// The image contains 7 fire locations.
// The 5 images are animation frames, NOT five fire locations.
// ============================================================

vector<FireZone> fires =
{
    // Upper fire
    { 615, 65, 60, 95 },

    // Right upper fire
    { 750, 290, 60, 95 },

    // Bottom-center fire cluster
    { 345, 360, 60, 95 },
    { 405, 360, 60, 95 },
    { 315, 465, 60, 95 },
    { 375, 465, 60, 95 },
    { 435, 465, 60, 95 }
};

// ============================================================
// OBSTACLES / WALLS
//
// Approximate positions based on supplied image.
// ============================================================

vector<Rect> obstacles =
{
    // Left vertical wall
    { 130, 0, 62, 188 },

    // Middle-left vertical wall
    { 280, 90, 55, 375 },

    // Middle upper horizontal wall
    { 425, 90, 190, 45 },

    // Upper right horizontal wall
    { 680, 95, 125, 40 },

    // Middle horizontal wall
    { 425, 240, 190, 45 },

    // Right middle horizontal wall
    { 720, 380, 541, 45 },

    // Bottom-middle vertical wall
    { 470, 350, 55, 350 },

    // Bottom-right vertical wall
    { 610, 385, 50, 315 },

    // Small upper-right block
    { 738, 90, 65, 45 }
};

// ============================================================
// DRAW RECTANGLE
// ============================================================

void drawRect(float x, float y, float w, float h, Color c)
{
    glColor3f(c.r, c.g, c.b);

    glBegin(GL_QUADS);

    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);

    glEnd();
}

// ============================================================
// DRAW GRID
// ============================================================

void drawGrid()
{
    glColor3f(0.88f, 0.88f, 0.88f);

    glLineWidth(1.0f);

    glBegin(GL_LINES);

    for (int x = 0; x <= MAP_WIDTH; x += GRID_SIZE)
    {
        glVertex2f(x, 0);
        glVertex2f(x, MAP_HEIGHT);
    }

    for (int y = 0; y <= MAP_HEIGHT; y += GRID_SIZE)
    {
        glVertex2f(0, y);
        glVertex2f(MAP_WIDTH, y);
    }

    glEnd();
}

// ============================================================
// DRAW OBSTACLES
// ============================================================

void drawObstacles()
{
    Color wall = { 0.43f, 0.43f, 0.43f };

    for (const auto& r : obstacles)
    {
        drawRect(r.x, r.y, r.w, r.h, wall);
    }
}

// ============================================================
// DRAW RESCUE STATION
// ============================================================

void drawRescueStation()
{
    // White building
    drawRect(
        30,
        475,
        75,
        75,
        {0.86f, 0.86f, 0.86f}
    );

    // Red roof
    glColor3f(0.95f, 0.05f, 0.08f);

    glBegin(GL_TRIANGLES);

    glVertex2f(25, 475);
    glVertex2f(67, 410);
    glVertex2f(110, 475);

    glEnd();

    // Red cross
    glColor3f(0.90f, 0.02f, 0.04f);

    glBegin(GL_QUADS);

    // Vertical
    glVertex2f(61, 455);
    glVertex2f(73, 455);
    glVertex2f(73, 505);
    glVertex2f(61, 505);

    // Horizontal
    glVertex2f(47, 470);
    glVertex2f(87, 470);
    glVertex2f(87, 482);
    glVertex2f(47, 482);

    glEnd();
}

// ============================================================
// DRAW ROBOT
// ============================================================

void drawRobot()
{
    // Body
    drawRect(
        robotX - 25,
        robotY - 25,
        50,
        60,
        {0.12f, 0.15f, 0.80f}
    );

    // Head
    drawRect(
        robotX - 17,
        robotY + 35,
        34,
        35,
        {0.55f, 0.82f, 0.88f}
    );

    // Head border
    glColor3f(0.02f, 0.02f, 0.02f);

    glLineWidth(4);

    glBegin(GL_LINE_LOOP);

    glVertex2f(robotX - 17, robotY + 35);
    glVertex2f(robotX + 17, robotY + 35);
    glVertex2f(robotX + 17, robotY + 70);
    glVertex2f(robotX - 17, robotY + 70);

    glEnd();

    // Screen
    drawRect(
        robotX - 10,
        robotY + 45,
        20,
        18,
        {0.75f, 0.92f, 0.95f}
    );

    // Antenna
    glColor3f(0.05f, 0.05f, 0.05f);

    glLineWidth(4);

    glBegin(GL_LINES);

    glVertex2f(robotX, robotY + 70);
    glVertex2f(robotX, robotY + 85);

    glEnd();

    // Antenna light
    drawRect(
        robotX - 5,
        robotY + 82,
        10,
        10,
        {1.0f, 0.0f, 0.05f}
    );

    // Wheels
    drawRect(
        robotX - 22,
        robotY - 40,
        14,
        15,
        {0.05f, 0.05f, 0.05f}
    );

    drawRect(
        robotX + 8,
        robotY - 40,
        14,
        15,
        {0.05f, 0.05f, 0.05f}
    );
}

// ============================================================
// DRAW SURVIVOR
// ============================================================

void drawSurvivor(float x, float y)
{
    // Head
    glColor3f(1.0f, 0.88f, 0.62f);

    glBegin(GL_POLYGON);

    for (int i = 0; i < 30; i++)
    {
        float angle = 2.0f * 3.1415926f * i / 30.0f;

        float px = x + cos(angle) * 12;
        float py = y + 42 + sin(angle) * 12;

        glVertex2f(px, py);
    }

    glEnd();

    // Body
    drawRect(
        x - 15,
        y,
        30,
        43,
        {0.55f, 0.90f, 0.05f}
    );

    // Arms
    glColor3f(0.05f, 0.05f, 0.05f);

    glLineWidth(3);

    glBegin(GL_LINES);

    glVertex2f(x - 15, y + 32);
    glVertex2f(x - 27, y + 18);

    glVertex2f(x + 15, y + 32);
    glVertex2f(x + 27, y + 18);

    // Legs
    glVertex2f(x - 7, y);
    glVertex2f(x - 7, y - 20);

    glVertex2f(x + 7, y);
    glVertex2f(x + 7, y - 20);

    glEnd();
}

// ============================================================
// LOAD FIRE TEXTURE
// ============================================================

GLuint loadTexture(const char* filename)
{
    int width;
    int height;
    int channels;

    unsigned char* image =
        stbi_load(
            filename,
            &width,
            &height,
            &channels,
            4
        );

    if (!image)
    {
        cout << "Failed to load: "
             << filename
             << endl;

        return 0;
    }

    GLuint texture;

    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_CLAMP
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_CLAMP
    );

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        image
    );

    stbi_image_free(image);

    return texture;
}

// ============================================================
// DRAW ONE FIRE
// ============================================================

void drawFire(float x, float y, float w, float h)
{
    GLuint texture = fireTextures[currentFireFrame];

    if (texture == 0)
        return;

    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, texture);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);

    glTexCoord2f(0, 0);
    glVertex2f(x, y);

    glTexCoord2f(1, 0);
    glVertex2f(x + w, y);

    glTexCoord2f(1, 1);
    glVertex2f(x + w, y + h);

    glTexCoord2f(0, 1);
    glVertex2f(x, y + h);

    glEnd();

    glDisable(GL_TEXTURE_2D);
}

// ============================================================
// DRAW ALL FIRE ZONES
// ============================================================

void drawFires()
{
    for (const auto& fire : fires)
    {
        drawFire(
            fire.x,
            fire.y,
            fire.w,
            fire.h
        );
    }
}

// ============================================================
// HUD
// ============================================================

void drawText(
    float x,
    float y,
    const string& text
)
{
    glColor3f(0.05f, 0.05f, 0.05f);

    glRasterPos2f(x, y);

    for (char c : text)
    {
        glutBitmapCharacter(
            GLUT_BITMAP_HELVETICA_12,
            c
        );
    }
}

// ============================================================
// INFORMATION PANEL
// ============================================================

void drawHUD()
{
    // Panel
    drawRect(
        800,
        430,
        460,
        270,
        {0.98f, 0.98f, 0.98f}
    );

    // Border
    glColor3f(0.02f, 0.02f, 0.02f);

    glLineWidth(4);

    glBegin(GL_LINE_LOOP);

    glVertex2f(800, 430);
    glVertex2f(1260, 430);
    glVertex2f(1260, 700);
    glVertex2f(800, 700);

    glEnd();

    drawText(
        825,
        665,
        "| RESCUEX"
    );

    drawText(
        825,
        640,
        "| AI Status: ACTIVE"
    );

    drawText(
        825,
        615,
        "| Survivors: 5"
    );

    drawText(
        825,
        590,
        "| Rescued: 0"
    );

    drawText(
        825,
        565,
        "| Current Target: #4"
    );

    drawText(
        825,
        540,
        "| Robot Battery: 74%"
    );

    drawText(
        825,
        515,
        "| Fire Zones: 7"
    );
}

// ============================================================
// DISPLAY
// ============================================================

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Background
    glColor3f(
        0.96f,
        0.96f,
        0.96f
    );

    glBegin(GL_QUADS);

    glVertex2f(0, 0);
    glVertex2f(MAP_WIDTH, 0);
    glVertex2f(MAP_WIDTH, MAP_HEIGHT);
    glVertex2f(0, MAP_HEIGHT);

    glEnd();

    // Grid
    drawGrid();

    // Obstacles
    drawObstacles();

    // Rescue station
    drawRescueStation();

    // Survivors
    for (const auto& survivor : survivors)
    {
        if (!survivor.rescued)
        {
            drawSurvivor(
                survivor.x,
                survivor.y
            );
        }
    }

    // Fire
    drawFires();

    // Robot
    drawRobot();

    // HUD
    drawHUD();

    glutSwapBuffers();
}

// ============================================================
// FIRE ANIMATION
// ============================================================

void updateFire(int value)
{
    currentFireFrame++;

    if (currentFireFrame >= 5)
        currentFireFrame = 0;

    glutPostRedisplay();

    // Animation speed
    glutTimerFunc(
        120,
        updateFire,
        0
    );
}

// ============================================================
// KEYBOARD
// ============================================================

void keyboard(
    unsigned char key,
    int x,
    int y
)
{
    const float speed = 10.0f;

    switch (key)
    {
        case 'w':
        case 'W':
            robotY += speed;
            break;

        case 's':
        case 'S':
            robotY -= speed;
            break;

        case 'a':
        case 'A':
            robotX -= speed;
            break;

        case 'd':
        case 'D':
            robotX += speed;
            break;

        case 27:
            exit(0);
    }

    glutPostRedisplay();
}

// ============================================================
// SPECIAL KEYS
// ============================================================

void specialKeys(
    int key,
    int x,
    int y
)
{
    const float speed = 10.0f;

    switch (key)
    {
        case GLUT_KEY_UP:
            robotY += speed;
            break;

        case GLUT_KEY_DOWN:
            robotY -= speed;
            break;

        case GLUT_KEY_LEFT:
            robotX -= speed;
            break;

        case GLUT_KEY_RIGHT:
            robotX += speed;
            break;
    }

    glutPostRedisplay();
}

// ============================================================
// INITIALIZATION
// ============================================================

void init()
{
    glClearColor(
        0.96f,
        0.96f,
        0.96f,
        1.0f
    );

    // 2D projection
    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();

    gluOrtho2D(
        0,
        MAP_WIDTH,
        0,
        MAP_HEIGHT
    );

    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();

    // Transparency
    glEnable(GL_BLEND);

    glBlendFunc(
        GL_SRC_ALPHA,
        GL_ONE_MINUS_SRC_ALPHA
    );

    // Load the 5 fire animation frames
    fireTextures[0] =
        loadTexture("assets/fire1.png");

    fireTextures[1] =
        loadTexture("assets/fire2.png");

    fireTextures[2] =
        loadTexture("assets/fire3.png");

    fireTextures[3] =
        loadTexture("assets/fire4.png");

    fireTextures[4] =
        loadTexture("assets/fire5.png");
}

// ============================================================
// MAIN
// ============================================================

int main(
    int argc,
    char** argv
)
{
    glutInit(&argc, argv);

    glutInitDisplayMode(
        GLUT_DOUBLE |
        GLUT_RGBA
    );

    glutInitWindowSize(
        MAP_WIDTH,
        MAP_HEIGHT
    );

    glutCreateWindow(
        "RescueX - 2D Intelligent Rescue Simulation"
    );

    init();

    glutDisplayFunc(display);

    glutKeyboardFunc(keyboard);

    glutSpecialFunc(specialKeys);

    glutTimerFunc(
        120,
        updateFire,
        0
    );

    glutMainLoop();

    return 0;
}

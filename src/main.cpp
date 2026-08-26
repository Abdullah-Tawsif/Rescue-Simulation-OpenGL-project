#include <GL/freeglut.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"

using namespace std;


// WINDOW / MAP

const int WINDOW_WIDTH  = 1261;
const int WINDOW_HEIGHT = 700;

// Simulation area
const float MAP_LEFT   = 20.0f;
const float MAP_BOTTOM = 20.0f;
const float MAP_RIGHT  = 915.0f;
const float MAP_TOP    = 620.0f;

// HUD
const float HUD_LEFT   = 935.0f;
const float HUD_RIGHT  = 1240.0f;
const float HUD_BOTTOM = 20.0f;
const float HUD_TOP    = 620.0f;

const int GRID_SIZE = 30;


// COLORS

struct Color
{
    float r;
    float g;
    float b;
};

// RECTANGLE
struct Rect
{
    float x;
    float y;
    float w;
    float h;
};

// SURVIVOR

struct Survivor
{
    float x;
    float y;
    bool rescued;
};


// FIRE ZONE

struct FireZone
{
    float x;
    float y;
    float w;
    float h;
};


// FIRE TEXTURES

GLuint fireTextures[5];

int currentFireFrame = 0;


// ROBOT

float robotX = 85.0f;
float robotY = 350.0f;


// SURVIVORS

vector<Survivor> survivors =
{
    //S1
    { 580, 540, false },
    
    //S2
    { 860, 540, false },

    //S3
    { 580, 370, false },

    //S4
    { 590, 70, false },

    //S5
    { 860, 60, false }
    
};

// FIRE ZONES

vector<FireZone> fires =
{
    // F1
    { 670, 440, 40, 50 },

    // F2
    { 280, 20, 60, 80 },

    // F3
    { 350, 20, 60, 80 },

    // F4
    { 420, 20, 60, 80 },

    // F5
    {450, 550, 45, 65},

    // F6
    {850, 170, 65, 95},
    
};


// OBSTACLES / WALLS

vector<Rect> obstacles =
{
    //left vertical wall l1
    { 160, 450, 40, 170 },
    
    // Left vertical wall l2
    { 160, 20, 40, 300 },

    // Middle-left vertical wall l3
    { 300, 120, 40, 370 },

    // Middle-left vertical wall l4
    { 500, 280, 40, 340},

    //L5
    {540, 450, 120, 40},
    
    //L6
    {540, 280, 120, 40},

    // Bottom-middle vertical wall L7
    { 500, 20, 40, 140 },

    // Bottom-right vertical wall L8
    { 660, 20, 40, 140 },

    // L9
    { 820, 450, 95, 40 },

    // L10
    { 760, 280, 155, 40 },

    // L11
    { 820, 120, 95, 40 },

};


// DRAW FILLED RECTANGLE

void drawRect(
    float x,
    float y,
    float w,
    float h,
    Color c
)
{
    glDisable(GL_TEXTURE_2D);

    glColor3f(
        c.r,
        c.g,
        c.b
    );

    glBegin(GL_QUADS);

    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);

    glEnd();
}


// DRAW OUTLINE RECTANGLE

void drawOutline(
    float x,
    float y,
    float w,
    float h,
    Color c,
    float thickness = 2.0f
)
{
    glDisable(GL_TEXTURE_2D);

    glColor3f(
        c.r,
        c.g,
        c.b
    );

    glLineWidth(thickness);

    glBegin(GL_LINE_LOOP);

    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);

    glEnd();
}


// DRAW TEXT

void drawText(
    float x,
    float y,
    const string& text,
    Color color,
    void* font = GLUT_BITMAP_HELVETICA_12
)
{
    glDisable(GL_TEXTURE_2D);

    glColor3f(
        color.r,
        color.g,
        color.b
    );

    glRasterPos2f(
        x,
        y
    );

    for (char c : text)
    {
        glutBitmapCharacter(
            font,
            c
        );
    }
}


// CENTERED TEXT

void drawCenteredText(
    float x,
    float y,
    const string& text,
    Color color,
    void* font = GLUT_BITMAP_HELVETICA_12
)
{
    int width = 0;

    for (char c : text)
    {
        width += glutBitmapWidth(
            font,
            c
        );
    }

    drawText(
        x - width / 2.0f,
        y,
        text,
        color,
        font
    );
}


// DRAW GRID

void drawGrid()
{
    glDisable(GL_TEXTURE_2D);

    // Very subtle grid
    glColor3f(
        0.11f,
        0.15f,
        0.18f
    );

    glLineWidth(1.0f);

    glBegin(GL_LINES);

    for (
        float x = MAP_LEFT;
        x <= MAP_RIGHT;
        x += GRID_SIZE
    )
    {
        glVertex2f(
            x,
            MAP_BOTTOM
        );

        glVertex2f(
            x,
            MAP_TOP
        );
    }

    for (
        float y = MAP_BOTTOM;
        y <= MAP_TOP;
        y += GRID_SIZE
    )
    {
        glVertex2f(
            MAP_LEFT,
            y
        );

        glVertex2f(
            MAP_RIGHT,
            y
        );
    }

    glEnd();
}


// MAP BACKGROUND

void drawMapBackground()
{
    // Main map
    drawRect(
        MAP_LEFT,
        MAP_BOTTOM,
        MAP_RIGHT - MAP_LEFT,
        MAP_TOP - MAP_BOTTOM,
        {0.055f, 0.075f, 0.085f}
    );

    // Inner play area
    drawRect(
        MAP_LEFT + 5,
        MAP_BOTTOM + 5,
        MAP_RIGHT - MAP_LEFT - 10,
        MAP_TOP - MAP_BOTTOM - 10,
        {0.065f, 0.090f, 0.100f}
    );

    drawGrid();
}

// MAP BORDER

void drawMapBorder()
{
    drawOutline(
        MAP_LEFT,
        MAP_BOTTOM,
        MAP_RIGHT - MAP_LEFT,
        MAP_TOP - MAP_BOTTOM,
        {0.18f, 0.55f, 0.62f},
        3.0f
    );

    // Small corner markers

    Color marker = {
        0.20f,
        0.80f,
        0.85f
    };

    float s = 15.0f;

    // Bottom-left
    glColor3f(
        marker.r,
        marker.g,
        marker.b
    );

    glLineWidth(3);

    glBegin(GL_LINES);

    glVertex2f(MAP_LEFT, MAP_BOTTOM + s);
    glVertex2f(MAP_LEFT, MAP_BOTTOM);

    glVertex2f(MAP_LEFT, MAP_BOTTOM);
    glVertex2f(MAP_LEFT + s, MAP_BOTTOM);

    // Bottom-right
    glVertex2f(MAP_RIGHT - s, MAP_BOTTOM);
    glVertex2f(MAP_RIGHT, MAP_BOTTOM);

    glVertex2f(MAP_RIGHT, MAP_BOTTOM);
    glVertex2f(MAP_RIGHT, MAP_BOTTOM + s);

    // Top-left
    glVertex2f(MAP_LEFT, MAP_TOP - s);
    glVertex2f(MAP_LEFT, MAP_TOP);

    glVertex2f(MAP_LEFT, MAP_TOP);
    glVertex2f(MAP_LEFT + s, MAP_TOP);

    // Top-right
    glVertex2f(MAP_RIGHT - s, MAP_TOP);
    glVertex2f(MAP_RIGHT, MAP_TOP);

    glVertex2f(MAP_RIGHT, MAP_TOP);
    glVertex2f(MAP_RIGHT, MAP_TOP - s);

    glEnd();
}


// DRAW OBSTACLE

void drawSingleObstacle(
    const Rect& r
)
{
    // Outer border
    drawRect(
        r.x - 2,
        r.y - 2,
        r.w + 4,
        r.h + 4,
        {0.08f, 0.09f, 0.10f}
    );

    // Main wall
    drawRect(
        r.x,
        r.y,
        r.w,
        r.h,
        {0.24f, 0.28f, 0.31f}
    );

    // Top highlight
    drawRect(
        r.x,
        r.y + r.h - 5,
        r.w,
        5,
        {0.38f, 0.43f, 0.46f}
    );

    // Vertical details
    glColor3f(
        0.17f,
        0.20f,
        0.22f
    );

    glLineWidth(1.5f);

    for (
        float x = r.x + 15;
        x < r.x + r.w;
        x += 25
    )
    {
        glBegin(GL_LINES);

        glVertex2f(
            x,
            r.y + 5
        );

        glVertex2f(
            x,
            r.y + r.h - 5
        );

        glEnd();
    }

    // Outline
    drawOutline(
        r.x,
        r.y,
        r.w,
        r.h,
        {0.45f, 0.50f, 0.53f},
        1.5f
    );
}


// DRAW ALL OBSTACLES

void drawObstacles()
{
    for (const auto& obstacle : obstacles)
    {
        drawSingleObstacle(
            obstacle
        );
    }
}


// DRAW RESCUE STATION

void drawRescueStation()
{
    float x = 45;
    float y = 60;

    // Station shadow
    drawRect(
        x + 5,
        y - 5,
        105,
        85,
        {0.015f, 0.02f, 0.025f}
    );

    // Main building
    drawRect(
        x,
        y,
        105,
        80,
        {0.82f, 0.86f, 0.87f}
    );

    // Building upper strip
    drawRect(
        x,
        y + 68,
        105,
        12,
        {0.18f, 0.55f, 0.62f}
    );

    // Roof
    glColor3f(
        0.75f,
        0.08f,
        0.10f
    );

    glBegin(GL_TRIANGLES);

    glVertex2f(
        x - 8,
        y + 80
    );

    glVertex2f(
        x + 52.5f,
        y + 115
    );

    glVertex2f(
        x + 113,
        y + 80
    );

    glEnd();

    // Medical cross
    Color red = {
        0.85f,
        0.05f,
        0.07f
    };

    drawRect(
        x + 46,
        y + 15,
        14,
        38,
        red
    );

    drawRect(
        x + 34,
        y + 27,
        38,
        14,
        red
    );

    // Station outline
    drawOutline(
        x,
        y,
        105,
        80,
        {0.95f, 0.95f, 0.95f},
        2.0f
    );

    drawText(
        x + 10,
        y - 18,
        "RESCUE BASE",
        {0.35f, 0.85f, 0.88f},
        GLUT_BITMAP_HELVETICA_10
    );
}


// DRAW ROBOT SHADOW

void drawRobotShadow()
{
    glDisable(GL_TEXTURE_2D);

    glColor4f(
        0.0f,
        0.0f,
        0.0f,
        0.35f
    );

    glBegin(GL_QUADS);

    glVertex2f(
        robotX - 25,
        robotY - 40
    );

    glVertex2f(
        robotX + 25,
        robotY - 40
    );

    glVertex2f(
        robotX + 25,
        robotY - 30
    );

    glVertex2f(
        robotX - 25,
        robotY - 30
    );

    glEnd();
}


// DRAW ROBOT

void drawRobot()
{
    drawRobotShadow();

    // Wheels

    drawRect(
        robotX - 25,
        robotY - 42,
        17,
        15,
        {0.025f, 0.03f, 0.035f}
    );

    drawRect(
        robotX + 8,
        robotY - 42,
        17,
        15,
        {0.025f, 0.03f, 0.035f}
    );

    // Wheel center
    drawRect(
        robotX - 20,
        robotY - 38,
        7,
        7,
        {0.22f, 0.70f, 0.75f}
    );

    drawRect(
        robotX + 13,
        robotY - 38,
        7,
        7,
        {0.22f, 0.70f, 0.75f}
    );

    // Main body

    drawRect(
        robotX - 28,
        robotY - 27,
        56,
        62,
        {0.07f, 0.25f, 0.55f}
    );

    // Body highlight
    drawRect(
        robotX - 23,
        robotY - 22,
        46,
        5,
        {0.18f, 0.48f, 0.82f}
    );

    // Body border
    drawOutline(
        robotX - 28,
        robotY - 27,
        56,
        62,
        {0.30f, 0.65f, 0.78f},
        2.0f
    );

    // Head

    drawRect(
        robotX - 20,
        robotY + 35,
        40,
        36,
        {0.45f, 0.75f, 0.80f}
    );

    // Head border
    drawOutline(
        robotX - 20,
        robotY + 35,
        40,
        36,
        {0.05f, 0.12f, 0.15f},
        3.0f
    );

    // Screen

    drawRect(
        robotX - 13,
        robotY + 45,
        26,
        18,
        {0.03f, 0.12f, 0.15f}
    );

    drawRect(
        robotX - 9,
        robotY + 49,
        7,
        7,
        {0.20f, 0.90f, 0.90f}
    );

    drawRect(
        robotX + 2,
        robotY + 49,
        7,
        7,
        {0.20f, 0.90f, 0.90f}
    );

    // Antenna

    glColor3f(
        0.05f,
        0.08f,
        0.10f
    );

    glLineWidth(3);

    glBegin(GL_LINES);

    glVertex2f(
        robotX,
        robotY + 71
    );

    glVertex2f(
        robotX,
        robotY + 88
    );

    glEnd();

    // Antenna light
    drawRect(
        robotX - 5,
        robotY + 87,
        10,
        10,
        {0.95f, 0.10f, 0.12f}
    );

    // Arms

    glColor3f(
        0.12f,
        0.32f,
        0.62f
    );

    glLineWidth(6);

    glBegin(GL_LINES);

    glVertex2f(
        robotX - 28,
        robotY + 12
    );

    glVertex2f(
        robotX - 42,
        robotY - 2
    );

    glVertex2f(
        robotX + 28,
        robotY + 12
    );

    glVertex2f(
        robotX + 42,
        robotY - 2
    );

    glEnd();

    // Robot label

    drawText(
        robotX - 22,
        robotY - 60,
        "ROBOT",
        {0.30f, 0.80f, 0.85f},
        GLUT_BITMAP_HELVETICA_10
    );
}


// DRAW SURVIVOR

void drawSurvivor(
    float x,
    float y
)
{
    // Shadow
    glColor4f(
        0.0f,
        0.0f,
        0.0f,
        0.35f
    );

    glBegin(GL_QUADS);

    glVertex2f(x - 18, y - 22);
    glVertex2f(x + 18, y - 22);
    glVertex2f(x + 18, y - 15);
    glVertex2f(x - 18, y - 15);

    glEnd();

    // Head

    glColor3f(
        1.0f,
        0.76f,
        0.53f
    );

    glBegin(GL_POLYGON);

    for (int i = 0; i < 30; i++)
    {
        float angle =
            2.0f *
            3.1415926f *
            i /
            30.0f;

        float px =
            x +
            cos(angle) * 11;

        float py =
            y +
            42 +
            sin(angle) * 11;

        glVertex2f(
            px,
            py
        );
    }

    glEnd();

    // Body

    drawRect(
        x - 15,
        y,
        30,
        40,
        {0.16f, 0.67f, 0.30f}
    );

    // Body stripe
    drawRect(
        x - 15,
        y + 25,
        30,
        7,
        {0.20f, 0.80f, 0.40f}
    );

    // Arms

    glColor3f(
        0.15f,
        0.18f,
        0.20f
    );

    glLineWidth(3);

    glBegin(GL_LINES);

    glVertex2f(
        x - 15,
        y + 30
    );

    glVertex2f(
        x - 27,
        y + 17
    );

    glVertex2f(
        x + 15,
        y + 30
    );

    glVertex2f(
        x + 27,
        y + 17
    );

    glEnd();

    // Legs

    glBegin(GL_LINES);

    glVertex2f(
        x - 7,
        y
    );

    glVertex2f(
        x - 7,
        y - 18
    );

    glVertex2f(
        x + 7,
        y
    );

    glVertex2f(
        x + 7,
        y - 18
    );

    glEnd();

    // ========================================================
    // Survivor marker
    // ========================================================

    drawOutline(
        x - 22,
        y - 25,
        44,
        75,
        {0.25f, 0.85f, 0.45f},
        1.5f
    );

    drawText(
        x - 13,
        y - 38,
        "SURVIVOR",
        {0.35f, 0.90f, 0.50f},
        GLUT_BITMAP_HELVETICA_10
    );
}


// ============================================================
// LOAD FIRE TEXTURE
// ============================================================

GLuint loadTexture(
    const char* filename
)
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
        cout
            << "Failed to load: "
            << filename
            << endl;

        return 0;
    }


    // Remove dark background from fire image
    
    for (
        int i = 0;
        i < width * height;
        i++
    )
    {
        unsigned char* pixel =
            &image[i * 4];

        int r = pixel[0];
        int g = pixel[1];
        int b = pixel[2];

        // Very dark pixels become transparent
        if (
            r < 35 &&
            g < 35 &&
            b < 35
        )
        {
            pixel[3] = 0;
        }
    }


    GLuint texture;

    glGenTextures(
        1,
        &texture
    );

    glBindTexture(
        GL_TEXTURE_2D,
        texture
    );


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


    stbi_image_free(
        image
    );

    return texture;
}


// DRAW FIRE

void drawFire(
    float x,
    float y,
    float w,
    float h
)
{
    GLuint texture =
        fireTextures[currentFireFrame];

    if (texture == 0)
        return;


    // ========================================================
    // Fire danger zone
    // ========================================================

    glDisable(GL_TEXTURE_2D);

    // Outer glow
    glColor4f(
        1.0f,
        0.20f,
        0.02f,
        0.08f
    );

    glBegin(GL_QUADS);

    glVertex2f(
        x - 12,
        y - 8
    );

    glVertex2f(
        x + w + 12,
        y - 8
    );

    glVertex2f(
        x + w + 12,
        y + h + 8
    );

    glVertex2f(
        x - 12,
        y + h + 8
    );

    glEnd();


    // ========================================================
    // Fire texture
    // ========================================================

    glEnable(GL_TEXTURE_2D);

    glBindTexture(
        GL_TEXTURE_2D,
        texture
    );

    glColor4f(
        1.0f,
        1.0f,
        1.0f,
        1.0f
    );


    glBegin(GL_QUADS);


    // vertically to correct the inverted fire image.

    glTexCoord2f(
        0.0f,
        1.0f
    );

    glVertex2f(
        x,
        y
    );


    glTexCoord2f(
        1.0f,
        1.0f
    );

    glVertex2f(
        x + w,
        y
    );


    glTexCoord2f(
        1.0f,
        0.0f
    );

    glVertex2f(
        x + w,
        y + h
    );


    glTexCoord2f(
        0.0f,
        0.0f
    );

    glVertex2f(
        x,
        y + h
    );


    glEnd();


    glBindTexture(
        GL_TEXTURE_2D,
        0
    );

    glDisable(GL_TEXTURE_2D);
}


// DRAW ALL FIRE

void drawFires()
{
    for (
        const auto& fire :
        fires
    )
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
// FIRE ZONE MARKERS
// ============================================================

void drawFireZoneMarkers()
{
    for (
        const auto& fire :
        fires
    )
    {
        glDisable(GL_TEXTURE_2D);

        glColor4f(
            1.0f,
            0.20f,
            0.05f,
            0.35f
        );

        glLineWidth(1.5f);

        glBegin(GL_LINE_LOOP);

        glVertex2f(
            fire.x - 5,
            fire.y - 5
        );

        glVertex2f(
            fire.x + fire.w + 5,
            fire.y - 5
        );

        glVertex2f(
            fire.x + fire.w + 5,
            fire.y + fire.h + 5
        );

        glVertex2f(
            fire.x - 5,
            fire.y + fire.h + 5
        );

        glEnd();
    }
}


// ============================================================
// HEADER
// ============================================================

void drawHeader()
{
    // Header background
    drawRect(
        20,
        635,
        1220,
        45,
        {0.035f, 0.055f, 0.065f}
    );

    // Cyan accent
    drawRect(
        20,
        635,
        6,
        45,
        {0.20f, 0.80f, 0.85f}
    );

    // Title
    drawText(
        40,
        658,
        "RESCUEX",
        {0.40f, 0.90f, 0.92f},
        GLUT_BITMAP_HELVETICA_18
    );

    drawText(
        130,
        658,
        "// INTELLIGENT EMERGENCY RESPONSE SIMULATION",
        {0.55f, 0.62f, 0.65f},
        GLUT_BITMAP_HELVETICA_10
    );

    // Live indicator
    glColor3f(
        0.15f,
        0.95f,
        0.40f
    );

    glPointSize(8);

    glBegin(GL_POINTS);

    glVertex2f(
        850,
        657
    );

    glEnd();

    drawText(
        865,
        653,
        "SIMULATION LIVE",
        {0.30f, 0.90f, 0.45f},
        GLUT_BITMAP_HELVETICA_10
    );
}


// ============================================================
// HUD PANEL
// ============================================================

void drawHUD()
{
    // Main panel
    drawRect(
        HUD_LEFT,
        HUD_BOTTOM,
        HUD_RIGHT - HUD_LEFT,
        HUD_TOP - HUD_BOTTOM,
        {0.045f, 0.060f, 0.070f}
    );

    // Border
    drawOutline(
        HUD_LEFT,
        HUD_BOTTOM,
        HUD_RIGHT - HUD_LEFT,
        HUD_TOP - HUD_BOTTOM,
        {0.20f, 0.45f, 0.50f},
        2.0f
    );


    // ========================================================
    // HUD HEADER
    // ========================================================

    drawRect(
        HUD_LEFT,
        565,
        HUD_RIGHT - HUD_LEFT,
        55,
        {0.065f, 0.095f, 0.105f}
    );

    drawText(
        HUD_LEFT + 20,
        595,
        "MISSION CONTROL",
        {0.40f, 0.90f, 0.92f},
        GLUT_BITMAP_HELVETICA_18
    );

    drawText(
        HUD_LEFT + 20,
        577,
        "AUTONOMOUS RESCUE UNIT",
        {0.45f, 0.50f, 0.53f},
        GLUT_BITMAP_HELVETICA_10
    );


    // ========================================================
    // AI STATUS
    // ========================================================

    drawText(
        HUD_LEFT + 20,
        535,
        "AI STATUS",
        {0.45f, 0.50f, 0.53f},
        GLUT_BITMAP_HELVETICA_10
    );

    drawText(
        HUD_LEFT + 145,
        535,
        "ACTIVE",
        {0.20f, 0.90f, 0.45f},
        GLUT_BITMAP_HELVETICA_12
    );


    // ========================================================
    // ALGORITHM
    // ========================================================

    drawText(
        HUD_LEFT + 20,
        505,
        "PATHFINDING",
        {0.45f, 0.50f, 0.53f},
        GLUT_BITMAP_HELVETICA_10
    );

    drawText(
        HUD_LEFT + 145,
        505,
        "A*",
        {0.30f, 0.80f, 0.90f},
        GLUT_BITMAP_HELVETICA_12
    );


    // ========================================================
    // SURVIVORS
    // ========================================================

    drawText(
        HUD_LEFT + 20,
        465,
        "SURVIVORS",
        {0.45f, 0.50f, 0.53f},
        GLUT_BITMAP_HELVETICA_10
    );

    drawText(
        HUD_LEFT + 145,
        465,
        "5",
        {0.90f, 0.90f, 0.90f},
        GLUT_BITMAP_HELVETICA_12
    );


    drawText(
        HUD_LEFT + 20,
        440,
        "RESCUED",
        {0.45f, 0.50f, 0.53f},
        GLUT_BITMAP_HELVETICA_10
    );

    int rescuedCount = 0;

    for (
        const auto& survivor :
        survivors
    )
    {
        if (survivor.rescued)
            rescuedCount++;
    }

    drawText(
        HUD_LEFT + 145,
        440,
        to_string(rescuedCount),
        {0.20f, 0.90f, 0.45f},
        GLUT_BITMAP_HELVETICA_12
    );


    // ========================================================
    // CURRENT TARGET
    // ========================================================

    drawText(
        HUD_LEFT + 20,
        400,
        "CURRENT TARGET",
        {0.45f, 0.50f, 0.53f},
        GLUT_BITMAP_HELVETICA_10
    );

    drawText(
        HUD_LEFT + 145,
        400,
        "#4",
        {0.95f, 0.75f, 0.25f},
        GLUT_BITMAP_HELVETICA_12
    );


    // ========================================================
    // BATTERY
    // ========================================================

    drawText(
        HUD_LEFT + 20,
        360,
        "ROBOT BATTERY",
        {0.45f, 0.50f, 0.53f},
        GLUT_BITMAP_HELVETICA_10
    );

    drawText(
        HUD_LEFT + 145,
        360,
        "74%",
        {0.20f, 0.90f, 0.45f},
        GLUT_BITMAP_HELVETICA_12
    );


    // Battery bar
    drawRect(
        HUD_LEFT + 20,
        335,
        240,
        10,
        {0.10f, 0.14f, 0.15f}
    );

    drawRect(
        HUD_LEFT + 20,
        335,
        178,
        10,
        {0.20f, 0.80f, 0.40f}
    );


    // ========================================================
    // FIRE ZONES
    // ========================================================

    drawText(
        HUD_LEFT + 20,
        300,
        "FIRE ZONES",
        {0.45f, 0.50f, 0.53f},
        GLUT_BITMAP_HELVETICA_10
    );

    drawText(
        HUD_LEFT + 145,
        300,
        "7 ACTIVE",
        {1.0f, 0.30f, 0.10f},
        GLUT_BITMAP_HELVETICA_12
    );


    // ========================================================
    // STATUS
    // ========================================================

    drawText(
        HUD_LEFT + 20,
        250,
        "MISSION STATUS",
        {0.45f, 0.50f, 0.53f},
        GLUT_BITMAP_HELVETICA_10
    );

    drawRect(
        HUD_LEFT + 20,
        215,
        240,
        25,
        {0.08f, 0.12f, 0.13f}
    );

    drawText(
        HUD_LEFT + 35,
        223,
        "SEARCH & RESCUE",
        {0.35f, 0.85f, 0.88f},
        GLUT_BITMAP_HELVETICA_12
    );


    // ========================================================
    // CONTROLS
    // ========================================================

    drawText(
        HUD_LEFT + 20,
        175,
        "CONTROLS",
        {0.45f, 0.50f, 0.53f},
        GLUT_BITMAP_HELVETICA_10
    );

    drawText(
        HUD_LEFT + 20,
        150,
        "W A S D / ARROW KEYS",
        {0.75f, 0.78f, 0.80f},
        GLUT_BITMAP_HELVETICA_12
    );

    drawText(
        HUD_LEFT + 20,
        125,
        "ESC  -  EXIT SIMULATION",
        {0.45f, 0.50f, 0.53f},
        GLUT_BITMAP_HELVETICA_10
    );


    // ========================================================
    // SYSTEM FOOTER
    // ========================================================

    drawRect(
        HUD_LEFT,
        20,
        HUD_RIGHT - HUD_LEFT,
        70,
        {0.030f, 0.040f, 0.045f}
    );

    drawText(
        HUD_LEFT + 20,
        65,
        "SYSTEM",
        {0.40f, 0.45f, 0.48f},
        GLUT_BITMAP_HELVETICA_10
    );

    drawText(
        HUD_LEFT + 20,
        42,
        "ALL SYSTEMS OPERATIONAL",
        {0.20f, 0.85f, 0.45f},
        GLUT_BITMAP_HELVETICA_10
    );
}


// ============================================================
// DISPLAY
// ============================================================

void display()
{
    glClear(
        GL_COLOR_BUFFER_BIT
    );

    glLoadIdentity();


    // ========================================================
    // BACKGROUND
    // ========================================================

    drawRect(
        0,
        0,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        {0.025f, 0.035f, 0.040f}
    );


    // ========================================================
    // HEADER
    // ========================================================

    drawHeader();


    // ========================================================
    // MAP
    // ========================================================

    drawMapBackground();

    drawFireZoneMarkers();

    drawObstacles();

    drawRescueStation();


    // ========================================================
    // SURVIVORS
    // ========================================================

    for (
        const auto& survivor :
        survivors
    )
    {
        if (!survivor.rescued)
        {
            drawSurvivor(
                survivor.x,
                survivor.y
            );
        }
    }


    // ========================================================
    // FIRE
    // ========================================================

    drawFires();


    // ========================================================
    // ROBOT
    // ========================================================

    drawRobot();


    // ========================================================
    // MAP LABEL
    // ========================================================

    drawText(
        40,
        605,
        "SECTOR A // ACTIVE EMERGENCY ZONE",
        {0.30f, 0.65f, 0.68f},
        GLUT_BITMAP_HELVETICA_10
    );


    // ========================================================
    // HUD
    // ========================================================

    drawHUD();


    // ========================================================
    // PRESENT FRAME
    // ========================================================

    glutSwapBuffers();
}


// ============================================================
// FIRE ANIMATION
// ============================================================

void updateFire(
    int value
)
{
    currentFireFrame++;

    if (
        currentFireFrame >= 5
    )
    {
        currentFireFrame = 0;
    }

    glutPostRedisplay();

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


    // Keep robot inside map

    robotX = max(
        MAP_LEFT + 35.0f,
        min(
            robotX,
            MAP_RIGHT - 35.0f
        )
    );

    robotY = max(
        MAP_BOTTOM + 55.0f,
        min(
            robotY,
            MAP_TOP - 90.0f
        )
    );


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


    // Keep robot inside map

    robotX = max(
        MAP_LEFT + 35.0f,
        min(
            robotX,
            MAP_RIGHT - 35.0f
        )
    );

    robotY = max(
        MAP_BOTTOM + 55.0f,
        min(
            robotY,
            MAP_TOP - 90.0f
        )
    );


    glutPostRedisplay();
}


// ============================================================
// INITIALIZATION
// ============================================================

void init()
{
    glClearColor(
        0.025f,
        0.035f,
        0.040f,
        1.0f
    );


    // ========================================================
    // 2D PROJECTION
    // ========================================================

    glMatrixMode(
        GL_PROJECTION
    );

    glLoadIdentity();

    gluOrtho2D(
        0,
        WINDOW_WIDTH,
        0,
        WINDOW_HEIGHT
    );


    glMatrixMode(
        GL_MODELVIEW
    );

    glLoadIdentity();


    // ========================================================
    // TRANSPARENCY
    // ========================================================

    glEnable(
        GL_BLEND
    );

    glBlendFunc(
        GL_SRC_ALPHA,
        GL_ONE_MINUS_SRC_ALPHA
    );


    // ========================================================
    // FIRE TEXTURES
    // ========================================================

    fireTextures[0] =
        loadTexture(
            "assets/fire1.png"
        );

    fireTextures[1] =
        loadTexture(
            "assets/fire2.png"
        );

    fireTextures[2] =
        loadTexture(
            "assets/fire3.png"
        );

    fireTextures[3] =
        loadTexture(
            "assets/fire4.png"
        );

    fireTextures[4] =
        loadTexture(
            "assets/fire5.png"
        );
}


// ============================================================
// MAIN
// ============================================================

int main(
    int argc,
    char** argv
)
{
    glutInit(
        &argc,
        argv
    );


    glutInitDisplayMode(
        GLUT_DOUBLE |
        GLUT_RGBA
    );


    glutInitWindowSize(
        WINDOW_WIDTH,
        WINDOW_HEIGHT
    );


    glutInitWindowPosition(
        50,
        30
    );


    glutCreateWindow(
        "RescueX - Intelligent Emergency Response Simulation"
    );


    init();


    // CALLBACKS

    glutDisplayFunc(
        display
    );

    glutKeyboardFunc(
        keyboard
    );

    glutSpecialFunc(
        specialKeys
    );


    // FIRE ANIMATION

    glutTimerFunc(
        120,
        updateFire,
        0
    );

    // START

    glutMainLoop();


    return 0;
}

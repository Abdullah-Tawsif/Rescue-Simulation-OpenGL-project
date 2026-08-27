#include <GL/freeglut.h>
#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <string>
#include <algorithm>
#include <limits>

#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"

using namespace std;

const int WINDOW_WIDTH  = 1261;
const int WINDOW_HEIGHT = 700;

const float MAP_LEFT   = 20.0f;
const float MAP_BOTTOM = 20.0f;
const float MAP_RIGHT  = 915.0f;
const float MAP_TOP    = 620.0f;

const float HUD_LEFT   = 935.0f;
const float HUD_RIGHT  = 1240.0f;
const float HUD_BOTTOM = 20.0f;
const float HUD_TOP    = 620.0f;

const int GRID_SIZE = 20;
const float ROBOT_RADIUS = 13.0f;
const float ROBOT_SPEED = 1.9f;
const float WAYPOINT_REACH = 4.0f;

struct Color {
    float r, g, b;
};

struct Rect {
    float x, y, w, h;
};

struct Survivor {
    float x, y;
    bool rescued;
    bool carried;
};

struct FireZone {
    float x, y, w, h;
};

struct Node {
    int x, y;
    float g, f;
    bool operator>(const Node& other) const {
        return f > other.f;
    }
};

enum MissionState {
    MISSION_READY,
    MISSION_TO_SURVIVOR,
    MISSION_RETURNING,
    MISSION_WAITING,
    MISSION_COMPLETE,
    MISSION_PAUSED,
    MISSION_FAILED_FIRE,
    MISSION_FAILED_BATTERY
};

GLuint fireTextures[5] = {};
int currentFireFrame = 0;

float robotX = 85.0f;
float robotY = 350.0f;

const float BASE_X = 97.0f;
const float BASE_Y = 100.0f;

float battery = 100.0f;
int currentTarget = -1;
int totalRescued = 0;
bool carryingSurvivor = false;
MissionState missionState = MISSION_READY;

bool manualControlMode = false;

// MENU SYSTEM

const float UI_W = 1261.0f;
const float UI_H = 700.0f;

enum AppScreen {
    SCREEN_MENU,
    SCREEN_CONTROLS,
    SCREEN_HELP,
    SCREEN_GAME
};

AppScreen appScreen = SCREEN_MENU;

int viewX = 0, viewY = 0, viewW = (int)UI_W, viewH = (int)UI_H;
float viewScale = 1.0f;

float mouseX = 0.0f;
float mouseY = 0.0f;

bool fullscreenMode = false;
int windowedWidth = (int)UI_W;
int windowedHeight = (int)UI_H;
int windowedX = 50;
int windowedY = 30;

int endPopupStarted = 0;
const int END_POPUP_DURATION_MS = 5000;


vector<pair<float,float>> currentPath;
size_t pathIndex = 0;

string missionMessage = "PRESS ENTER / SPACE TO START";
string waitingReason = "";

vector<Survivor> survivors = {
    { 580, 540, false, false }, // S1
    { 860, 540, false, false }, // S2
    { 580, 370, false, false }, // S3
    { 590,  70, false, false }, // S4
    { 860,  60, false, false }  // S5
};

vector<FireZone> fires = {
    { 670, 440, 40, 50 }, // F1
    { 280,  25, 60, 80 }, // F2
    { 350,  25, 60, 80 }, // F3
    { 420,  25, 60, 80 }, // F4
    { 450, 550, 40, 60 }, // F5
    { 850, 170, 60, 95 }  // F6
};

vector<Rect> obstacles = {
    { 160, 450, 40, 170 }, // L1
    { 160,  20, 40, 300 }, // L2
    { 300, 120, 40, 370 }, // L3
    { 500, 280, 40, 340 }, // L4
    { 540, 450, 120, 40 },  // L5
    { 540, 280, 120, 40 },  // L6
    { 500,  20, 40, 140 },  // L7
    { 660,  20, 40, 140 },  // L8
    { 820, 450, 95, 40 },   // L9
    { 760, 280, 155, 40 },  // L10
    { 820, 120, 95, 40 }    // L11
};

// BASIC DRAWING

void drawRect(float x, float y, float w, float h, Color c) {
    glDisable(GL_TEXTURE_2D);
    glColor3f(c.r, c.g, c.b);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void drawOutline(float x, float y, float w, float h, Color c, float thickness = 2.0f) {
    glDisable(GL_TEXTURE_2D);
    glColor3f(c.r, c.g, c.b);
    glLineWidth(thickness);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void drawText(float x, float y, const string& text, Color color,
              void* font = GLUT_BITMAP_HELVETICA_12) {
    glDisable(GL_TEXTURE_2D);
    glColor3f(color.r, color.g, color.b);


    void* actualFont = font;
    if (font == GLUT_BITMAP_HELVETICA_10)
        actualFont = GLUT_BITMAP_HELVETICA_12;
    else if (font == GLUT_BITMAP_HELVETICA_12)
        actualFont = GLUT_BITMAP_HELVETICA_18;

    glRasterPos2f(x + 1.0f, y);
    for (char c : text) glutBitmapCharacter(actualFont, c);

    glRasterPos2f(x, y);
    for (char c : text) glutBitmapCharacter(actualFont, c);
}

void drawCenteredText(float x, float y, const string& text, Color color,
                      void* font = GLUT_BITMAP_HELVETICA_12) {
    void* actualFont = font;
    if (font == GLUT_BITMAP_HELVETICA_10)
        actualFont = GLUT_BITMAP_HELVETICA_12;
    else if (font == GLUT_BITMAP_HELVETICA_12)
        actualFont = GLUT_BITMAP_HELVETICA_18;

    int width = 0;
    for (char c : text) width += glutBitmapWidth(actualFont, c);
    drawText(x - width / 2.0f, y, text, color, font);
}


void drawBoldText(float x, float y, const string& text, Color color,
                  void* font = GLUT_BITMAP_HELVETICA_18) {
    drawText(x, y, text, color, font);
}

void drawCenteredBoldText(float x, float y, const string& text, Color color,
                          void* font = GLUT_BITMAP_HELVETICA_18) {
    void* actualFont = font;
    if (font == GLUT_BITMAP_HELVETICA_10)
        actualFont = GLUT_BITMAP_HELVETICA_12;
    else if (font == GLUT_BITMAP_HELVETICA_12)
        actualFont = GLUT_BITMAP_HELVETICA_18;

    int width = 0;
    for (char c : text) width += glutBitmapWidth(actualFont, c);
    drawBoldText(x - width / 2.0f, y, text, color, font);
}

bool pointInRect(float px, float py, float x, float y, float w, float h) {
    return px >= x && px <= x + w && py >= y && py <= y + h;
}

void drawRoundedPanel(float x, float y, float w, float h, Color fill, Color border) {
    drawRect(x + 4, y - 4, w, h, {0.01f, 0.015f, 0.020f});
    drawRect(x, y, w, h, fill);
    drawOutline(x, y, w, h, border, 2.5f);
}

void drawButton(float x, float y, float w, float h,
                const string& label, bool hovered,
                Color accent = {0.20f, 0.72f, 0.78f}) {
    Color fill = hovered
        ? Color{0.11f, 0.25f, 0.29f}
        : Color{0.055f, 0.095f, 0.110f};

    Color border = hovered
        ? Color{0.35f, 0.90f, 0.92f}
        : Color{0.16f, 0.38f, 0.42f};

    drawRect(x, y, w, h, fill);
    drawOutline(x, y, w, h, border, hovered ? 3.0f : 2.0f);

    if (hovered)
        drawRect(x, y, 5, h, accent);

    drawCenteredBoldText(
        x + w / 2.0f,
        y + h / 2.0f - 6,
        label,
        hovered ? Color{0.92f, 0.98f, 0.98f}
                : Color{0.78f, 0.86f, 0.87f},
        GLUT_BITMAP_HELVETICA_18
    );
}

void restartMission();
void beginMission();
void returnToMenu() {
    restartMission();
    appScreen = SCREEN_MENU;
    endPopupStarted = 0;
}

void startGameFromMenu() {
    restartMission();
    appScreen = SCREEN_GAME;
    beginMission();
}

void drawMenuBackground() {
    drawRect(0, 0, UI_W, UI_H, {0.018f, 0.028f, 0.034f});

    glColor3f(0.035f, 0.075f, 0.085f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (float x = 0; x <= UI_W; x += 50) {
        glVertex2f(x, 0); glVertex2f(x, UI_H);
    }
    for (float y = 0; y <= UI_H; y += 50) {
        glVertex2f(0, y); glVertex2f(UI_W, y);
    }
    glEnd();

    drawRect(0, 0, UI_W, 6, {0.10f, 0.45f, 0.50f});
    drawRect(0, UI_H - 6, UI_W, 6, {0.10f, 0.45f, 0.50f});

    drawRect(875, 80, 290, 510, {0.025f, 0.060f, 0.068f});
    drawOutline(875, 80, 290, 510, {0.08f, 0.24f, 0.27f}, 1.5f);
}

void drawMenu() {
    drawMenuBackground();

    drawText(80, 610, "RESCUEX",
             {0.35f, 0.92f, 0.95f}, GLUT_BITMAP_HELVETICA_18);
    drawBoldText(80, 555, "INTELLIGENT",
                 {0.92f, 0.96f, 0.96f}, GLUT_BITMAP_HELVETICA_18);
    drawBoldText(80, 520, "EMERGENCY RESPONSE",
                 {0.92f, 0.96f, 0.96f}, GLUT_BITMAP_HELVETICA_18);

    drawText(80, 485,
             "A* powered 2D rescue and navigation simulation",
             {0.45f, 0.62f, 0.65f}, GLUT_BITMAP_HELVETICA_12);

    drawRect(80, 455, 510, 2, {0.15f, 0.55f, 0.60f});

    /// Feature cards.
    const float cardY = 330;
    const float cardW = 150;
    const float gap = 15;
    const char* titles[3] = {"A* NAVIGATION", "FIRE AVOIDANCE", "SURVIVOR RESCUE"};
    const char* subs[3] = {"Smart path planning", "Collision detection", "Pickup & return"};

    for (int i = 0; i < 3; ++i) {
        float x = 80 + i * (cardW + gap);
        drawRoundedPanel(x, cardY, cardW, 85,
                         {0.045f, 0.075f, 0.085f},
                         {0.12f, 0.32f, 0.35f});
        drawBoldText(x + 14, cardY + 54, titles[i],
                     {0.35f, 0.82f, 0.85f}, GLUT_BITMAP_HELVETICA_10);
        drawText(x + 14, cardY + 30, subs[i],
                 {0.55f, 0.63f, 0.65f}, GLUT_BITMAP_HELVETICA_10);
    }

    /// Main menu buttons.
    bool hs = pointInRect(mouseX, mouseY, 80, 205, 510, 62);
    bool hc = pointInRect(mouseX, mouseY, 80, 130, 245, 55);
    bool hh = pointInRect(mouseX, mouseY, 345, 130, 245, 55);

    drawButton(80, 205, 510, 62, "START MISSION", hs,
               {0.20f, 0.85f, 0.48f});
    drawButton(80, 130, 245, 55, "CONTROLS", hc);
    drawButton(345, 130, 245, 55, "HELP", hh);

    drawText(80, 82, "F11  FULLSCREEN",
             {0.35f, 0.50f, 0.53f}, GLUT_BITMAP_HELVETICA_10);
    drawText(80, 62, "ESC  EXIT",
             {0.35f, 0.50f, 0.53f}, GLUT_BITMAP_HELVETICA_10);

    /// Right information
    drawRoundedPanel(915, 120, 210, 430,
                     {0.035f, 0.065f, 0.075f},
                     {0.12f, 0.35f, 0.38f});

    drawCenteredBoldText(1020, 510, "MISSION BRIEF",
                         {0.35f, 0.85f, 0.88f},
                         GLUT_BITMAP_HELVETICA_12);

    drawCenteredText(1020, 465, "Navigate the rescue robot", 
                     {0.65f, 0.72f, 0.74f}, GLUT_BITMAP_HELVETICA_10);
    drawCenteredText(1020, 445, "through obstacles and fire.",
                     {0.65f, 0.72f, 0.74f}, GLUT_BITMAP_HELVETICA_10);

    drawCenteredText(1020, 390, "RESCUE", {0.30f, 0.92f, 0.50f},
                     GLUT_BITMAP_HELVETICA_18);
    drawCenteredText(1020, 360, "5 SURVIVORS", {0.90f, 0.90f, 0.90f},
                     GLUT_BITMAP_HELVETICA_12);

    drawCenteredText(1020, 310, "AVOID", {1.0f, 0.45f, 0.20f},
                     GLUT_BITMAP_HELVETICA_18);
    drawCenteredText(1020, 280, "ACTIVE FIRE ZONES", {0.90f, 0.90f, 0.90f},
                     GLUT_BITMAP_HELVETICA_12);

    drawCenteredText(1020, 230, "MANAGE", {0.95f, 0.78f, 0.25f},
                     GLUT_BITMAP_HELVETICA_18);
    drawCenteredText(1020, 200, "ROBOT BATTERY", {0.90f, 0.90f, 0.90f},
                     GLUT_BITMAP_HELVETICA_12);

    drawCenteredText(1020, 145, "A*  •  GLUT  •  2D SIMULATION",
                     {0.30f, 0.65f, 0.68f}, GLUT_BITMAP_HELVETICA_10);
}

void drawInfoPage(bool controlsPage) {
    drawMenuBackground();

    const char* title = controlsPage ? "CONTROLS" : "HOW TO PLAY";
    const char* subtitle = controlsPage
        ? "Everything you need to operate the rescue robot."
        : "Complete the rescue mission and bring every survivor home.";

    drawBoldText(80, 610, title,
                 {0.38f, 0.90f, 0.93f}, GLUT_BITMAP_HELVETICA_18);
    drawText(80, 575, subtitle,
             {0.55f, 0.67f, 0.69f}, GLUT_BITMAP_HELVETICA_12);

    drawRoundedPanel(80, 120, 1100, 400,
                     {0.035f, 0.060f, 0.070f},
                     {0.12f, 0.35f, 0.38f});

    if (controlsPage) {
        drawBoldText(125, 465, "KEYBOARD",
                     {0.95f, 0.85f, 0.30f}, GLUT_BITMAP_HELVETICA_18);

        drawText(125, 420, "M", {0.35f, 0.90f, 0.92f}, GLUT_BITMAP_HELVETICA_18);
        drawText(190, 422, "Toggle AUTO A* / MANUAL control",
                 {0.78f, 0.84f, 0.85f}, GLUT_BITMAP_HELVETICA_12);

        drawText(125, 380, "W A S D", {0.35f, 0.90f, 0.92f}, GLUT_BITMAP_HELVETICA_18);
        drawText(190, 382, "Move robot manually",
                 {0.78f, 0.84f, 0.85f}, GLUT_BITMAP_HELVETICA_12);

        drawText(125, 340, "ARROWS", {0.35f, 0.90f, 0.92f}, GLUT_BITMAP_HELVETICA_18);
        drawText(190, 342, "Move robot manually",
                 {0.78f, 0.84f, 0.85f}, GLUT_BITMAP_HELVETICA_12);

        drawText(125, 300, "P", {0.35f, 0.90f, 0.92f}, GLUT_BITMAP_HELVETICA_18);
        drawText(190, 302, "Pause / resume automatic navigation",
                 {0.78f, 0.84f, 0.85f}, GLUT_BITMAP_HELVETICA_12);

        drawText(125, 260, "R", {0.35f, 0.90f, 0.92f}, GLUT_BITMAP_HELVETICA_18);
        drawText(190, 262, "Reset mission",
                 {0.78f, 0.84f, 0.85f}, GLUT_BITMAP_HELVETICA_12);

        drawText(650, 420, "ENTER / SPACE",
                 {0.35f, 0.90f, 0.92f}, GLUT_BITMAP_HELVETICA_18);
        drawText(650, 382, "Start mission from the menu",
                 {0.78f, 0.84f, 0.85f}, GLUT_BITMAP_HELVETICA_12);

        drawText(650, 340, "ESC",
                 {0.35f, 0.90f, 0.92f}, GLUT_BITMAP_HELVETICA_18);
        drawText(650, 302, "Exit the simulation",
                 {0.78f, 0.84f, 0.85f}, GLUT_BITMAP_HELVETICA_12);

        drawBoldText(650, 245, "TIP",
                     {0.95f, 0.78f, 0.25f}, GLUT_BITMAP_HELVETICA_18);
        drawText(650, 210, "Use M whenever you want to take",
                 {0.65f, 0.72f, 0.74f}, GLUT_BITMAP_HELVETICA_10);
        drawText(650, 190, "direct control of the robot.",
                 {0.65f, 0.72f, 0.74f}, GLUT_BITMAP_HELVETICA_10);
    } else {
        drawBoldText(125, 465, "MISSION OBJECTIVE",
                     {0.95f, 0.85f, 0.30f}, GLUT_BITMAP_HELVETICA_18);

        drawText(125, 420, "1.", {0.35f, 0.90f, 0.92f}, GLUT_BITMAP_HELVETICA_18);
        drawText(165, 422, "Start the mission and let A* select a safe route.",
                 {0.78f, 0.84f, 0.85f}, GLUT_BITMAP_HELVETICA_12);

        drawText(125, 380, "2.", {0.35f, 0.90f, 0.92f}, GLUT_BITMAP_HELVETICA_18);
        drawText(165, 382, "The robot travels to the current survivor.",
                 {0.78f, 0.84f, 0.85f}, GLUT_BITMAP_HELVETICA_12);

        drawText(125, 340, "3.", {0.35f, 0.90f, 0.92f}, GLUT_BITMAP_HELVETICA_18);
        drawText(165, 342, "It picks them up and automatically returns to base.",
                 {0.78f, 0.84f, 0.85f}, GLUT_BITMAP_HELVETICA_12);

        drawText(125, 300, "4.", {0.35f, 0.90f, 0.92f}, GLUT_BITMAP_HELVETICA_18);
        drawText(165, 302, "Repeat until all five survivors are rescued.",
                 {0.78f, 0.84f, 0.85f}, GLUT_BITMAP_HELVETICA_12);

        drawText(650, 420, "WARNING",
                 {1.0f, 0.40f, 0.15f}, GLUT_BITMAP_HELVETICA_18);
        drawText(650, 382, "Fire contact causes immediate mission failure.",
                 {0.78f, 0.84f, 0.85f}, GLUT_BITMAP_HELVETICA_10);
        drawText(650, 345, "If the battery reaches 0%, the mission fails.",
                 {0.78f, 0.84f, 0.85f}, GLUT_BITMAP_HELVETICA_10);

        drawBoldText(650, 285, "A*",
                     {0.35f, 0.90f, 0.92f}, GLUT_BITMAP_HELVETICA_18);
        drawText(690, 287, "finds a path around blocked cells.",
                 {0.65f, 0.72f, 0.74f}, GLUT_BITMAP_HELVETICA_10);
    }

    bool hb = pointInRect(mouseX, mouseY, 80, 45, 180, 52);
    drawButton(80, 45, 180, 52, "BACK TO MENU", hb);
}

void showEndPopup() {
    if (endPopupStarted == 0)
        endPopupStarted = glutGet(GLUT_ELAPSED_TIME);
}

float dist2(float ax, float ay, float bx, float by) {
    float dx = ax - bx;
    float dy = ay - by;
    return dx * dx + dy * dy;
}

// /COLLISION

bool circleIntersectsRect(float cx, float cy, float radius, const Rect& r) {
    float closestX = max(r.x, min(cx, r.x + r.w));
    float closestY = max(r.y, min(cy, r.y + r.h));
    return dist2(cx, cy, closestX, closestY) <= radius * radius;
}

bool robotHitsObstacle(float x, float y) {
    for (const Rect& r : obstacles)
        if (circleIntersectsRect(x, y, ROBOT_RADIUS, r))
            return true;
    return false;
}

bool robotHitsFire(float x, float y) {
    for (const FireZone& f : fires) {
        Rect r{f.x, f.y, f.w, f.h};
        if (circleIntersectsRect(x, y, ROBOT_RADIUS + 4.0f, r))
            return true;
    }
    return false;
}

bool robotInsideMap(float x, float y) {
    return x >= MAP_LEFT + ROBOT_RADIUS &&
           x <= MAP_RIGHT - ROBOT_RADIUS &&
           y >= MAP_BOTTOM + ROBOT_RADIUS &&
           y <= MAP_TOP - ROBOT_RADIUS;
}

bool positionBlocked(float x, float y) {
    return !robotInsideMap(x, y) ||
           robotHitsObstacle(x, y) ||
           robotHitsFire(x, y);
}

// A* GRID

const int GRID_COLS = (int)((MAP_RIGHT - MAP_LEFT) / GRID_SIZE) + 1;
const int GRID_ROWS = (int)((MAP_TOP - MAP_BOTTOM) / GRID_SIZE) + 1;

struct GridPoint {
    int x, y;
};

GridPoint worldToGrid(float x, float y) {
    GridPoint p;
    p.x = (int)round((x - MAP_LEFT) / GRID_SIZE);
    p.y = (int)round((y - MAP_BOTTOM) / GRID_SIZE);
    p.x = max(0, min(GRID_COLS - 1, p.x));
    p.y = max(0, min(GRID_ROWS - 1, p.y));
    return p;
}

pair<float,float> gridToWorld(int gx, int gy) {
    return {
        MAP_LEFT + gx * GRID_SIZE,
        MAP_BOTTOM + gy * GRID_SIZE
    };
}

bool gridBlocked(int gx, int gy) {
    if (gx < 0 || gx >= GRID_COLS || gy < 0 || gy >= GRID_ROWS)
        return true;

    auto p = gridToWorld(gx, gy);
    return positionBlocked(p.first, p.second);
}

float heuristic(int x1, int y1, int x2, int y2) {
    return (float)(abs(x1 - x2) + abs(y1 - y2));
}

vector<pair<float,float>> findAStarPath(float sx, float sy, float gx, float gy) {
    GridPoint start = worldToGrid(sx, sy);
    GridPoint goal  = worldToGrid(gx, gy);

    if (gridBlocked(goal.x, goal.y)) {
        bool found = false;
        for (int radius = 1; radius <= 5 && !found; ++radius) {
            for (int dy = -radius; dy <= radius && !found; ++dy) {
                for (int dx = -radius; dx <= radius; ++dx) {
                    int nx = goal.x + dx;
                    int ny = goal.y + dy;
                    if (!gridBlocked(nx, ny)) {
                        goal = {nx, ny};
                        found = true;
                        break;
                    }
                }
            }
        }
        if (!found) return {};
    }

    if (gridBlocked(start.x, start.y))
        return {};

    const float INF = numeric_limits<float>::infinity();

    vector<vector<float>> gScore(
        GRID_COLS, vector<float>(GRID_ROWS, INF)
    );
    vector<vector<GridPoint>> parent(
        GRID_COLS, vector<GridPoint>(GRID_ROWS, {-1, -1})
    );
    vector<vector<bool>> closed(
        GRID_COLS, vector<bool>(GRID_ROWS, false)
    );

    priority_queue<Node, vector<Node>, greater<Node>> open;

    gScore[start.x][start.y] = 0.0f;
    open.push({start.x, start.y,
               0.0f,
               heuristic(start.x, start.y, goal.x, goal.y)});

    const int dirs[4][2] = {
        {1,0}, {-1,0}, {0,1}, {0,-1}
    };

    while (!open.empty()) {
        Node current = open.top();
        open.pop();

        if (closed[current.x][current.y])
            continue;

        closed[current.x][current.y] = true;

        if (current.x == goal.x && current.y == goal.y) {
            vector<GridPoint> gridPath;
            GridPoint p = goal;

            while (!(p.x == -1 && p.y == -1)) {
                gridPath.push_back(p);
                if (p.x == start.x && p.y == start.y) break;
                p = parent[p.x][p.y];
            }

            reverse(gridPath.begin(), gridPath.end());

            vector<pair<float,float>> result;
            for (auto& gp : gridPath)
                result.push_back(gridToWorld(gp.x, gp.y));

            vector<pair<float,float>> simplified;
            if (!result.empty()) {
                simplified.push_back(result.front());

                for (size_t i = 1; i + 1 < result.size(); ++i) {
                    float ax = result[i].first - result[i-1].first;
                    float ay = result[i].second - result[i-1].second;
                    float bx = result[i+1].first - result[i].first;
                    float by = result[i+1].second - result[i].second;

                    if (ax * by != ay * bx)
                        simplified.push_back(result[i]);
                }

                simplified.push_back(result.back());
            }

            return simplified;
        }

        for (auto& d : dirs) {
            int nx = current.x + d[0];
            int ny = current.y + d[1];

            if (gridBlocked(nx, ny) || closed[nx][ny])
                continue;

            float tentativeG = gScore[current.x][current.y] + 1.0f;

            if (tentativeG < gScore[nx][ny]) {
                gScore[nx][ny] = tentativeG;
                parent[nx][ny] = {current.x, current.y};

                float f =
                    tentativeG +
                    heuristic(nx, ny, goal.x, goal.y);

                open.push({nx, ny, tentativeG, f});
            }
        }
    }

    return {};
}

// MISSION MANAGEMENT

int chooseNearestSurvivor() {
    int best = -1;
    float bestDistance = numeric_limits<float>::max();

    for (int i = 0; i < (int)survivors.size(); ++i) {
        if (survivors[i].rescued)
            continue;

        float d = dist2(robotX, robotY, survivors[i].x, survivors[i].y);

        if (d < bestDistance) {
            bestDistance = d;
            best = i;
        }
    }

    return best;
}

void setWaiting(const string& reason) {
    missionState = MISSION_WAITING;
    waitingReason = reason;
    missionMessage = "WAITING FOR DIRECTION";
    currentPath.clear();
    pathIndex = 0;
}

bool createPathTo(float x, float y) {
    vector<pair<float,float>> newPath =
        findAStarPath(robotX, robotY, x, y);

    if (newPath.empty())
        return false;

    currentPath = newPath;
    pathIndex = 0;

    if (!currentPath.empty() &&
        dist2(robotX, robotY,
              currentPath[0].first,
              currentPath[0].second) <
            WAYPOINT_REACH * WAYPOINT_REACH) {
        pathIndex = 1;
    }

    return pathIndex < currentPath.size();
}

void startNextMission() {
    if (battery <= 1.0f) {
        setWaiting("BATTERY LOW");
        return;
    }

    int target = chooseNearestSurvivor();

    if (target == -1) {
        missionState = MISSION_COMPLETE;
        currentTarget = -1;
        missionMessage = "ALL SURVIVORS RESCUED";
        currentPath.clear();
        return;
    }

    currentTarget = target;
    carryingSurvivor = false;

    if (!createPathTo(survivors[target].x, survivors[target].y)) {
        setWaiting("NO SAFE PATH TO SURVIVOR");
        return;
    }

    missionState = MISSION_TO_SURVIVOR;
    missionMessage = "NAVIGATING TO SURVIVOR";
}

void beginMission() {
    if (missionState == MISSION_READY ||
        missionState == MISSION_COMPLETE ||
        missionState == MISSION_FAILED_FIRE ||
        missionState == MISSION_FAILED_BATTERY) {

        robotX = 85.0f;
        robotY = 350.0f;
        battery = 100.0f;
        currentTarget = -1;
        totalRescued = 0;
        carryingSurvivor = false;
        currentPath.clear();
        pathIndex = 0;

        for (auto& s : survivors) {
            s.rescued = false;
            s.carried = false;
        }

        startNextMission();
    }
}

void finishSurvivorPickup() {
    if (currentTarget < 0 ||
        currentTarget >= (int)survivors.size())
        return;

    survivors[currentTarget].carried = true;
    carryingSurvivor = true;

    missionMessage = "SURVIVOR SECURED";

    if (!createPathTo(BASE_X, BASE_Y)) {
        setWaiting("NO SAFE RETURN ROUTE");
        return;
    }

    missionState = MISSION_RETURNING;
}

void finishReturnToBase() {
    if (currentTarget >= 0 &&
        currentTarget < (int)survivors.size()) {

        survivors[currentTarget].carried = false;
        survivors[currentTarget].rescued = true;
        totalRescued++;
    }

    carryingSurvivor = false;
    currentTarget = -1;

    missionMessage = "SURVIVOR RESCUED";

    if (totalRescued >= (int)survivors.size()) {
        missionState = MISSION_COMPLETE;
        missionMessage = "MISSION COMPLETE";
        currentPath.clear();
        showEndPopup();
        return;
    }

    startNextMission();
}

// MOVEMENT othoba MANUAL OVERRIDE

bool tryMove(float dx, float dy) {
    float nx = robotX + dx;
    float ny = robotY + dy;

    if (positionBlocked(nx, ny))
        return false;

    robotX = nx;
    robotY = ny;

    battery -= (abs(dx) + abs(dy)) * 0.004f;
    battery = max(0.0f, battery);

    return true;
}

void failByFire();
void drawControlHint();
void failByBattery();
void drawMissionOverlay();
void drawStartOverlay();
void drawMenu();
void drawInfoPage(bool controlsPage);
void returnToMenu();
void startGameFromMenu();
void showEndPopup();
void restartMission();
void beginMission();
void drawControlHint();

void toggleControlMode() {
    if (missionState == MISSION_COMPLETE ||
        missionState == MISSION_FAILED_FIRE ||
        missionState == MISSION_FAILED_BATTERY ||
        missionState == MISSION_READY) {
        return;
    }

    manualControlMode = !manualControlMode;

    if (manualControlMode) {
        currentPath.clear();
        pathIndex = 0;
        missionState = MISSION_WAITING;
        missionMessage = "MANUAL CONTROL ACTIVE";
        waitingReason = "W/A/S/D OR ARROW KEYS";
    } else {
        // Return control to A* system and again calculate korbe fresh path
        /// from the robot's current position.
        if (carryingSurvivor) {
            if (createPathTo(BASE_X, BASE_Y)) {
                missionState = MISSION_RETURNING;
                missionMessage = "AUTOMATIC A* CONTROL ACTIVE";
                waitingReason.clear();
            } else {
                setWaiting("NO SAFE RETURN ROUTE");
            }
        } else if (currentTarget >= 0 &&
                   currentTarget < (int)survivors.size() &&
                   !survivors[currentTarget].rescued) {
            if (createPathTo(survivors[currentTarget].x,
                             survivors[currentTarget].y)) {
                missionState = MISSION_TO_SURVIVOR;
                missionMessage = "AUTOMATIC A* CONTROL ACTIVE";
                waitingReason.clear();
            } else {
                setWaiting("NO SAFE PATH TO SURVIVOR");
            }
        } else {
            startNextMission();
        }
    }
}

void manualMove(float dx, float dy) {
    if (missionState != MISSION_WAITING &&
        missionState != MISSION_PAUSED)
        return;

    if (battery <= 0.0f) {
        battery = 0.0f;
        failByBattery();
        return;
    }

    float nx=robotX+dx, ny=robotY+dy;
    if(robotHitsFire(nx,ny)){ robotX=nx; robotY=ny; failByFire(); return; }
    if(!tryMove(dx,dy)){ missionMessage="DIRECTION BLOCKED"; return; }
    if(battery<=0.0f){ battery=0.0f; failByBattery(); return; }
    missionState=MISSION_WAITING;
    missionMessage = "MANUAL OVERRIDE ACTIVE";

    // If user manually reaches target/base, continue mission.
    if (carryingSurvivor &&
        dist2(robotX, robotY, BASE_X, BASE_Y) < 45.0f * 45.0f) {
        finishReturnToBase();
        return;
    }

    if (!carryingSurvivor &&
        currentTarget >= 0 &&
        dist2(robotX, robotY,
              survivors[currentTarget].x,
              survivors[currentTarget].y) < 45.0f * 45.0f) {
        finishSurvivorPickup();
    }
}

void updateRobot(int value) {
    if (missionState == MISSION_TO_SURVIVOR ||
        missionState == MISSION_RETURNING) {

        if (pathIndex >= currentPath.size()) {
            if (missionState == MISSION_TO_SURVIVOR)
                finishSurvivorPickup();
            else
                finishReturnToBase();

            glutPostRedisplay();
            glutTimerFunc(16, updateRobot, 0);
            return;
        }

        float tx = currentPath[pathIndex].first;
        float ty = currentPath[pathIndex].second;

        float dx = tx - robotX;
        float dy = ty - robotY;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance <= WAYPOINT_REACH) {
            pathIndex++;
        } else {
            float step = min(ROBOT_SPEED, distance);

            float mx = dx / distance * step;
            float my = dy / distance * step;

            float nx=robotX+mx, ny=robotY+my;
            if(robotHitsFire(nx,ny)){ robotX=nx; robotY=ny; failByFire(); }
            else if(!tryMove(mx,my)){ setWaiting("PATH BLOCKED - CHOOSE DIRECTION"); }
        }
        if(battery<=0.0f && missionState!=MISSION_FAILED_FIRE){ battery=0.0f; failByBattery(); }
    }

    glutPostRedisplay();
    glutTimerFunc(16, updateRobot, 0);
}

// MAP

void drawGrid() {
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.11f, 0.15f, 0.18f);
    glLineWidth(1.0f);

    glBegin(GL_LINES);

    for (float x = MAP_LEFT; x <= MAP_RIGHT; x += 30.0f) {
        glVertex2f(x, MAP_BOTTOM);
        glVertex2f(x, MAP_TOP);
    }

    for (float y = MAP_BOTTOM; y <= MAP_TOP; y += 30.0f) {
        glVertex2f(MAP_LEFT, y);
        glVertex2f(MAP_RIGHT, y);
    }

    glEnd();
}

void drawMapBackground() {
    drawRect(
        MAP_LEFT, MAP_BOTTOM,
        MAP_RIGHT - MAP_LEFT,
        MAP_TOP - MAP_BOTTOM,
        {0.055f, 0.075f, 0.085f}
    );

    drawRect(
        MAP_LEFT + 5, MAP_BOTTOM + 5,
        MAP_RIGHT - MAP_LEFT - 10,
        MAP_TOP - MAP_BOTTOM - 10,
        {0.065f, 0.090f, 0.100f}
    );

    drawGrid();

    drawOutline(
        MAP_LEFT, MAP_BOTTOM,
        MAP_RIGHT - MAP_LEFT,
        MAP_TOP - MAP_BOTTOM,
        {0.18f, 0.55f, 0.62f},
        3.0f
    );
}

void drawSingleObstacle(const Rect& r) {
    drawRect(
        r.x - 2, r.y - 2,
        r.w + 4, r.h + 4,
        {0.08f, 0.09f, 0.10f}
    );

    drawRect(
        r.x, r.y, r.w, r.h,
        {0.24f, 0.28f, 0.31f}
    );

    drawRect(
        r.x, r.y + r.h - 5,
        r.w, 5,
        {0.38f, 0.43f, 0.46f}
    );

    glColor3f(0.17f, 0.20f, 0.22f);
    glLineWidth(1.5f);

    for (float x = r.x + 15; x < r.x + r.w; x += 25) {
        glBegin(GL_LINES);
        glVertex2f(x, r.y + 5);
        glVertex2f(x, r.y + r.h - 5);
        glEnd();
    }

    drawOutline(
        r.x, r.y, r.w, r.h,
        {0.45f, 0.50f, 0.53f},
        1.5f
    );
}

void drawObstacles() {
    for (const auto& r : obstacles)
        drawSingleObstacle(r);
}

void drawRescueStation() {
    float x=40.0f,y=60.0f;
    drawRect(x+5,y-5,115,92,{0.015f,0.02f,0.025f});
    drawRect(x,y,115,87,{0.78f,0.83f,0.85f});
    drawOutline(x,y,115,87,{0.20f,0.55f,0.60f},2.5f);
    glDisable(GL_TEXTURE_2D); glColor3f(0.72f,0.08f,0.10f);
    glBegin(GL_TRIANGLES); glVertex2f(x-8,y+87); glVertex2f(x+57.5f,y+123); glVertex2f(x+123,y+87); glEnd();
    drawRect(x+48,y+103,19,8,{0.92f,0.92f,0.92f}); drawRect(x+53,y+111,9,8,{0.85f,0.12f,0.12f});
    drawRect(x,y+68,115,10,{0.10f,0.48f,0.62f});
    drawRect(x+12,y+8,42,50,{0.16f,0.20f,0.22f}); drawOutline(x+12,y+8,42,50,{0.40f,0.46f,0.48f},1.5f);
    for(int i=1;i<5;i++){ glColor3f(0.30f,0.34f,0.36f); glLineWidth(1); glBegin(GL_LINES); glVertex2f(x+12,y+8+i*10); glVertex2f(x+54,y+8+i*10); glEnd(); }
    Color red{0.88f,0.08f,0.10f}; drawRect(x+77,y+29,13,31,red); drawRect(x+68,y+38,31,13,red);
    drawRect(x+67,y+8,39,17,{0.10f,0.22f,0.28f}); drawOutline(x+67,y+8,39,17,{0.45f,0.65f,0.70f},1.2f);
    drawText(x,y-18,"RESCUE BASE",{0.35f,0.85f,0.88f},GLUT_BITMAP_HELVETICA_12);
    drawText(x+26,y-31,"SAFE ZONE",{0.55f,0.62f,0.65f},GLUT_BITMAP_HELVETICA_10);
}

// FIRE

GLuint loadTexture(const char* filename) {
    int width, height, channels;

    unsigned char* image =
        stbi_load(filename, &width, &height, &channels, 4);

    if (!image) {
        cout << "Failed to load: " << filename << endl;
        return 0;
    }

    // Remove near-black background.
    for (int i = 0; i < width * height; ++i) {
        unsigned char* pixel = &image[i * 4];

        if (pixel[0] < 35 &&
            pixel[1] < 35 &&
            pixel[2] < 35) {
            pixel[3] = 0;
        }
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA,
        width, height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, image
    );

    stbi_image_free(image);
    return texture;
}

void drawFire(float x, float y, float w, float h) {
    GLuint texture = fireTextures[currentFireFrame];
    if (!texture) return;

   
    glDisable(GL_TEXTURE_2D);
    glColor4f(1.0f, 0.20f, 0.02f, 0.08f);

    glBegin(GL_QUADS);
    glVertex2f(x - 12, y - 8);
    glVertex2f(x + w + 12, y - 8);
    glVertex2f(x + w + 12, y + h + 8);
    glVertex2f(x - 12, y + h + 8);
    glEnd();

    // Fire texture.
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glColor4f(1, 1, 1, 1);

    glBegin(GL_QUADS);

    glTexCoord2f(0, 1);
    glVertex2f(x, y);

    glTexCoord2f(1, 1);
    glVertex2f(x + w, y);

    glTexCoord2f(1, 0);
    glVertex2f(x + w, y + h);

    glTexCoord2f(0, 0);
    glVertex2f(x, y + h);

    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

void drawFires() {
    for (const auto& f : fires)
        drawFire(f.x, f.y, f.w, f.h);
}

void drawFireMarkers() {
    glDisable(GL_TEXTURE_2D);

    for (const auto& f : fires) {
        glColor4f(1.0f, 0.20f, 0.05f, 0.35f);
        glLineWidth(1.5f);

        glBegin(GL_LINE_LOOP);
        glVertex2f(f.x - 5, f.y - 5);
        glVertex2f(f.x + f.w + 5, f.y - 5);
        glVertex2f(f.x + f.w + 5, f.y + f.h + 5);
        glVertex2f(f.x - 5, f.y + f.h + 5);
        glEnd();
    }
}

// SURVIVOR

void drawSurvivor(float x, float y, int number) {
    glDisable(GL_TEXTURE_2D);

    glColor4f(0.20f, 0.95f, 0.50f, 0.12f);
    glBegin(GL_LINE_LOOP);

    for (int i = 0; i < 32; ++i) {
        float a = 2.0f * 3.1415926f * i / 32.0f;
        glVertex2f(
            x + cos(a) * 25.0f,
            y + 30.0f + sin(a) * 25.0f
        );
    }

    glEnd();

    // Shadow.
    glColor4f(0, 0, 0, 0.35f);
    glBegin(GL_QUADS);
    glVertex2f(x - 18, y - 22);
    glVertex2f(x + 18, y - 22);
    glVertex2f(x + 18, y - 15);
    glVertex2f(x - 18, y - 15);
    glEnd();

    // Head.
    glColor3f(1.0f, 0.76f, 0.53f);
    glBegin(GL_POLYGON);

    for (int i = 0; i < 30; ++i) {
        float angle = 2.0f * 3.1415926f * i / 30.0f;
        glVertex2f(
            x + cos(angle) * 11,
            y + 42 + sin(angle) * 11
        );
    }

    glEnd();

    // Body.
    drawRect(
        x - 15, y, 30, 40,
        {0.16f, 0.67f, 0.30f}
    );

    drawRect(
        x - 15, y + 25, 30, 7,
        {0.20f, 0.80f, 0.40f}
    );

    // Arms-legs.
    glColor3f(0.15f, 0.18f, 0.20f);
    glLineWidth(3);

    glBegin(GL_LINES);

    glVertex2f(x - 15, y + 30);
    glVertex2f(x - 27, y + 17);

    glVertex2f(x + 15, y + 30);
    glVertex2f(x + 27, y + 17);

    glVertex2f(x - 7, y);
    glVertex2f(x - 7, y - 18);

    glVertex2f(x + 7, y);
    glVertex2f(x + 7, y - 18);

    glEnd();

    drawOutline(
        x - 22, y - 25, 44, 75,
        {0.25f, 0.85f, 0.45f},
        1.5f
    );

    drawText(
        x - 18, y - 38,
        "S" + to_string(number),
        {0.35f, 0.90f, 0.50f},
        GLUT_BITMAP_HELVETICA_10
    );
}

void drawCarriedSurvivor() {
    if (!carryingSurvivor || currentTarget < 0) return;

    /// Small survivor indicator above robot.
    float x = robotX;
    float y = robotY + 95;

    glColor3f(1.0f, 0.76f, 0.53f);
    glBegin(GL_POLYGON);

    for (int i = 0; i < 24; ++i) {
        float a = 2.0f * 3.1415926f * i / 24.0f;
        glVertex2f(
            x + cos(a) * 8,
            y + 12 + sin(a) * 8
        );
    }

    glEnd();

    drawRect(
        x - 10, y - 15, 20, 28,
        {0.16f, 0.67f, 0.30f}
    );

    drawText(
        x - 30, y + 28,
        "RESCUED",
        {0.30f, 0.95f, 0.50f},
        GLUT_BITMAP_HELVETICA_10
    );
}

// ROBOT

void drawRobot() {
    glDisable(GL_TEXTURE_2D);
    glColor4f(0, 0, 0, 0.35f);
    glBegin(GL_QUADS); glVertex2f(robotX-23,robotY-28); glVertex2f(robotX+23,robotY-28); glVertex2f(robotX+23,robotY-21); glVertex2f(robotX-23,robotY-21); glEnd();
    drawRect(robotX-20,robotY-27,13,11,{0.025f,0.03f,0.035f});
    drawRect(robotX+7,robotY-27,13,11,{0.025f,0.03f,0.035f});
    drawRect(robotX-17,robotY-24,6,6,{0.22f,0.70f,0.75f});
    drawRect(robotX+11,robotY-24,6,6,{0.22f,0.70f,0.75f});
    drawRect(robotX-22,robotY-15,44,42,{0.07f,0.25f,0.55f});
    drawRect(robotX-18,robotY+21,36,4,{0.18f,0.48f,0.82f});
    drawOutline(robotX-22,robotY-15,44,42,{0.30f,0.65f,0.78f},2.0f);
    drawRect(robotX-17,robotY+27,34,27,{0.45f,0.75f,0.80f});
    drawOutline(robotX-17,robotY+27,34,27,{0.05f,0.12f,0.15f},2.5f);
    drawRect(robotX-11,robotY+35,22,13,{0.03f,0.12f,0.15f});
    drawRect(robotX-8,robotY+38,5,5,{0.20f,0.90f,0.90f}); drawRect(robotX+3,robotY+38,5,5,{0.20f,0.90f,0.90f});
    glColor3f(0.05f,0.08f,0.10f); glLineWidth(2.5f); glBegin(GL_LINES); glVertex2f(robotX,robotY+54); glVertex2f(robotX,robotY+65); glEnd();
    drawRect(robotX-4,robotY+64,8,8,missionState==MISSION_WAITING?Color{1.0f,0.70f,0.05f}:Color{0.95f,0.10f,0.12f});
    glColor3f(0.12f,0.32f,0.62f); glLineWidth(5); glBegin(GL_LINES); glVertex2f(robotX-21,robotY+12); glVertex2f(robotX-25,robotY-1); glVertex2f(robotX+21,robotY+12); glVertex2f(robotX+25,robotY-1); glEnd();
    glPointSize(5); glBegin(GL_POINTS); glVertex2f(robotX-25,robotY-1); glVertex2f(robotX+25,robotY-1); glEnd();
    drawText(robotX-20,robotY-43,"ROBOT",{0.30f,0.80f,0.85f},GLUT_BITMAP_HELVETICA_10);
}

// PATH VISUAL

void drawPath() {
    if (currentPath.size() < 2)
        return;

    glDisable(GL_TEXTURE_2D);

    glColor4f(0.20f, 0.80f, 0.90f, 0.45f);
    glLineWidth(2.0f);

    glBegin(GL_LINE_STRIP);

    glVertex2f(robotX, robotY);

    for (size_t i = pathIndex; i < currentPath.size(); ++i)
        glVertex2f(currentPath[i].first, currentPath[i].second);

    glEnd();

    // Target marker.
    if (currentTarget >= 0 &&
        missionState == MISSION_TO_SURVIVOR) {

        float tx = survivors[currentTarget].x;
        float ty = survivors[currentTarget].y;

        glColor4f(1.0f, 0.80f, 0.15f, 0.55f);
        glLineWidth(2.0f);

        glBegin(GL_LINE_LOOP);

        for (int i = 0; i < 32; ++i) {
            float a = 2.0f * 3.1415926f * i / 32.0f;
            glVertex2f(
                tx + cos(a) * 30,
                ty + 30 + sin(a) * 30
            );
        }

        glEnd();
    }
}

// HEADER - HUD

void drawHeader() {
    drawRect(
        20, 635, 1220, 45,
        {0.035f, 0.055f, 0.065f}
    );

    drawRect(
        20, 635, 6, 45,
        {0.20f, 0.80f, 0.85f}
    );

    drawText(
        40, 658,
        "RESCUEX",
        {0.40f, 0.90f, 0.92f},
        GLUT_BITMAP_HELVETICA_18
    );

    drawText(
        130, 658,
        "// INTELLIGENT EMERGENCY RESPONSE SIMULATION",
        {0.55f, 0.62f, 0.65f},
        GLUT_BITMAP_HELVETICA_10
    );

    glColor3f(
        missionState == MISSION_COMPLETE
            ? 0.20f
            : 0.15f,
        0.95f,
        0.40f
    );

    glPointSize(8);

    glBegin(GL_POINTS);
    glVertex2f(850, 657);
    glEnd();

    drawText(
        865, 653,
        missionState == MISSION_COMPLETE
            ? "MISSION COMPLETE"
            : "SIMULATION LIVE",
        {0.30f, 0.90f, 0.45f},
        GLUT_BITMAP_HELVETICA_10
    );
}

string stateText() {
    switch (missionState) {
        case MISSION_READY:      return "READY";
        case MISSION_TO_SURVIVOR:return "NAVIGATING";
        case MISSION_RETURNING:  return "RETURNING";
        case MISSION_WAITING:    return "WAITING";
        case MISSION_COMPLETE:   return "COMPLETE";
        case MISSION_PAUSED:     return "PAUSED";
        case MISSION_FAILED_FIRE:
        case MISSION_FAILED_BATTERY: return "FAILED";
    }
    return "UNKNOWN";
}

string targetText() {
    if (currentTarget < 0)
        return "--";

    return "#" + to_string(currentTarget + 1);
}

void drawHUD() {
    drawRect(
        HUD_LEFT, HUD_BOTTOM,
        HUD_RIGHT - HUD_LEFT,
        HUD_TOP - HUD_BOTTOM,
        {0.045f, 0.060f, 0.070f}
    );

    drawOutline(
        HUD_LEFT, HUD_BOTTOM,
        HUD_RIGHT - HUD_LEFT,
        HUD_TOP - HUD_BOTTOM,
        {0.20f, 0.45f, 0.50f},
        2.0f
    );

    drawRect(
        HUD_LEFT, 565,
        HUD_RIGHT - HUD_LEFT, 55,
        {0.065f, 0.095f, 0.105f}
    );

    drawText(
        HUD_LEFT + 20, 595,
        "MISSION CONTROL",
        {0.40f, 0.90f, 0.92f},
        GLUT_BITMAP_HELVETICA_18
    );

    drawText(
        HUD_LEFT + 20, 577,
        "AUTONOMOUS RESCUE UNIT",
        {0.45f, 0.50f, 0.53f},
        GLUT_BITMAP_HELVETICA_10
    );

    drawText(
        HUD_LEFT + 20, 535,
        "AI STATUS",
        {0.65f, 0.70f, 0.72f},
        GLUT_BITMAP_HELVETICA_12
    );

    drawText(
        HUD_LEFT + 145, 535,
        missionState == MISSION_WAITING ? "WAITING" : "ACTIVE",
        missionState == MISSION_WAITING
            ? Color{1.0f, 0.70f, 0.05f}
            : Color{0.20f, 0.90f, 0.45f},
        GLUT_BITMAP_HELVETICA_12
    );

    drawText(
        HUD_LEFT + 20, 505,
        "ALGORITHM",
        {0.65f, 0.70f, 0.72f},
        GLUT_BITMAP_HELVETICA_12
    );

    drawText(
        HUD_LEFT + 145, 505,
        "A*",
        {0.30f, 0.80f, 0.90f},
        GLUT_BITMAP_HELVETICA_12
    );

    drawText(
        HUD_LEFT + 20, 465,
        "SURVIVORS",
        {0.65f, 0.70f, 0.72f},
        GLUT_BITMAP_HELVETICA_12
    );

    drawText(
        HUD_LEFT + 145, 465,
        "5",
        {0.90f, 0.90f, 0.90f},
        GLUT_BITMAP_HELVETICA_12
    );

    drawText(
        HUD_LEFT + 20, 440,
        "RESCUED",
        {0.65f, 0.70f, 0.72f},
        GLUT_BITMAP_HELVETICA_12
    );

    drawText(
        HUD_LEFT + 145, 440,
        to_string(totalRescued),
        {0.20f, 0.90f, 0.45f},
        GLUT_BITMAP_HELVETICA_12
    );

    drawText(
        HUD_LEFT + 20, 400,
        "CURRENT TARGET",
        {0.65f, 0.70f, 0.72f},
        GLUT_BITMAP_HELVETICA_12
    );

    drawText(
        HUD_LEFT + 145, 400,
        targetText(),
        {0.95f, 0.75f, 0.25f},
        GLUT_BITMAP_HELVETICA_12
    );

    drawText(
        HUD_LEFT + 20, 370,
        "MISSION STATE",
        {0.65f, 0.70f, 0.72f},
        GLUT_BITMAP_HELVETICA_12
    );

    drawText(
        HUD_LEFT + 145, 370,
        stateText(),
        {0.30f, 0.80f, 0.90f},
        GLUT_BITMAP_HELVETICA_12
    );

    drawText(
        HUD_LEFT + 20, 335,
        "ROBOT BATTERY",
        {0.65f, 0.70f, 0.72f},
        GLUT_BITMAP_HELVETICA_12
    );

    char batteryText[32];
    snprintf(batteryText, sizeof(batteryText), "%.0f%%", battery);

    drawText(
        HUD_LEFT + 145, 335,
        batteryText,
        battery > 25
            ? Color{0.20f, 0.90f, 0.45f}
            : Color{1.0f, 0.30f, 0.10f},
        GLUT_BITMAP_HELVETICA_12
    );

    drawRect(
        HUD_LEFT + 20, 310, 240, 10,
        {0.10f, 0.14f, 0.15f}
    );

    drawRect(
        HUD_LEFT + 20, 310,
        240.0f * (battery / 100.0f),
        10,
        battery > 25
            ? Color{0.20f, 0.80f, 0.40f}
            : Color{0.90f, 0.20f, 0.08f}
    );

    drawText(
        HUD_LEFT + 20, 275,
        "FIRE ZONES",
        {0.65f, 0.70f, 0.72f},
        GLUT_BITMAP_HELVETICA_12
    );

    drawText(
        HUD_LEFT + 145, 275,
        to_string((int)fires.size()) + " ACTIVE",
        {1.0f, 0.30f, 0.10f},
        GLUT_BITMAP_HELVETICA_12
    );

    drawText(
        HUD_LEFT + 20, 235,
        "STATUS",
        {0.45f, 0.50f, 0.53f},
        GLUT_BITMAP_HELVETICA_10
    );

    drawRect(
        HUD_LEFT + 20, 198, 240, 25,
        {0.08f, 0.12f, 0.13f}
    );

    drawText(
        HUD_LEFT + 30, 206,
        missionMessage,
        missionState == MISSION_WAITING
            ? Color{1.0f, 0.70f, 0.05f}
            : Color{0.35f, 0.85f, 0.88f},
        GLUT_BITMAP_HELVETICA_10
    );

    if (missionState == MISSION_WAITING) {
        drawText(
            HUD_LEFT + 20, 165,
            waitingReason,
            {1.0f, 0.45f, 0.20f},
            GLUT_BITMAP_HELVETICA_10
        );
    }

    drawText(
        HUD_LEFT + 20, 130,
        "ENTER / SPACE  START",
        {0.75f, 0.78f, 0.80f},
        GLUT_BITMAP_HELVETICA_10
    );

    drawText(
        HUD_LEFT + 20, 108,
        "P  PAUSE    R  RESTART",
        {0.55f, 0.60f, 0.63f},
        GLUT_BITMAP_HELVETICA_10
    );

    drawText(
        HUD_LEFT + 20, 86,
        "WASD / ARROWS  MANUAL OVERRIDE",
        {0.55f, 0.60f, 0.63f},
        GLUT_BITMAP_HELVETICA_10
    );

    drawRect(
        HUD_LEFT, 20,
        HUD_RIGHT - HUD_LEFT, 48,
        {0.030f, 0.040f, 0.045f}
    );

    drawText(
        HUD_LEFT + 20, 50,
        "A*  |  COLLISION AVOIDANCE  |  RESCUE",
        {0.25f, 0.65f, 0.68f},
        GLUT_BITMAP_HELVETICA_10
    );
}

// DISPLAY

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    if (appScreen == SCREEN_MENU) {
        drawMenu();
    } else if (appScreen == SCREEN_CONTROLS) {
        drawInfoPage(true);
    } else if (appScreen == SCREEN_HELP) {
        drawInfoPage(false);
    } else {
        drawRect(0, 0, UI_W, UI_H,
                 {0.025f, 0.035f, 0.040f});

        drawHeader();
        drawMapBackground();
        drawFireMarkers();
        drawObstacles();
        drawRescueStation();
        drawPath();

        for (int i = 0; i < (int)survivors.size(); ++i) {
            if (!survivors[i].rescued &&
                !survivors[i].carried) {
                drawSurvivor(survivors[i].x,
                             survivors[i].y,
                             i + 1);
            }
        }

        drawFires();
        drawRobot();
        drawCarriedSurvivor();

        drawText(40, 605,
                 "SECTOR A // ACTIVE EMERGENCY ZONE",
                 {0.30f, 0.65f, 0.68f},
                 GLUT_BITMAP_HELVETICA_10);

        drawHUD();
        drawControlHint();
        drawMissionOverlay();
    }

    glutSwapBuffers();
}


// INPUT

void failByFire(){ missionState=MISSION_FAILED_FIRE; currentPath.clear(); pathIndex=0; carryingSurvivor=false; missionMessage="THE ROBOT IS BURNT IN FIRE"; waitingReason="YOU FAILED TO RESCUE"; showEndPopup(); }
void failByBattery(){ missionState=MISSION_FAILED_BATTERY; currentPath.clear(); pathIndex=0; missionMessage="BATTERY DEPLETED"; waitingReason="YOU FAILED TO RESCUE"; showEndPopup(); }

void drawControlHint() {
    if (missionState == MISSION_READY ||
        missionState == MISSION_COMPLETE ||
        missionState == MISSION_FAILED_FIRE ||
        missionState == MISSION_FAILED_BATTERY)
        return;

    const char* modeText = manualControlMode
        ? "MANUAL: W/A/S/D OR ARROWS  |  M: RETURN TO A*"
        : "AUTO A*: ROBOT NAVIGATES  |  M: TAKE MANUAL CONTROL";

    drawCenteredText(
        545, 630,
        modeText,
        manualControlMode
            ? Color{1.0f, 0.75f, 0.20f}
            : Color{0.40f, 0.85f, 0.90f},
        GLUT_BITMAP_HELVETICA_10
    );
}

void drawMissionOverlay(){
    bool failed = missionState == MISSION_FAILED_FIRE ||
                  missionState == MISSION_FAILED_BATTERY;
    bool complete = missionState == MISSION_COMPLETE;
    if (!failed && !complete) return;

    drawRect(0, 0, UI_W, UI_H, {0.005f, 0.008f, 0.010f});

    float x = 280, y = 180, w = 700, h = 340;
    drawRoundedPanel(x, y, w, h,
                     {0.045f, 0.065f, 0.075f},
                     complete ? Color{0.20f, 0.85f, 0.48f}
                              : Color{0.95f, 0.22f, 0.12f});

    if (complete) {
        drawCenteredBoldText(630, 430, "MISSION COMPLETE",
                             {0.25f, 0.95f, 0.50f},
                             GLUT_BITMAP_HELVETICA_18);
        drawCenteredBoldText(630, 375, "CONGRATULATIONS!",
                             {0.95f, 0.90f, 0.25f},
                             GLUT_BITMAP_HELVETICA_18);
        drawCenteredText(630, 325, "YOU SAVED ALL THE SURVIVORS.",
                           {0.82f, 0.92f, 0.85f},
                           GLUT_BITMAP_HELVETICA_18);
    } else {
        drawCenteredBoldText(630, 430, "MISSION FAILED",
                             {0.98f, 0.25f, 0.15f},
                             GLUT_BITMAP_HELVETICA_18);
        drawCenteredBoldText(
            630, 375,
            missionState == MISSION_FAILED_FIRE
                ? "THE ROBOT IS BURNT IN FIRE."
                : "ROBOT BATTERY IS DEAD.",
            {1.0f, 0.45f, 0.20f},
            GLUT_BITMAP_HELVETICA_18
        );
        drawCenteredText(630, 325,
                         "YOU FAILED TO RESCUE THE SURVIVORS.",
                         {0.90f, 0.90f, 0.90f},
                         GLUT_BITMAP_HELVETICA_12);
    }

    bool hm = pointInRect(mouseX, mouseY, 465, 220, 330, 55);
    drawButton(465, 220, 330, 55, "RETURN TO MAIN MENU", hm,
               complete ? Color{0.20f, 0.85f, 0.48f}
                        : Color{0.95f, 0.30f, 0.15f});

    drawCenteredText(630, 195,
                     "Returning to menu automatically...",
                     {0.45f, 0.55f, 0.57f},
                     GLUT_BITMAP_HELVETICA_10);
}

void drawStartOverlay() {}


void restartMission() {
    manualControlMode = false;
    robotX = 85.0f;
    robotY = 350.0f;
    battery = 100.0f;
    currentTarget = -1;
    totalRescued = 0;
    carryingSurvivor = false;
    currentPath.clear();
    pathIndex = 0;
    missionState = MISSION_READY;
    missionMessage = "PRESS ENTER / SPACE TO START";
    waitingReason.clear();

    for (auto& s : survivors) {
        s.rescued = false;
        s.carried = false;
    }
}

void keyboard(unsigned char key, int, int) {
    if (key == 27) {
        if (appScreen == SCREEN_MENU) {
            exit(0);
        } else {
            appScreen = SCREEN_MENU;
            restartMission();
            glutPostRedisplay();
            return;
        }
    }

    // Fullscreen toggle.
    if (key == 0x7A) { // F11  special key
        return;
    }

    if (appScreen == SCREEN_MENU) {
        if (key == 13 || key == ' ') {
            startGameFromMenu();
        } else if (key == 'c' || key == 'C') {
            appScreen = SCREEN_CONTROLS;
        } else if (key == 'h' || key == 'H') {
            appScreen = SCREEN_HELP;
        }
        glutPostRedisplay();
        return;
    }

    if (appScreen == SCREEN_CONTROLS || appScreen == SCREEN_HELP) {
        if (key == 13 || key == ' ') {
            appScreen = SCREEN_MENU;
        }
        glutPostRedisplay();
        return;
    }

    // GAME SCREEN.
    if (missionState == MISSION_COMPLETE ||
        missionState == MISSION_FAILED_FIRE ||
        missionState == MISSION_FAILED_BATTERY) {
        if (key == 13 || key == ' ' ||
            key == 'm' || key == 'M') {
            returnToMenu();
        }
        glutPostRedisplay();
        return;
    }

    if (key == 'm' || key == 'M') {
        toggleControlMode();
        return;
    }

    switch (key) {
        case 13:
        case ' ':
            break;

        case 'p':
        case 'P':
            if (missionState == MISSION_TO_SURVIVOR ||
                missionState == MISSION_RETURNING) {
                missionState = MISSION_PAUSED;
                missionMessage = "MISSION PAUSED";
            } else if (missionState == MISSION_PAUSED &&
                       !manualControlMode) {
                if (carryingSurvivor) {
                    if (!createPathTo(BASE_X, BASE_Y))
                        setWaiting("NO SAFE RETURN ROUTE");
                    else {
                        missionState = MISSION_RETURNING;
                        missionMessage = "RETURNING TO BASE";
                    }
                } else if (currentTarget >= 0) {
                    if (!createPathTo(survivors[currentTarget].x,
                                      survivors[currentTarget].y)) {
                        setWaiting("NO SAFE PATH");
                    } else {
                        missionState = MISSION_TO_SURVIVOR;
                        missionMessage = "RESUMING A* NAVIGATION";
                    }
                }
            }
            break;

        case 'r':
        case 'R':
            restartMission();
            startGameFromMenu();
            break;

        case 'w':
        case 'W':
            manualMove(0, 10);
            break;

        case 's':
        case 'S':
            manualMove(0, -10);
            break;

        case 'a':
        case 'A':
            manualMove(-10, 0);
            break;

        case 'd':
        case 'D':
            manualMove(10, 0);
            break;
    }

    glutPostRedisplay();
}


void specialKeys(int key, int, int) {
    if (key == GLUT_KEY_F11) {
        if (!fullscreenMode) {
            windowedWidth = glutGet(GLUT_WINDOW_WIDTH);
            windowedHeight = glutGet(GLUT_WINDOW_HEIGHT);
            windowedX = glutGet(GLUT_WINDOW_X);
            windowedY = glutGet(GLUT_WINDOW_Y);

            fullscreenMode = true;
            glutFullScreen();
        } else {
            fullscreenMode = false;
            glutReshapeWindow(windowedWidth, windowedHeight);
            glutPositionWindow(windowedX, windowedY);
        }
        return;
    }

    if (appScreen != SCREEN_GAME)
        return;

    if (missionState == MISSION_COMPLETE ||
        missionState == MISSION_FAILED_FIRE ||
        missionState == MISSION_FAILED_BATTERY)
        return;

    switch (key) {
        case GLUT_KEY_UP:
            manualMove(0, 10);
            break;
        case GLUT_KEY_DOWN:
            manualMove(0, -10);
            break;
        case GLUT_KEY_LEFT:
            manualMove(-10, 0);
            break;
        case GLUT_KEY_RIGHT:
            manualMove(10, 0);
            break;
    }

    glutPostRedisplay();
}


// FIRE ANIMATION

void updateFire(int) {
    currentFireFrame++;

    if (currentFireFrame >= 5)
        currentFireFrame = 0;

    if (appScreen == SCREEN_GAME &&
        (missionState == MISSION_COMPLETE ||
         missionState == MISSION_FAILED_FIRE ||
         missionState == MISSION_FAILED_BATTERY) &&
        endPopupStarted > 0) {
        int elapsed = glutGet(GLUT_ELAPSED_TIME) - endPopupStarted;
        if (elapsed >= END_POPUP_DURATION_MS) {
            returnToMenu();
        }
    }

    glutPostRedisplay();
    glutTimerFunc(120, updateFire, 0);
}

// INITIALIZ

void init() {
    glClearColor(
        0.025f, 0.035f, 0.040f, 1.0f
    );

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, UI_W, 0, UI_H);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    fireTextures[0] = loadTexture("assets/fire1.png");
    fireTextures[1] = loadTexture("assets/fire2.png");
    fireTextures[2] = loadTexture("assets/fire3.png");
    fireTextures[3] = loadTexture("assets/fire4.png");
    fireTextures[4] = loadTexture("assets/fire5.png");
}

// WINDOW - MOUSE 

void reshape(int w, int h) {
    if (w <= 0 || h <= 0) return;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();


    float windowAspect = (float)w / (float)h;
    float uiAspect = UI_W / UI_H;

    if (windowAspect >= uiAspect) {
        float visibleW = UI_H * windowAspect;
        float extraW = (visibleW - UI_W) * 0.5f;
        gluOrtho2D(-extraW, UI_W + extraW, 0.0, UI_H);
    } else {
        float visibleH = UI_W / windowAspect;
        float extraH = (visibleH - UI_H) * 0.5f;
        gluOrtho2D(0.0, UI_W, -extraH, UI_H + extraH);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void updateMousePosition(int x, int y) {
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    if (w <= 0 || h <= 0) return;

    float windowAspect = (float)w / (float)h;
    float uiAspect = UI_W / UI_H;

    if (windowAspect >= uiAspect) {
        float visibleW = UI_H * windowAspect;
        float extraW = (visibleW - UI_W) * 0.5f;
        mouseX = ((float)x / (float)w) * visibleW - extraW;
        mouseY = ((float)(h - y) / (float)h) * UI_H;
    } else {
        float visibleH = UI_W / windowAspect;
        float extraH = (visibleH - UI_H) * 0.5f;
        mouseX = ((float)x / (float)w) * UI_W;
        mouseY = ((float)(h - y) / (float)h) * visibleH - extraH;
    }
}

void passiveMouse(int x, int y) {
    updateMousePosition(x, y);
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN)
        return;

    updateMousePosition(x, y);

    // MAIN MENU 
    if (appScreen == SCREEN_MENU) {
        if (pointInRect(mouseX, mouseY, 80, 205, 510, 62)) {
            startGameFromMenu();
        }
        else if (pointInRect(mouseX, mouseY, 80, 130, 245, 55)) {
            appScreen = SCREEN_CONTROLS;
        }
        else if (pointInRect(mouseX, mouseY, 345, 130, 245, 55)) {
            appScreen = SCREEN_HELP;
        }
    }
    /// CONTROLS / HELP
    else if (appScreen == SCREEN_CONTROLS || appScreen == SCREEN_HELP) {
        if (pointInRect(mouseX, mouseY, 80, 45, 180, 52)) {
            appScreen = SCREEN_MENU;
        }
    }
    //GAme
    else if (appScreen == SCREEN_GAME) {
        if (missionState == MISSION_COMPLETE ||
            missionState == MISSION_FAILED_FIRE ||
            missionState == MISSION_FAILED_BATTERY) {
            if (pointInRect(mouseX, mouseY, 465, 220, 330, 55)) {
                returnToMenu();
            }
        }
    }

    glutPostRedisplay();
}

// MAIN

int main(int argc, char** argv) {
    glutInit(&argc, argv);

    glutInitDisplayMode(
        GLUT_DOUBLE | GLUT_RGBA
    );

    glutInitWindowSize(
        WINDOW_WIDTH,
        WINDOW_HEIGHT
    );

    glutInitWindowPosition(50, 30);

    glutCreateWindow(
        "RescueX - Intelligent Emergency Response Simulation"
    );

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutPassiveMotionFunc(passiveMouse);

    glutTimerFunc(16, updateRobot, 0);
    glutTimerFunc(120, updateFire, 0);

    glutMainLoop();

    return 0;
}

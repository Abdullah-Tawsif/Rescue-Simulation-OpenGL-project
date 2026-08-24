#include <GL/freeglut.h>
#include <cmath>

// =========================
// Camera
// =========================

float cameraX = 0.0f;
float cameraY = 5.0f;
float cameraZ = 10.0f;

float cameraYaw = 0.0f;

// =========================
// Initialization
// =========================

void init()
{
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat lightPosition[] = {
        5.0f, 10.0f, 5.0f, 1.0f
    };

    glLightfv(
        GL_LIGHT0,
        GL_POSITION,
        lightPosition
    );

    GLfloat ambient[] = {
        0.3f, 0.3f, 0.3f, 1.0f
    };

    GLfloat diffuse[] = {
        0.8f, 0.8f, 0.8f, 1.0f
    };

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
}

// =========================
// Ground
// =========================

void drawGround()
{
    glColor3f(0.25f, 0.25f, 0.25f);

    glBegin(GL_QUADS);

    glVertex3f(-20.0f, 0.0f, -20.0f);
    glVertex3f( 20.0f, 0.0f, -20.0f);
    glVertex3f( 20.0f, 0.0f,  20.0f);
    glVertex3f(-20.0f, 0.0f,  20.0f);

    glEnd();
}

// =========================
// Cube
// =========================

void drawCube()
{
    glColor3f(0.2f, 0.5f, 0.8f);

    glPushMatrix();

    glTranslatef(0.0f, 1.0f, 0.0f);

    glutSolidCube(2.0);

    glPopMatrix();
}

// =========================
// Display
// =========================

void display()
{
    glClear(
        GL_COLOR_BUFFER_BIT |
        GL_DEPTH_BUFFER_BIT
    );

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Calculate camera direction
    float directionX = sin(cameraYaw);
    float directionZ = -cos(cameraYaw);

    gluLookAt(
        cameraX,
        cameraY,
        cameraZ,

        cameraX + directionX,
        cameraY,
        cameraZ + directionZ,

        0.0f,
        1.0f,
        0.0f
    );

    drawGround();
    drawCube();

    glutSwapBuffers();
}

// =========================
// Window resize
// =========================

void reshape(int width, int height)
{
    if (height == 0)
        height = 1;

    float aspect =
        (float)width / (float)height;

    glViewport(
        0,
        0,
        width,
        height
    );

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(
        60.0,
        aspect,
        0.1,
        100.0
    );

    glMatrixMode(GL_MODELVIEW);
}

// =========================
// Keyboard
// =========================

void keyboard(
    unsigned char key,
    int x,
    int y
)
{
    const float speed = 0.5f;

    float directionX = sin(cameraYaw);
    float directionZ = -cos(cameraYaw);

    switch (key)
    {
        // Forward
        case 'w':
        case 'W':

            cameraX += directionX * speed;
            cameraZ += directionZ * speed;

            break;

        // Backward
        case 's':
        case 'S':

            cameraX -= directionX * speed;
            cameraZ -= directionZ * speed;

            break;

        // Left
        case 'a':
        case 'A':

            cameraX -= directionZ * speed;
            cameraZ += directionX * speed;

            break;

        // Right
        case 'd':
        case 'D':

            cameraX += directionZ * speed;
            cameraZ -= directionX * speed;

            break;

        // Exit
        case 27:

            exit(0);

            break;
    }

    glutPostRedisplay();
}

// =========================
// Main
// =========================

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
        GLUT_RGB |
        GLUT_DEPTH
    );

    glutInitWindowSize(
        1000,
        700
    );

    glutCreateWindow(
        "RescueX 3D"
    );

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);

    glutMainLoop();

    return 0;
}
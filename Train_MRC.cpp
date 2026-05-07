#include <GL/glut.h>
#include <cstdlib>  // for rand()
#include <ctime>    // for time()
#include <math.h>

//global variable for sun position
float sunAngle = 0.0f;
float moonAngle = 3.14159f; //since moon is opposite to the sun, we start from the opposite direction, half a revolution
float red = 0.0f, green = 0.0f, blue = 0.0f; // background color

float offset = 0;           // Will start at 0, used to shift mountains to the left
float offsetBack = 0;       // This will be used for the mountains behind
float offsetFar = 0;        // Farthest back layer
float heightsFront[30];     // Stores fixed mountain heights so they don't change every frame
float heightsMid[30];       // Mid layer heights
float heightsFar[30];       // Far layer heights
int winW = 800, winH = 400; // current window size — updated on resize

// draws a filled 2D circle at (cx, cy) with given radius
void drawCircle(float cx, float cy, float radius, int segments) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy); // center point
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * 3.14159f * i / segments;
        glVertex2f(cx + cos(angle) * radius, cy + sin(angle) * radius);
    }
    glEnd();
}

void drawStars()
{
    // Show stars only at night
    if (sin(sunAngle) > 0.0f)
        return;

    glPointSize(3.0f);

    glBegin(GL_POINTS);
    glColor3f(1.0f, 1.0f, 1.0f);
        glVertex2f(50, 300);
        glVertex2f(100, 350);
        glVertex2f(200, 280);
        glVertex2f(300, 320);
        glVertex2f(600, 250);
    glEnd();
}

void drawSky() {
    float s = sin(sunAngle); // -1 = midnight, 0 = horizon, +1 = noon

    // FIX: proper day/night/sunrise/sunset color transitions
    // t goes 0 (night) => 1 (full day) based on sun height
    float t = (s + 1.0f) / 2.0f; // 0.0 at midnight, 1.0 at noon

    // horizon glow during sunrise/sunset, peaks when sun is near horizon (s ≈ 0)
    float horizonGlow = 1.0f - fabsf(s); // 1.0 at horizon, 0.0 at noon/midnight
    horizonGlow = horizonGlow * horizonGlow; // sharpen the glow curve

    // night sky → deep blue, day sky → bright blue, horizon → orange/pink glow
    red = 0.05f * (1.0f - t) + 0.53f * t + 0.6f * horizonGlow * t;
    green = 0.05f * (1.0f - t) + 0.75f * t + 0.2f * horizonGlow * t;
    blue = 0.20f * (1.0f - t) + 0.95f * t - 0.3f * horizonGlow * t;

    // clamp so colors stay valid
    if (red > 1.0f) red = 1.0f;
    if (green > 1.0f) green = 1.0f;
    if (blue > 1.0f) blue = 1.0f;
    if (blue < 0.0f) blue = 0.0f;

    glClearColor(red, green, blue, 1.0f); // Set the background color
}

void drawSun() {
    glPushMatrix();// saves current transformation

    // FIX: make it work with projection 
    float worldWidth = 400.0f * ((float)winW / winH);

    float x = cos(sunAngle) * (worldWidth / 2.0f - 50) + worldWidth / 2.0f; // the x changes to adapt with the new windows changes
    float y = sin(sunAngle) * 150 + 200;

    //  FIX: draw a 2D filled circle instead of glutSolidSphere
    // glutSolidSphere is 3D and renders hollow/broken in 2D ortho projection
    glColor3f(1.0f, 0.95f, 0.3f);  // warm yellow sun
    drawCircle(x, y, 20, 48);

    glPopMatrix();
}

void drawMoon() {
    glPushMatrix();

    // FIX: same correction as sun
    float worldWidth = 400.0f * ((float)winW / winH);

    float x = cos(moonAngle) * (worldWidth / 2.0f - 50) + worldWidth / 2.0f;
    float y = sin(moonAngle) * 150 + 200;

    // FIX: draw a 2D filled circle instead of glutSolidSphere
    // same reason as sun — glutSolidSphere breaks in 2D ortho
    glColor3f(0.95f, 0.95f, 0.85f); // slightly warm white moon
    drawCircle(x, y, 15, 48);

    glPopMatrix();//restores the old transformation state
}

// FRONT mountain — wide base, sharp peak, no shoulder bumps
void drawMountain(float x, float h) {
    glBegin(GL_TRIANGLES);
    // main triangle — left base pulled back by 40 to overlap with previous mountain
    glVertex2f(x - 40, 0);
    glVertex2f(x + 240, 0);
    glVertex2f(x + 120, h);
    // right bump
    glVertex2f(x + 120, h * 0.55f);
    glVertex2f(x + 240, 0);
    glVertex2f(x + 180, h * 0.65f);
    glEnd();
}

// MID mountain
void drawBackMountain(float x, float h) {
    glBegin(GL_TRIANGLES);
    // main triangle — left base pulled back by 30
    glVertex2f(x - 30, 0);
    glVertex2f(x + 200, 0);
    glVertex2f(x + 100, h);
    // right shoulder
    glVertex2f(x + 100, h * 0.5f);
    glVertex2f(x + 200, 0);
    glVertex2f(x + 150, h * 0.6f);
    glEnd();
}

// FAR mountain — simple clean triangle
void drawFarMountain(float x, float h) {
    glBegin(GL_TRIANGLES);
    // left base pulled back by 40 to blend with neighbour
    glVertex2f(x - 40, 0);
    glVertex2f(x + 280, 0);
    glVertex2f(x + 140, h);
    glEnd();
}

void display() {
    drawSky();

    glClear(GL_COLOR_BUFFER_BIT); //Wipe out the entire screen before drawing every single time

    // FIX: reset modelview every frame
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();// resets the transformation matrix

    drawStars();
    drawSun();
    drawMoon();

    // FAR BACK (darkest, tallest, draw first)
    glColor3f(0.25, 0.25, 0.35);
    for (int i = 0; i < 30; i++) {
        GLfloat x = i * 280 - offsetFar;
        drawFarMountain(x, heightsFar[i]);
    }

    // BACKGROUND (draw second)
    glColor3f(0.38, 0.40, 0.52);
    for (int i = 0; i < 30; i++) {
        GLfloat x = i * 200 - offsetBack;
        drawBackMountain(x, heightsMid[i]);
    }

    // FRONT (draw last, brightest)
    glColor3f(0.20, 0.52, 0.28);
    for (int i = 0; i < 30; i++) {
        GLfloat x = i * 240 - offset;
        drawMountain(x, heightsFront[i]);
    }

    glutSwapBuffers();// swaps hidden buffer with visible buffer
}

void update() {
    offset += 0.5f;    // front moves faster
    offsetBack += 0.18f;   // back moves slower
    offsetFar += 0.06f;   // far back moves slowest

    // reset early 
    if (offset >= 240 * 26) offset -= 240 * 26;
    if (offsetBack >= 200 * 26) offsetBack -= 200 * 26;
    if (offsetFar >= 280 * 26) offsetFar -= 280 * 26;

    //  merged day/night cycle here
    sunAngle += 0.001f;
    moonAngle += 0.001f;
    //Making sure the val is always between 0 -> 2PI
    if (sunAngle > 2 * 3.14159f)
        sunAngle = 0.0f;
    if (moonAngle > 2 * 3.14159f)
        moonAngle = 0.0f;

    glutPostRedisplay();// redraws the screen
}

// Called automatically whenever the window is resized
void reshape(int w, int h) {
    winW = w;
    winH = h;
    glViewport(0, 0, w, h);          //fill the whole window

    glMatrixMode(GL_PROJECTION); //We tell opengl to apply all matrix operations on the projection matrix
    glLoadIdentity(); //Wipe out the old projection matrix

    float aspect = (float)w / (float)h;
    gluOrtho2D(0, 400 * aspect, 0, 400); //creates the new matrix

    glMatrixMode(GL_MODELVIEW); //  important to switch back
}

void init() {
    glClearColor(0.15f, 0.18f, 0.28f, 1);  // dark night sky
    srand(time(0));

    for (int i = 0; i < 30; i++) {
        heightsFar[i] = 210 + rand() % 80;
        heightsMid[i] = 180 + rand() % 60;
        heightsFront[i] = 90 + rand() % 50;
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 400);
    glutCreateWindow("Mountains + Day/Night");

    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(update);
    glutMainLoop();
    return 0;
}
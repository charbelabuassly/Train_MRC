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
float baseHeight = 100;       // The starting height of the mountains
float heightsFront[30];     // Stores fixed mountain heights so they don't change every frame
float heightsMid[30];       // Mid layer heights
float heightsFar[30];       // Far layer heights
int winW = 800, winH = 400; // current window size — updated on resize

float trainBaseY = baseHeight - 60; //Used to make sure all train cabins are on the same base height

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
    glColor3f(0.9f, 0.9f, 1.0f);
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

    // night sky => deep blue, day sky => bright blue, horizon => orange/pink glow
    red = 0.08f * (1.0f - t) + 0.55f * t + 0.7f * horizonGlow * t;
    green = 0.06f * (1.0f - t) + 0.70f * t + 0.25f * horizonGlow * t;
    blue = 0.18f * (1.0f - t) + 0.98f * t - 0.25f * horizonGlow * t;

    // clamp so colors stay valid
    if (red > 1.0f) red = 1.0f;
    if (green > 1.0f) green = 1.0f;
    if (blue > 1.0f) blue = 1.0f;
    if (blue < 0.0f) blue = 0.0f;

    glClearColor(red, green, blue, 1.0f); // Set the background color
}

void drawSun() {
    glPushMatrix();// saves current transformation

    float worldWidth = 400.0f * ((float)winW / winH);

    // coordinates of a ellipse around the center of the screen, 
    float x = cos(sunAngle) * (worldWidth / 2.0f + 50) + worldWidth / 2.0f; // cos(x)*radius + centerX 
    float y = sin(sunAngle) * 150 + 200;// sin(x)* radius + centerY
    //centerX and centerY are two focus points of the ellipse, radius is the distance from the center to the edge of the ellipse along the x or y axis.

    glColor3f(1.0f, 0.85f, 0.3f);
    drawCircle(x, y, 20, 48);

    glPopMatrix();
}

void drawMoon() {
    glPushMatrix();

    float worldWidth = 400.0f * ((float)winW / winH);

    float x = cos(moonAngle) * (worldWidth / 2.0f + 50) + worldWidth / 2.0f;
    float y = sin(moonAngle) * 150 + 200;

    glColor3f(0.85f, 0.9f, 1.0f);
    drawCircle(x, y, 15, 48);

    glPopMatrix();//restores the old transformation state
}

// FRONT mountain 
void drawMountain(float x, float h) {
    glBegin(GL_TRIANGLES);
    glColor3f(0.18f, 0.45f, 0.25f);
    glVertex2f(x - 40, baseHeight);
    glVertex2f(x + 240, baseHeight);
    glVertex2f(x + 120, h + baseHeight);

    glColor3f(0.12f, 0.35f, 0.20f);
    glVertex2f(x + 120, h * 0.55f + baseHeight);
    glVertex2f(x + 240, baseHeight);
    glVertex2f(x + 180, h * 0.65f + baseHeight);
    glEnd();
}

// MID mountain
void drawBackMountain(float x, float h) {
    glBegin(GL_TRIANGLES);
    glColor3f(0.25f, 0.30f, 0.45f);
    glVertex2f(x - 30, baseHeight);
    glVertex2f(x + 200, baseHeight);
    glVertex2f(x + 100, h + baseHeight);

    glColor3f(0.20f, 0.25f, 0.38f);
    glVertex2f(x + 100, h * 0.5f + baseHeight);
    glVertex2f(x + 200, baseHeight);
    glVertex2f(x + 150, h * 0.6f + baseHeight);
    glEnd();
}

// FAR mountain — simple clean triangle
void drawFarMountain(float x, float h) {
    glBegin(GL_TRIANGLES);
    glColor3f(0.18f, 0.20f, 0.30f);
    glVertex2f(x - 40, baseHeight);
    glVertex2f(x + 280, baseHeight);
    glVertex2f(x + 140, h + baseHeight);
    glEnd();
}

void drawGround() {
    glBegin(GL_POLYGON);
    glColor3f(0.05f, 0.35f, 0.10f);
    glVertex2f(0, 0); // bottom-left
    glVertex2f(winW, 0);// bottom-right
    glColor3f(0.08f, 0.45f, 0.12f);
    glVertex2f(winW, baseHeight); // top-right
    glVertex2f(0, baseHeight);  // top-left
    glEnd();
}

// TRAIN DRAWING FUNCTIONS
void drawEngine() {

}

void drawPassenger(float x, float y) {
    // Hardcoded declared length and width of the passenger cabin
    float width = 200;
    float height = 40;

    int numWindows = 5;
    float windowWidth = 25;
    float windowHeight = 15;
    float spacing = 35;

    // Main body
    glColor3f(0.75f, 0.25f, 0.30f);
    glBegin(GL_POLYGON);
    glVertex2f(x, y);// bottom-left
    glVertex2f(x + width, y);// bottom-right
    glVertex2f(x + width, y + height); // top-right
    glVertex2f(x, y + height);// top-left
    glEnd();

    // Roof 
    glColor3f(0.95f, 0.85f, 0.65f);
    glBegin(GL_POLYGON);
    glVertex2f(x - 5, y + height);// bottom-left
    glVertex2f(x + width + 5, y + height);// bottom-right
    glVertex2f(x + width, y + height + 10);// top-right
    glVertex2f(x, y + height + 10);// top-left
    glEnd();

    for (int i = 0; i < numWindows; i++) {
        if (i == 0) continue;
        float wx = x + i * spacing;
        float wy = y + height - 20;

        // Window
        glColor3f(0.55f, 0.85f, 1.0f);
        glBegin(GL_POLYGON);
        glVertex2f(wx, wy);// bottom-left
        glVertex2f(wx + windowWidth, wy);// bottom-right
        glVertex2f(wx + windowWidth, wy + windowHeight);// top-right
        glVertex2f(wx, wy + windowHeight);// top-left
        glEnd();
    }

    // Bottom stripe
    glColor3f(0.15f, 0.08f, 0.08f);
    glBegin(GL_POLYGON);
    glVertex2f(x, y + 5);           // bottom-left
    glVertex2f(x + width, y + 5);   // bottom-right
    glVertex2f(x + width, y + 10);  // top-right
    glVertex2f(x, y + 10);          // top-left
    glEnd();
}

void drawCargo(float x, float y) {
    float width = 100;
    float height = 40;

    // Main body
    glColor3f(0.45f, 0.25f, 0.15f);
    glBegin(GL_POLYGON);
    glVertex2f(x, y); // bottom-left
    glVertex2f(x + width, y);  // bottom-right
    glVertex2f(x + width, y + height); // top-right
    glVertex2f(x, y + height);  // top-left
    glEnd();

    // Roof
    glColor3f(0.05f, 0.05f, 0.05f);
    glBegin(GL_POLYGON);
    glVertex2f(x + 5, y + height); // bottom-left
    glVertex2f(x + width - 5, y + height); // bottom-right
    glVertex2f(x + width - 10, y + height + 12); // top-right
    glVertex2f(x + 10, y + height + 12); // top-left
    glEnd();

    // Bottom stripe
    glColor3f(0.12f, 0.08f, 0.08f);
    glBegin(GL_POLYGON);
    glVertex2f(x, y + 5); // bottom-left
    glVertex2f(x + width, y + 5);// bottom-right
    glVertex2f(x + width, y + 10); // top-right
    glVertex2f(x, y + 10); // top-left
    glEnd();
}

void drawConnector(float startX, float gap, float y) {
    float height = 6;

    glColor3f(0.12f, 0.12f, 0.12f);
    glBegin(GL_POLYGON);
    glVertex2f(startX, y + 10); // bottom-left
    glVertex2f(startX + gap, y + 10); // bottom-right
    glVertex2f(startX + gap, y + 10 + height); // top-right
    glVertex2f(startX, y + 10 + height);  // top-left
    glEnd();
}

void drawTrain(float worldWidth) {
    float startX = int(worldWidth / 2 - worldWidth / 6);
    float y = trainBaseY;
    float gap = 10;

    float passengerWidth = 200;
    float cargoWidth = 100;

    drawPassenger(startX, y);

    drawConnector(startX - gap, gap, y);

    drawCargo(startX - gap - cargoWidth, y);

    drawConnector(startX - gap - cargoWidth - gap, gap, y);

    drawCargo(startX - gap - cargoWidth - gap - cargoWidth, y);
}

void display() {
    float worldWidth = 400.0f * ((float)winW / winH);
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

    drawGround();

    drawTrain(worldWidth);

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
    sunAngle += 0.0001f;
    moonAngle += 0.0001f;
    //Making sure the val is always between 0 -> 2PI
    if (sunAngle > 2 * 3.14159f) sunAngle = 0.0f;
    if (moonAngle > 2 * 3.14159f) moonAngle = 0.0f;

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

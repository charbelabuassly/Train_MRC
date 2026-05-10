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
float groundColor[12];
int patchCount = 12;         // Number of patches of green on the ground, change array size when changing this
int patchLength = 1110;     // Length of each patch of green
int winW = 800, winH = 400; // current window size — updated on resize

float trainBaseY = baseHeight - 60; //Used to make sure all train cabins are on the same base height

// draws a filled 2D circle at (cx, cy) with given radius
void drawCircle(float cx, float cy, float radius, int segments) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy); // center point
    for (int i = 0; i <= segments; i++) {
		float angle = 2.0f * 3.14159f * i / segments; // angle goes from 0 to 2PI as i goes from 0 to segments
		glVertex2f(cx + cos(angle) * radius, cy + sin(angle) * radius); // point on the circle edge at the current angle
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
    float s = sin(sunAngle); // -1 = midnight, 0 = horizon, +1 = noon [-1, 1]

    // t goes 0 (night) => 1 (full day) based on sun height
    float t = (s + 1.0f) / 2.0f; // 0.0 at midnight, 1.0 at noon [0, 1]

    // horizon glow during sunrise/sunset, peaks when sun is near horizon (s ≈ 0)
    float horizonGlow = 1.0f - fabsf(s); // 1.0 at horizon, 0.0 at noon/midnight
    horizonGlow = horizonGlow * horizonGlow; // sharpen the glow curve

    // night sky => deep blue, day sky => bright blue, horizon => orange/pink glow
    red = 0.08f * (1.0f - t) + 0.55f * t + 0.7f * horizonGlow * t;
    green = 0.06f * (1.0f - t) + 0.70f * t + 0.25f * horizonGlow * t;
    blue = 0.18f * (1.0f - t) + 0.98f * t - 0.25f * horizonGlow * t;
    // clamp so colors stay valid
    //if (red > 1.0f) red = 1.0f;
    //if (green > 1.0f) green = 1.0f;
    //if (blue > 1.0f) blue = 1.0f;
    //if (blue < 0.0f) blue = 0.0f;

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
    // Take the offset reset length, divide it into different patches with linked colors and increase length a little, and reset the same at offset

    int groundSpeed = 2;

    for (int i = 1; i < patchCount + 1; i++) {
        glBegin(GL_POLYGON);
        glColor3f(0, groundColor[i - 1], 0.1);          // color of the previous patch of grass
        glVertex2f((i - 1) * patchLength - offset * groundSpeed, 0);
        glColor3f(0, groundColor[i % patchCount], 0.1);              // color of this patch of grass
        glVertex2f((i - 1) * patchLength + patchLength - offset * groundSpeed, 0);
        glVertex2f((i - 1) * patchLength + patchLength - offset * groundSpeed, baseHeight);
        glColor3f(0, groundColor[i - 1], 0.1);          // color of the previous patch of grass
        glVertex2f((i - 1) * patchLength - offset * groundSpeed, baseHeight);
        glEnd();
    }

}

void drawWheel(float cx, float cy, float radius) {
    glColor3f(0.1f, 0.1f, 0.1f);
    drawCircle(cx, cy, radius, 32);
    glLineWidth(3.0f);
    //the wheels added details to make it appear that it is turning by adding spokes to the wheel which are lines, and the spokes rotate based on the offset to create the illusion of movement
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_LINES);
    for (int i = 0; i < 8; i++) {
        float angle = -offset * 0.001f + i * 3.14159f / 4; // rotates the spokes based on the offset the equation is : angle = offset * 0.1f + i * (PI / 4) where offset * 0.1f controls the rotation speed and i * (PI / 4) spaces the spokes evenly around the wheel
        glVertex2f(cx, cy); // center of the wheel
        glVertex2f(cx + cos(angle) * radius, cy + sin(angle) * radius); // end of the 
    }
    glEnd();
}

void drawEngine(float x, float y) {
    float width = 120;
    float height = 60; // increased from 40 → taller engine

    // BASE RECTANGLE
    // Full width (120), height = 20 
    glColor3f(0.25f, 0.25f, 0.28f);
    glBegin(GL_POLYGON);
    glVertex2f(x, y); // bottom-left
    glVertex2f(x + width, y); // bottom-right
    glVertex2f(x + width, y + height / 2); // top-right
    glVertex2f(x, y + height / 2);// top-left
    glEnd();

    // BASE BOTTOM STRIPE
    // Thin red accent stripe along the bottom 
    glColor3f(0.75f, 0.10f, 0.10f);
    glBegin(GL_POLYGON);
    glVertex2f(x, y);// bottom-left
    glVertex2f(x + width, y);// bottom-right
    glVertex2f(x + width, y + 4);// top-right
    glVertex2f(x, y + 4);// top-left
    glEnd();

    // top-left rectangle
    // 40% of width height = 20, sits on top of base rect
    float cabinWidth = width * 0.40f;
    float cabinHeight = 30; // increased from 20
    glColor3f(0.30f, 0.30f, 0.35f);
    glBegin(GL_POLYGON);
    glVertex2f(x, y + height / 2);// bottom-left
    glVertex2f(x + cabinWidth, y + height / 2);  // bottom-right
    glVertex2f(x + cabinWidth, y + height / 2 + cabinHeight); // top-right
    glVertex2f(x, y + height / 2 + cabinHeight);// top-left
    glEnd();

    // CABIN WINDOW 
	// change the windows color from blue when there is sun slowly to yellow when there is moon 
    float t = 0.5f + 0.5f * sin(moonAngle);

    float r = 0.6f + t * (1.0f - 0.6f);
    float g = 0.8f + t * (1.0f - 0.8f);
    float b = 1.0f + t * (0.2f - 1.0f);

    glColor3f(r, g, b);
    glBegin(GL_POLYGON);
    glVertex2f(x + 8, y + height / 2 + 8);  // bottom-left (slightly raised)
    glVertex2f(x + 30, y + height / 2 + 8);  // bottom-right
    glVertex2f(x + 30, y + height / 2 + 22); // top-right
    glVertex2f(x + 8, y + height / 2 + 22); // top-left
    glEnd();

    // CABIN TOP STRIPE
    glColor3f(0.75f, 0.10f, 0.10f);
    glBegin(GL_POLYGON);
    glVertex2f(x, y + height / 2 + cabinHeight - 4);// bottom-left
    glVertex2f(x + cabinWidth, y + height / 2 + cabinHeight - 4); // bottom-right
    glVertex2f(x + cabinWidth, y + height / 2 + cabinHeight);// top-right
    glVertex2f(x, y + height / 2 + cabinHeight);// top-left
    glEnd();

    // PENTAGON 
    // Width 30, height 18, flat bottom with angled top peak
    float pentWidth = 30;
    float pentHeight = 24; // slightly taller
    glColor3f(0.35f, 0.22f, 0.10f);
    glBegin(GL_POLYGON);
    glVertex2f(x + cabinWidth, y + height / 2);  // bottom-left
    glVertex2f(x + cabinWidth + pentWidth, y + height / 2); // bottom-right
    glVertex2f(x + cabinWidth + pentWidth, y + height / 2 + pentHeight - 6); // upper-right
    glVertex2f(x + cabinWidth + pentWidth / 2, y + height / 2 + pentHeight); // top-center peak
    glVertex2f(x + cabinWidth, y + height / 2 + pentHeight - 6);  // upper-left
    glEnd();

    // CHIMNEY BASE (laying rectangle)
    glColor3f(0.2f, 0.2f, 0.2f);
    float baseWidth = 20; //width of the laying rectangle
    float baseHeightChim = 5; //height of it
    float baseX = x + cabinWidth + pentWidth + 5; //base of the laying rectange X (Where it starts)
    float baseY = y + height / 2; //base of laying rectangle y 

    glBegin(GL_POLYGON);
    glVertex2f(baseX, baseY); // bottom-left
    glVertex2f(baseX + baseWidth, baseY); // bottom-right
    glVertex2f(baseX + baseWidth, baseY + baseHeightChim); // top-right
    glVertex2f(baseX, baseY + baseHeightChim); // top-left
    glEnd();

    // CHIMNEY 
    // vertical rectangle centered on base
    glColor3f(0.1f, 0.1f, 0.1f);
    float chimWidth = 8;
    float chimHeight = 18;
    float chimX = baseX + baseWidth / 2 - chimWidth / 2;

    glBegin(GL_POLYGON);
    glVertex2f(chimX, baseY + baseHeightChim); // bottom-left
    glVertex2f(chimX + chimWidth, baseY + baseHeightChim); // bottom-right
    glVertex2f(chimX + chimWidth, baseY + baseHeightChim + chimHeight); // top-right
    glVertex2f(chimX, baseY + baseHeightChim + chimHeight); // top-left
    glEnd();

    // LOCOMOTIVE FRONT NOSE 
    // Sits at the far right of the base rect
    float triWidth = 30;
    glColor3f(0.45f, 0.15f, 0.10f);
    glBegin(GL_TRIANGLES);
    glVertex2f(x + width, y + height / 2); // top-left of triangle base
    glVertex2f(x + width, y);            // bottom-left of triangle base
    glVertex2f(x + width + triWidth, y); // point OUTWARD 
    glEnd();
	

    drawWheel(x + 20, y + 2, 12);
    drawWheel(x + 45, y + 2, 12);
    drawWheel(x + 100, y, 10);

    //Window frame
    glColor3f(0.2f, 0.2f, 0.2f);
    //make the frames compatible with the windows size 
    glLineWidth(3.0);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x + 8, y + height / 2 + 8);  // bottom-left (slightly raised)
    glVertex2f(x + 30, y + height / 2 + 8);  // bottom-right
    glVertex2f(x + 30, y + height / 2 + 22); // top-right
    glVertex2f(x + 8, y + height / 2 + 22); // top-left
	glEnd();
}

void drawPassenger(float x, float y) {
    // Hardcoded declared length and width of the passenger cabin
    float width = 200;
    float height = 40;

    int numWindows = 5;
    float windowWidth = 25;
    float windowHeight = 15;
    float spacing = 35;

    drawWheel(x + 40, y, 10);
    drawWheel(x + 150, y, 10);
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

		float t = 0.5f + 0.5f * sin(moonAngle);// 0.5 * sin(theta) change the range from [-1, 1] to [-0.5, 0.5], then add 0.5 to shift to [0, 1] and it changes according to the moon angle

        float r = 0.6f + t * (1.0f - 0.6f);
        float g = 0.8f + t * (1.0f - 0.8f);
        float b = 1.0f + t * (0.2f - 1.0f);
        //C = C1 + t(C2 - C1)
        //    Let's take for example from blue to yellow

        //    (0.6, 0.8, 1.0)

        //    red = 0.6 + t * (1.0 - 0.6)
        //    blue = 1.0 + t * (0.2 - 1.0)
		//    green = 0.8 + t * (1.0 - 0.8)
        //        and t changes over time according to the moon angle and these equation only make the transition from light blue to light yellow

        glColor3f(r, g, b);
        glBegin(GL_POLYGON);
        glVertex2f(wx, wy);// bottom-left
        glVertex2f(wx + windowWidth, wy);// bottom-right
        glVertex2f(wx + windowWidth, wy + windowHeight);// top-right
        glVertex2f(wx, wy + windowHeight);// top-left
        glEnd();

		// Window frame
        glLineWidth(2.0f);
        glColor3f(0.95f, 0.85f, 0.65f);
        glBegin(GL_LINE_LOOP);
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
    drawWheel(x + 30, y, 10);
    drawWheel(x + 70, y, 10);
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

void drawSmoke(float x, float y) {

    glPointSize(3.0f);
    glBegin(GL_POINTS);

	for (int i = 0; i < 100; i++) {// draw 100 smoke puffs

        float rise = rand() % 30;// puts the smoke above the chimney and makes it rise up to 30 units above the chimey

        float randomX = rand() % 20 - 10;// randomX and randomY spreads the smoke points
        float randomY = rand() % 10 - 5;

        // offset only changes the shape, not the position
        float wave = sin((offset + i) * 0.1f) * 4.0f; // makes the spoke shape move left/right smoothly
        // put the offset in sin() since if it is outside the sin() it will just move away
        float puffX = x + randomX + wave;
        float puffY = y + rise + randomY;

        glColor3f(0.8f, 0.8f, 0.8f);
        glVertex2f(puffX, puffY);
    }

    glEnd();
}

void drawTrain(float worldWidth) {
    float startX = int(worldWidth / 2 - worldWidth / 6);
    float y = trainBaseY + 2;
    float gap = 10;

    float passengerWidth = 200;
    float cargoWidth = 100;

    drawEngine(startX + gap + 200, y);

    drawConnector(startX + 200 ,gap, y);

    drawPassenger(startX, y);

    drawConnector(startX - gap, gap, y);

    drawCargo(startX - gap - cargoWidth, y);

    drawConnector(startX - gap - cargoWidth - gap, gap, y);

    drawCargo(startX - gap - cargoWidth - gap - cargoWidth, y);

}

void drawRailWay(float worldWidth, float y, float height) {

    // Ground/base under railway
    glColor3f(0.2f, 0.2f, 0.2f);

    glBegin(GL_POLYGON);
    glVertex2f(0, y - height);
    glVertex2f(worldWidth, y - height);
    glVertex2f(worldWidth, y + height);
    glVertex2f(0, y + height);
    glEnd();

    float width1 = 17;
	float width2 = 20;

    // Wooden sleepers
	for (float x = -offset * 0.3; x < worldWidth; x += width2) {// sleepers are spaced every 20 units, but we offset them by a fraction of the train's offset to create the illusion of movement

        // wood color with slight variation
        glColor3f(0.45f, 0.28f, 0.12f);

        glBegin(GL_POLYGON);
        glVertex2f(x, y - height);
        glVertex2f(x + width1 / 3, y - height);
        glVertex2f(x + width1 / 3, y + 3);
        glVertex2f(x, y + 3);
        glEnd();   
    }

    // Top rail
    glColor3f(0.7f, 0.7f, 0.7f);

    glBegin(GL_POLYGON);
    glVertex2f(0, y + 3);
    glVertex2f(worldWidth, y + 3);
    glVertex2f(worldWidth, y + 7);
    glVertex2f(0, y + 7);
    glEnd();

    // Bottom rail
    glBegin(GL_POLYGON);
    glVertex2f(0, y - height - 4);
    glVertex2f(worldWidth, y - height - 4);
    glVertex2f(worldWidth, y - height);
    glVertex2f(0, y - height);
    glEnd();
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

    drawRailWay(worldWidth, trainBaseY, 7);
    drawTrain(worldWidth);
    //draw the smoke on the chimney engine
 
	float chimneyX = worldWidth / 2 - worldWidth / 6 + 63 + 200 + 40; // x position of the chimney
	float chimneyY = trainBaseY + 60 + 7; // y position of the chimney (base height + engine height + chimney base height)
	drawSmoke(chimneyX, chimneyY);
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

    for (int i = 0; i < patchCount; i++) {
        groundColor[i] = rand() % 10 * 0.01 + 0.3;
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

#include <GL/glut.h>
#include <iostream>
#include <math.h>

//global variable for sun position
float sunAngle = 0.0f;
float moonAngle = 3.14159f; //since moon is opposite to the sun, we start from the opposite direction
float red = 0.0f, green = 0.0f, blue = 0.0f; // background color

void drawStars()
{
    // Show stars only at night
    if (sin(sunAngle) > 0.0f)
        return;

    glPointSize(3.0f);

    glBegin(GL_POINTS);

    glColor3f(1.0f, 1.0f, 1.0f);

    glVertex2f(-0.8f, 0.7f);
    glVertex2f(-0.5f, 0.9f);
    glVertex2f(-0.2f, 0.6f);
    glVertex2f(0.3f, 0.8f);
    glVertex2f(0.7f, 0.5f);
    glVertex2f(0.9f, 0.9f);
    glVertex2f(-0.9f, 0.4f);

    glVertex2f(0.1f, 0.9f);
    glVertex2f(0.5f, 0.7f);
    glVertex2f(-0.4f, 0.3f);
    glVertex2f(0.8f, 0.2f);
    glVertex2f(-0.1f, 0.85f);

    glEnd();
}

void drawSky() {
    red = 0;
	green = 0;
	blue = (sin(sunAngle) + 1.0f) / 2.0f; // Adjust blue to be between 0 and 1
	glClearColor(red, green, blue, 1.0f); // Set the background color
}

void drawSun() {
    glPushMatrix();// saves current transformation

    float x = cos(sunAngle) * 0.85f;
    float y = sin(sunAngle) * 0.85f;
    glTranslatef(x, y, 0.0f);
    
    // Draw sun as a yellow sphere
    glColor3f(1.0f, 1.0f, 0.0f);  // Yellow
    glutSolidSphere(0.15f, 32, 32);
    
    glPopMatrix();
}

void drawMoon() {
    glPushMatrix();

    float x = cos(moonAngle) * 0.85f;
    float y = sin(moonAngle) * 0.85f;
    glTranslatef(x, y, 0.0f);//moves the drawing position

    glColor3f(1.0f, 1.0f, 1.0f); 
    glutSolidSphere(0.15f, 32, 32);
    glPopMatrix();//restores the old transformation state

}

void display() {
    drawSky();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// clears the screen before drawing next frame
    glLoadIdentity();// resets the transformation matrix
    
    drawStars();
    drawSun();
    drawMoon();

    glutSwapBuffers();// swaps hidden buffer with visible buffer
}

void idle()
{
    sunAngle += 0.0001f;
    moonAngle += 0.0001f;
    if (sunAngle> 2 * 3.14159f )
        sunAngle = 0.0f;
    if (moonAngle > 2 * 3.14159f)
        moonAngle = 0.0f;
	glutPostRedisplay();// redraws the screen with updated positions of sun and moon
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Moving Sun");
    
    glEnable(GL_DEPTH_TEST);
    glutDisplayFunc(display);
    glutIdleFunc(idle);// runs idle function
    
    glutMainLoop();
    return 0;
}
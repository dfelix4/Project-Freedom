//Author: Andres Zamora
//Date: 9/26/18

#include "fonts.h"
#include <iostream>
#include <GL/glx.h>
#include <math.h>
using namespace std;

void showAndresName(int x, int y)
{
    Rect r;
    r.bot = y;
    r.left = x;
    r.center = 0;
    ggprint16(&r, 16, 0x00ff0000, "Andres Zamora");
}
void showAndresPic(int x, int y, GLuint textid)
{
    static float angle = 0.0f;
    float fx = (float)x;
    float fy = (float)y;
    angle += 0.5f;
    fx += cos(angle) * 10.0f;
    fy += sin(angle) * 10.0f;

    glColor3ub(255,255,255);
    int wid = 40;
    glPushMatrix();

    glTranslatef(x,y,0);
    glRotatef(angle, 0.0f, 0.0f ,1.0f);

    glBindTexture(GL_TEXTURE_2D, textid);
    //glEnable(GL_ALPHA_TEST);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex2i(-wid,-wid);
    glTexCoord2f(0.0f, 0.0f); glVertex2i(-wid, wid);
    glTexCoord2f(1.0f, 0.0f); glVertex2i( wid, wid);
    glTexCoord2f(1.0f, 1.0f); glVertex2i( wid,-wid);
    glEnd();
    glPopMatrix();

}


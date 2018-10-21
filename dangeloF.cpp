//Author: D'Angelo Felix
//Date: 10/20/18

#include "fonts.h"
#include <GL/glx.h>
#include <cmath>

void creditsD(int x, int y)
{
    Rect r;
    r.bot = y;
    r.left = x;
    r.center = 0;
    ggprint16(&r, 16, 0x00ff0000, "D'Angelo Felix");
}

void showDPic(int x, int y, GLuint textid)
{
    static float angle = 0.0f;
    float fx = (float)x;
    float fy = (float)y;
    angle += 0.5f;
    //fx += sin(angle);
    fx += cos(angle) * 10.0f;
    fy += sin(angle) * 10.0f;

    glColor3ub(255,255,255);
    int wid = 40;
    glPushMatrix();

    glTranslatef(x,y,0);
    glRotatef(angle, 0.0f, 0.0f ,1.0f);

    glBindTexture(GL_TEXTURE_2D, textid);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex2i(-wid,-wid);
    glTexCoord2f(0.0f, 0.0f); glVertex2i(-wid, wid);
    glTexCoord2f(1.0f, 0.0f); glVertex2i( wid, wid);
    glTexCoord2f(1.0f, 1.0f); glVertex2i( wid,-wid);
    glEnd();
    glPopMatrix();
}

void test() 
{
    printf("I'm right here!\n");
}
void fireball() 
{
}
void deagle()
{
}
void shottie () 
{
}

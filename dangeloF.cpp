//Author: D'Angelo Felix
//Date: 9/26/18

#include "fonts.h"
#include <GL/glx.h>
#include<iostream>
using namespace std;
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
 glColor3ub(255,255,255);
                int wid = 40;
                glPushMatrix();
                glTranslatef(x,y,0);
                glBindTexture(GL_TEXTURE_2D, textid);
                glBegin(GL_QUADS);
                glTexCoord2f(0.0f, 1.0f); glVertex2i(-wid,-wid);
                glTexCoord2f(0.0f, 0.0f); glVertex2i(-wid, wid);
                glTexCoord2f(1.0f, 0.0f); glVertex2i( wid, wid);
                glTexCoord2f(1.0f, 1.0f); glVertex2i( wid,-wid);
                glEnd();
                glPopMatrix();
}

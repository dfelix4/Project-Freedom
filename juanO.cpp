//Author: Juan Orozco
//Date: 9/26/18

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cmath>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "fonts.h"
void ShowCredits(int x, int y)
{
    	Rect r;
	r.bot =  y;
        r.left = x;
        r.center = 0;
     ggprint16(&r, 16, 0x00ff0000, "\nJuan Orozco\n ");

}
void JuanPicture(int x, int y, GLuint texid)
{
    glColor3ub(255,255,255);
    int wid=30;
    glPushMatrix();
    glTranslatef(x, y, 0);
    glBindTexture(GL_TEXTURE_2D, texid);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex2i(-wid,-wid);
    glTexCoord2f(0.0f, 0.0f); glVertex2i(-wid, wid);
    glTexCoord2f(1.0f, 0.0f); glVertex2i( wid, wid);
    glTexCoord2f(1.0f, 1.0f); glVertex2i( wid,-wid);

    glEnd();
    glPopMatrix();

}

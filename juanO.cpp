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
    static float angle = 0.0f;
    float fx = (float)x;
    float fy = (float)y;
    angle -= 1.0f;
    fx += cos(angle) * 10.0f;
    fy += sin(angle) * 10.0f;

    glColor3ub(255,255,255);
    int wid = 40;
    glPushMatrix();

    glTranslatef(x,y,0);
    glRotatef(angle, 0.0f, 0.0f ,1.0f);

    glBindTexture(GL_TEXTURE_2D, texid);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex2i(-wid,-wid);
    glTexCoord2f(0.0f, 0.0f); glVertex2i(-wid, wid);
    glTexCoord2f(1.0f, 0.0f); glVertex2i( wid, wid);
    glTexCoord2f(1.0f, 1.0f); glVertex2i( wid,-wid);

    glEnd();
    glPopMatrix();

}
//Juan Orozco
void showEagle(float x, float y, GLuint g)
{    //output eagle
    glColor3ub(255,255,255);

    int wid=80;
    glPushMatrix();
    glTranslatef(x, y, 0);
    glBindTexture(GL_TEXTURE_2D, g);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0f);
    glColor4ub(255,255,255,255);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex2i(-wid,-wid);
    glTexCoord2f(0.0f, 0.0f); glVertex2i(-wid, wid);
    glTexCoord2f(1.0f, 0.0f); glVertex2i( wid, wid);
    glTexCoord2f(1.0f, 1.0f); glVertex2i( wid,-wid);

    glEnd();
    glPopMatrix();
}
//Juan Orozco
void showBackground(int x,GLuint g)
{
    glColor3ub(255,255,255);

    int back = x -200 ;
    glPushMatrix();
    glTranslatef(255, 255, 0);
    glBindTexture(GL_TEXTURE_2D, g);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex2i(-back,-back);
    glTexCoord2f(0.0f, 0.0f); glVertex2i(-back, back);
    glTexCoord2f(1.0f, 0.0f); glVertex2i( back, back);
    glTexCoord2f(1.0f, 1.0f); glVertex2i( back,-back);

    glEnd();
    glPopMatrix();
    //----------------
}


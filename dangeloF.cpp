//Author: D'Angelo Felix
//Date: 10/20/18

#include "fonts.h"
#include <GL/glx.h>
#include <cmath>
#include <stdio.h>
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
}/*
void fireball() 
{
}
void deagle_time()
{
    //a little time between each bullet
    struct timespec bt;
    clock_gettime(CLOCK_REALTIME, &bt);
    double ts = timeDiff(&g.bulletTimer, &bt);
    if (ts > 0.1) {
        timeCopy(&g.bulletTimer, &bt);
        if (g.nbullets < MAX_BULLETS) {
            //shoot a bullet...
            //Bullet *b = new Bullet;
            Bullet *b = &g.barr[g.nbullets];
            timeCopy(&b->time, &bt);
            b->pos[0] = g.ship.pos[0];
            b->pos[1] = g.ship.pos[1];
            b->vel[0] = g.ship.vel[0];
            b->vel[1] = g.ship.vel[1];
            //convert ship angle to radians
            Flt rad = ((g.ship.angle+90.0) / 360.0f) * PI * 2.0;
            //convert angle to a vector
            Flt xdir = cos(rad);
            Flt ydir = sin(rad);
            b->pos[0] += xdir*20.0f;
            b->pos[1] += ydir*20.0f;
            b->vel[0] += xdir*6.0f + rnd()*0.1;
            b->vel[1] += ydir*6.0f + rnd()*0.1;
            b->color[0] = 1.0f;
            b->color[1] = 1.0f;
            b->color[2] = 1.0f;
            g.nbullets++;
        }
    }
}
void deagle_speed() 
{
}
void deagle_impact() 
{
    memcpy(&g.barr[i], &g.barr[g.nbullets-1], sizeof(Bullet));
    g.nbullets--;
}
void shottie () 
{
}
*/

//Author: D'Angelo Felix
//Date: 10/20/18
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cmath>
using namespace std;

#include <unistd.h>
#include <X11/Xlib.h>
//#include <X11/Xutil.h>
//#include <GL/gd.h>
//#include <GL/gdu.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "log.h"
#include "fonts.h"


#include "fonts.h"
#include <GL/glx.h>
#include <cmath>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctime>
#ifndef _CLASS_
typedef float Flt;
typedef float Vec[3];
typedef Flt	Matrix[4][4];

//macros
#define rnd() (((Flt)rand())/(Flt)RAND_MAX)
#define random(a) (rand()%(a))
#define VecZero(v) (v)[0]=0.0,(v)[1]=0.0,(v)[2]=0.0
#define MakeVector(x, y, z, v) (v)[0]=(x),(v)[1]=(y),(v)[2]=(z)
#define VecCopy(a,b) (b)[0]=(a)[0];(b)[1]=(a)[1];(b)[2]=(a)[2]
#define VecDot(a,b)	((a)[0]*(b)[0]+(a)[1]*(b)[1]+(a)[2]*(b)[2])
#define VecSub(a,b,c) (c)[0]=(a)[0]-(b)[0]; \
                             (c)[1]=(a)[1]-(b)[1]; \
(c)[2]=(a)[2]-(b)[2]
const float TIMESLICE = 1.0f;
const float GRAVITY = -0.2f;
#define PI 3.141592653589793
#define ALPHA 1
const int MAX_BULLETS = 11;
const Flt MINIMUM_ASTEROID_SIZE = 60.0;

//-----------------------------------------------------------------------------
//Setup timers
const double OOBILLION = 1.0 / 1e9;
extern struct timespec timeStart, timeCurrent;
extern double timeDiff(struct timespec *start, struct timespec *end);
extern void timeCopy(struct timespec *dest, struct timespec *source);
//-----------------------------------------------------------------------------

class Image {
    public:
        int width, height;
        unsigned char *data;
        ~Image() { delete [] data; }
        Image(const char *fname) {
            if (fname[0] == '\0')
                return;
            //printf("fname **%s**\n", fname);
            int ppmFlag = 0;
            char name[40];
            strcpy(name, fname);
            int slen = strlen(name);
            char ppmname[80];
            if (strncmp(name+(slen-4), ".ppm", 4) == 0)
                ppmFlag = 1;
            if (ppmFlag) {
                strcpy(ppmname, name);
            } else {
                name[slen-4] = '\0';
                //printf("name **%s**\n", name);
                sprintf(ppmname,"%s.ppm", name);
                //printf("ppmname **%s**\n", ppmname);
                char ts[100];
                //system("convert eball.jpg eball.ppm");
                sprintf(ts, "convert %s %s", fname, ppmname);
                system(ts);
            }
            //sprintf(ts, "%s", name);
            FILE *fpi = fopen(ppmname, "r");
            if (fpi) {
                char line[200];
                fgets(line, 200, fpi);
                fgets(line, 200, fpi);
                //skip comments and blank lines
                while (line[0] == '#' || strlen(line) < 2)
                    fgets(line, 200, fpi);
                sscanf(line, "%i %i", &width, &height);
                fgets(line, 200, fpi);
                //get pixel data
                int n = width * height * 3;			
                data = new unsigned char[n];			
                for (int i=0; i<n; i++)
                    data[i] = fgetc(fpi);
                fclose(fpi);
            } else {
                printf("ERROR opening image: %s\n",ppmname);
                exit(0);
            }
            if (!ppmFlag)
                unlink(ppmname);
        }
};
//------------------------From Background Framework----------------------------
class Texture {
    public:
        Image *backImage;
        GLuint backTexture;
        float xc[2];
        float yc[2];
};
//-----------------------------------------------------------------------------

class Global {
    public:
        int xres, yres;
        char keys[65536];
        //--------------------------------
        bool credits;
        int menuState = 0;
        Texture tex;
        GLuint perfectTexture;
        GLuint nobleTexture;
        GLuint tigerTexture;
        GLuint spongebobTexture;
        //--------------------------------
        Global() {
            xres = 1250;
            yres = 900;
            memset(keys, 0, 65536);
        }
} gd;

class Ship {
    public:
        Vec dir;
        Vec pos;
        Vec vel;
        float angle;
        float color[3];
    public:
        Ship() {
            VecZero(dir);
            pos[0] = (Flt)(gd.xres/2);
            pos[1] = 525;
            pos[2] = 0.0f;
            VecZero(vel);
            angle = -90.0;
            color[0] = color[1] = color[2] = 1.0;
        }
};

class Bullet {
    public:
        Vec pos;
        Vec vel;
        float color[3];
        struct timespec time;
        int rate;
    public:
        Bullet() {
            rate = 0; 
        }
};

class Asteroid {
    public:
        //Vec pos;
        //
        Vec vel;
        int nverts;
        //Flt radius;
        Vec vert[8];
        float angle;
        float rotate;
        //float color[3];

        int life;        
        float width, height;
        Vec pos;
        Flt radius;
        float color[3];
        struct Asteroid *prev;
        struct Asteroid *next;
    public:
        Asteroid() {
            prev = NULL;
            next = NULL;
            life = 5;
        }
};

class Game {
    public:
        Ship ship;
        Asteroid *ahead;
        Bullet *barr;
        //Shape *box;
        int nasteroids;
        int nbullets;
        struct timespec bulletTimer;
        struct timespec mouseThrustTimer;
        bool mouseThrustOn;
        int lives;
        //--------------------------------
        struct timespec gTime;
        GLuint eagdeSprite;
        GLuint eagdeNone;
        GLuint backgroundTexture;
        //GameTime *runningTime;
        //GameTime *pauseTime;
        //--------------------------------
    public:
        Game() {
            lives = 0;
            clock_gettime(CLOCK_REALTIME, &gTime);
            ahead = NULL;
            //pauseTime = new GameTime;
            //runningTime = new GameTime;
            barr = new Bullet[MAX_BULLETS];
            nasteroids = 0;
            nbullets = 0;
            mouseThrustOn = false;
            //build 10 asteroids...
            int piece = 0;
            for (int j=0; j<10; j++) {
                Asteroid *a = new Asteroid;
                //a->width = 100;
                //a->height = 100;
                a->nverts = 8;
                a->radius = 100;
                Flt r2 = a->radius / 2.0;
                Flt angle = 0.0f;
                Flt inc = (PI * 2.0) / (Flt)a->nverts;
                for (int i=0; i<a->nverts; i++) {
                    a->vert[i][0] = sin(angle) * (r2 + rnd() * a->radius);
                    a->vert[i][1] = cos(angle) * (r2 + rnd() * a->radius);
                    angle += inc;
                }
                //a->pos[0] = (Flt)(rand() % gd.xres);
                //a->pos[1] = (Flt)(rand() % gd.xres);
                int position = piece % 4;
                //a->pos[0] = (Flt)(910 + 5 * 65);
                a->pos[0] = (Flt)(gd.xres + 500);
                a->pos[1] = (Flt)(position*250);
                piece++;
                //a->pos[1] = (Flt)(900 - 5*60);
                a->pos[2] = 0.0f;
                a->angle = 0.0;
                a->rotate = rnd() * 4.0 - 2.0;
                a->color[0] = 0.8;
                a->color[1] = 0.8;
                a->color[2] = 0.7;
                a->vel[0] = (Flt)(rnd()*-20.0-1.0);
                a->vel[1] = (Flt)(0);//rnd()*2.0-1.0);
                a->next = ahead;
                if (ahead != NULL)
                    ahead->prev = a;
                ahead = a;
                ++nasteroids;
                /*Asteroid *a = new Asteroid;
                  a->nverts = 8;
                  a->radius = rnd()*80.0 + 40.0;
                  Flt r2 = a->radius / 2.0;
                  Flt angle = 0.0f;
                  Flt inc = (PI * 2.0) / (Flt)a->nverts;
                  for (int i=0; i<a->nverts; i++) {
                  a->vert[i][0] = sin(angle) * (r2 + rnd() * a->radius);
                  a->vert[i][1] = cos(angle) * (r2 + rnd() * a->radius);
                  angle += inc;
                  }
                  a->pos[0] = (Flt)(rand() % gd.xres);
                  a->pos[1] = (Flt)(rand() % gd.yres);
                  a->pos[2] = 0.0f;
                  a->angle = 0.0;
                  a->rotate = rnd() * 4.0 - 2.0;
                  a->color[0] = 0.8;
                  a->color[1] = 0.8;
                  a->color[2] = 0.7;
                  a->vel[0] = (Flt)(rnd()*2.0-1.0);
                  a->vel[1] = (Flt)(rnd()*2.0-1.0);
                //std::cout << "asteroid" << std::endl;
                //add to front of linked list
                a->next = ahead;
                if (ahead != NULL)
                ahead->prev = a;
                ahead = a;
                ++nasteroids;*/
            }
            clock_gettime(CLOCK_REALTIME, &bulletTimer);
        }
        ~Game() {
            delete [] barr;
        }
};
#endif
struct timespec st;
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
/*void fireball() 
  {
  }*/

int deagle_time(Game &g, int cLife)
{
    //printf("Lives#1: %d\n",g.lives);
    g.lives = cLife;
    //printf("Lives#2: %d\n",g.lives);
    //a little time between each bullet
    struct timespec bt;
    clock_gettime(CLOCK_REALTIME, &bt);
    double ts = timeDiff(&g.bulletTimer, &bt);
    printf("\n\nSOme shit %lf\n\n", ts);
    if (ts > 1.0) {
        printf("\n\nSomething else %lf\n\n", ts);
        timeCopy(&g.bulletTimer, &bt);
        if (g.nbullets < MAX_BULLETS) {
            printf("\n\nrandom shit or some\n\n");
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
            b->vel[0] += xdir*25.0f + rnd()*0.95;
            b->vel[1] += ydir*25.0f + rnd()*0.95;
            b->color[0] = 1.0f;
            b->color[1] = 1.0f;
            b->color[2] = 1.0f;
            g.nbullets++;
        }
        //  printf("Lives#3: %d\n",g.lives);
    }
    return(g.lives);
}
void deagle_speed(Game &g, Global &gl) 
{
    struct timespec bt;
    clock_gettime(CLOCK_REALTIME, &bt);
    int i=0;
    while (i < g.nbullets) {
        Bullet *b = &g.barr[i];
        //How long has bullet been alive?
        double ts = timeDiff(&b->time, &bt);
        if (ts > 2.5) {
            //time to delete the bullet.
            memcpy(&g.barr[i], &g.barr[g.nbullets-1],
                    sizeof(Bullet));
            g.nbullets--;
            //do not increment i.
            continue;
        }
        //move the bullet
        b->pos[0] += b->vel[0]+10;
        b->pos[1] += b->vel[1]+10;
        //Check for collision with window edges
        if (b->pos[0] < 0.0) {
            b->pos[0] += (float)gl.xres;
        }
        else if (b->pos[0] > (float)gl.xres) {
            b->pos[0] -= (float)gl.xres;
        }
        else if (b->pos[1] < 0.0) {
            b->pos[1] += (float)gl.yres;
        }
        else if (b->pos[1] > (float)gl.yres) {
            b->pos[1] -= (float)gl.yres;
        }
        i++;
    }

}
/*
   void deagde_impact() 
   {
   memcpy(&g.barr[i], &g.barr[g.nbullets-1], sizeof(Bullet));
   g.nbullets--;
   }
   void shottie () 
   {
   }*/

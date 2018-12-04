//Author: Juan Orozco
//Date: 9/26/18



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
class Sprite {
    public:
        int onoff;
        int frame;
        double delay;
        Vec pos;
        Image *image;
        GLuint tex;
        struct timespec time;
        Sprite() {
            onoff = 0;
            frame = 0;
            image = NULL;
            delay = 0.1;
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
class Timers {
public:
    double physicsRate;
    double oobillion;
    struct timespec timeStart, timeEnd, timeCurrent;
    struct timespec walkTime;
    Timers() {
        physicsRate = 1.0 / 30.0;
        oobillion = 1.0 / 1e9;
    }
    double timeDiff(struct timespec *start, struct timespec *end) {
        return (double)(end->tv_sec - start->tv_sec ) +
                (double)(end->tv_nsec - start->tv_nsec) * oobillion;
    }
    void timeCopy(struct timespec *dest, struct timespec *source) {
        memcpy(dest, source, sizeof(struct timespec));
    }
    void recordTime(struct timespec *t) {
        clock_gettime(CLOCK_REALTIME, t);
    }
} timerss;

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
        Sprite exp;
        //--------------------------------
        Global() {
            xres = 1250;
            yres = 900;
            memset(keys, 0, 65536);
            exp.onoff=0;
            exp.frame=0;
            exp.image=NULL;
            exp.delay = 0.02;

        }
} gj;

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
            pos[0] = (Flt)(gj.xres/2);
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
                a->pos[0] = (Flt)(gj.xres + 500);
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
void showFireBall(Global gl)
{
    //float cx = gl.xres/2.0;
    //float cy = gl.yres/2.0;

    //float h = 80.0;
    //float w = 80.0;
    /////////////////////////////////////////////////////////////////////////////////

    //	timers.recordTime(&gl.exp.time);
    gl.exp.onoff ^= 1;
   // gl.exp.pos[0] = g.ship.pos[0];
   // gl.exp.pos[1] = g.ship.pos[1];
    //gl.exp.pos[2] = 0;
    glPushMatrix();
    glColor3f(1.0, 1.0, 1.0);
    glBindTexture(GL_TEXTURE_2D, gl.exp.tex);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0f);
    glColor4ub(255,255,255,255);
    //glTranslated(gl.exp.pos[0], gl.exp.pos[1], gl.exp.pos[2]);
   // glTranslatef(g.ship.pos[0], g.ship.pos[1], gl.exp.pos[2]);

    int ix = gl.exp.frame % 5;
    int iy = gl.exp.frame / 5;
    float tx = (float)ix / 5.0;
    float ty = (float)iy / 5.0;
    int wid = 500;
    glBegin(GL_QUADS);
    /*
       glTexCoord2f(0.0f, 1.0f); glVertex2i(-wid,-wid);
       glTexCoord2f(0.0f, 0.0f); glVertex2i(-wid, wid);
       glTexCoord2f(1.0f, 0.0f); glVertex2i( wid, wid);
       glTexCoord2f(1.0f, 1.0f); glVertex2i( wid,-wid);

*/
    glTexCoord2f(tx,      ty+0.25); glVertex2i(-wid, -wid);
    glTexCoord2f(tx,      ty);      glVertex2i(-wid, wid);
    glTexCoord2f(tx+0.25, ty);      glVertex2i(wid, wid);
    glTexCoord2f(tx+0.25, ty+0.25); glVertex2i(wid, -wid);

    glEnd();
    glPopMatrix();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_ALPHA_TEST);
}
void showLifeBar(int health, int max)
{
    //glClear(GL_COLOR_BUFFER_BIT);
    glPushMatrix();
    glBegin(GL_QUADS);
    float barWidth;
    float remaining = (float)health/max;
    if (remaining == 1) {
            barWidth = 100;
    } else {
        barWidth = remaining*100;
    }
    printf("B: %f, h: %d r:%f\n", barWidth, health, remaining);
    glColor3ub(0, 0, 0);
    //glTexCoord2f(0.0f, 1.0f); 
    glVertex2i(0, 875);
    //glTexCoord2f(0.0f, 0.0f); 
    glVertex2i(0, 900);
    //glTexCoord2f(1.0f, 1.0f); 
    glVertex2i(barWidth*5, 900);
    //glTexCoord2f(1.0f, 1.0f); 
    glVertex2i(barWidth*5, 875);
    glEnd();
    glPopMatrix();

}

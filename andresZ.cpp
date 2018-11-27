//Author: Andres Zamora
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
//#include <GL/gl.h>
//#include <GL/glu.h>
#include <X11/keysym.h>

#include "fonts.h"
#include <GL/glx.h>
#include <math.h>
#ifndef _CLASS_
//defined types
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
//constants
const float TIMESLICE = 1.0f;
const float GRAVITY = -0.2f;
#define PI 3.141592653589793
#define ALPHA 1
const int MAX_BULLETS = 20;
const Flt MINIMUM_ASTEROID_SIZE = 60.0;

//-----------------------------------------------------------------------------
//Setup timers
const double OOBILLION = 1.0 / 1e9;
extern struct timespec timeStart, timeCurrent;
extern double timeDiff(struct timespec *start, struct timespec *end);
extern void timeCopy(struct timespec *dest, struct timespec *source);
//-----------------------------------------------------------------------------

//------------------------ADDED Image class from Rainforrest Framework---------
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
} ga;

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
            pos[0] = (Flt)(ga.xres/2);
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
    public:
        Bullet() { }
};

class Asteroid {
    public:
        //Vec pos;
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
class GameTime {
    public: 
        struct timespec rTime;
    public:
        GameTime() {
            clock_gettime(CLOCK_REALTIME, &rTime);
        }
};
//AZ

/*
class Shape {
public:
	float width, height;
	float radius;
	Vec center;
    float color[3];
    struct Shape *prev;
    struct Shape *next;
public:
	Shape() {
        prev = NULL;
		next = NULL;
	}
};
*/

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
        //--------------------------------
        struct timespec gTime;
        GLuint eagleSprite;
        GLuint eagleNone;
        GLuint backgroundTexture;
        GameTime *runningTime;
        //--------------------------------
    public:
        Game() {
            clock_gettime(CLOCK_REALTIME, &gTime);
            ahead = NULL;
            runningTime = new GameTime;
            barr = new Bullet[MAX_BULLETS];
            nasteroids = 0;
            nbullets = 0;
            mouseThrustOn = false;
            //build 10 asteroids...
			int piece = 1;
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
				//a->pos[0] = (Flt)(rand() % ga.xres);
				//a->pos[1] = (Flt)(rand() % ga.xres);
                //int position = piece % 4;
				//if (piece == 5)
                    int position = piece %5;
                if (position == 0)
                    position+=1;
				//a->pos[0] = (Flt)(910 + 5 * 65);
				a->pos[0] = (Flt)(ga.xres + 500);
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
                a->pos[0] = (Flt)(rand() % ga.xres);
                a->pos[1] = (Flt)(rand() % ga.yres);
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
    //gaEnable(GL_ALPHA_TEST);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex2i(-wid,-wid);
    glTexCoord2f(0.0f, 0.0f); glVertex2i(-wid, wid);
    glTexCoord2f(1.0f, 0.0f); glVertex2i( wid, wid);
    glTexCoord2f(1.0f, 1.0f); glVertex2i( wid,-wid);
    glEnd();
    glPopMatrix();

}

void showEnemy(Game &g)
{
    //------------------
    //Draw the asteroids
        Asteroid *a = g.ahead;
            //showEnemy(g.eagleNone, );
        while (a) {
            //showEnemy(g.eagleNone, Vec, Vec, Vec, float);
            
            /*
            //Log("draw asteroid...\n");
            glColor3fv(a->color);
            glPushMatrix();
            glTranslatef(a->pos[0], a->pos[1], a->pos[2]);
            glRotatef(a->angle, 0.0f, 0.0f, 1.0f);
            glBegin(GL_LINE_LOOP);
            //Log("%i verts\n",a->nverts);
            for (int j=0; j<a->nverts; j++) {
                glVertex2f(a->vert[j][0], a->vert[j][1]);
            }
            glEnd();
            glPopMatrix();
            glColor3f(1.0f, 0.0f, 0.0f);
            glBegin(GL_POINTS);
            glVertex2f(a->pos[0], a->pos[1]);
            glEnd();
            a = a->next;
            */
            glPushMatrix();
            glTranslatef(a->pos[0], a->pos[1], a->pos[2]);
            glRotatef(a->angle, 0.0f, 0.0f, 1.0f);
            for (int i = 0; i < 10; i++) {
            int random = 0;
            int num = random %2;
            if (num == 0)
                glBindTexture(GL_TEXTURE_2D, g.eagleNone);
            else if (num == 1)
                glBindTexture(GL_TEXTURE_2D, g.eagleSprite);
            random++;
            }
            //glBindTexture(GL_TEXTURE_2D, g.eagleNone);
            glEnable(GL_ALPHA_TEST);
            glAlphaFunc(GL_GREATER, 0.0f);
            glColor4ub(255,255,255,255);
            glBegin(GL_QUADS);
            //for (int j=0; j<a->nverts; j++) {
            //    glVertex2f(a->vert[j][0], a->vert[j][1]);
            //}
            glTexCoord2f( 0.0f, 1.0f); 
            glVertex2i(   -80,  -80);
            glTexCoord2f( 0.0f, 0.0f); 
            glVertex2i(   -80,  80);
            glTexCoord2f( 1.0f, 0.0f); 
            glVertex2i(   80,   80);
            glTexCoord2f( 1.0f, 1.0f); 
            glVertex2i(   80,   -80);
            glEnd();
            glPopMatrix();
            //
            //
            glPushMatrix();
            glColor3f(1.0f, 0.0f, 0.0f);
            glBegin(GL_POINTS);
            glVertex2f(a->pos[0], a->pos[1]);
            glEnd();
            glPopMatrix();
            //
            a = a->next;
        }
}


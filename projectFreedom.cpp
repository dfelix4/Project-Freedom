//Modified by: D'Angelo Felix
//             Juan Orozco	
//             Joshua Rodriguez
//             Andres Zamora
//Date Modified: 9/26/18             
//program: asteroids.cpp
//author:  Gordon Griesel
//date:    2014 - 2018
//mod spring 2015: added constructors
//mod spring 2018: X11 wrapper class
//This program is a game starting point for a 3350 project.
//
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
#include <GL/glx.h>
#include "log.h"
#include "fonts.h"

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
#define _CLASS_ 1
//constants
const float TIMESLICE = 1.0f;
const float GRAVITY = -0.2f;
#define PI 3.141592653589793
#define ALPHA 1
const int MAX_BULLETS = 11;
const Flt MINIMUM_ASTEROID_SIZE = 100.0;

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
Image img[10] = {
    "./images/notperfect.jpg",
    "./images/spongebob.jpg",
    "./images/Noblelogo.jpg",
    "./images/tiger.jpg",
    "./images/eagle.jpg",
    "./images/city.jpg",
    "./images/title_screennew.jpg",
    "./images/enemy.jpg",
    "./images/GameOver.jpg",
    "./images/exp.png"
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
        Image *titleImage;
        GLuint titleTexture;
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
} timers;

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
        GLuint enemyTexture;
        GLuint gameOverTexture;
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
} gl;

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
            pos[0] = (Flt)(gl.xres/2);
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
}bullet;

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
            life = 3;
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
        int lives;
        int died;
        struct timespec bulletTimer;
        struct timespec mouseThrustTimer;
        bool mouseThrustOn;
        bool gameover;
        //--------------------------------
        struct timespec gTime;
        GLuint eagleSprite;
        GLuint eagleNone;
        GLuint backgroundTexture;
        GLuint enemyNone;
        GLuint tileScreen;
        GLuint gameOverScreen;
        //GameTime *runningTime;
        //GameTime *pauseTime;
        //--------------------------------
    public:
        Game() {
            died = 0;
            gameover = false;
            clock_gettime(CLOCK_REALTIME, &gTime);
            ahead = NULL;
            //pauseTime = new GameTime;
            //runningTime = new GameTime;
            barr = new Bullet[MAX_BULLETS];
            nasteroids = 0;
            lives = 3;
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
                //a->pos[0] = (Flt)(rand() % gl.xres);
                //a->pos[1] = (Flt)(rand() % gl.xres);
                int position = piece % 4;
                //a->pos[0] = (Flt)(910 + 5 * 65);
                a->pos[0] = (Flt)(gl.xres + 500);
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
                  a->pos[0] = (Flt)(rand() % gl.xres);
                  a->pos[1] = (Flt)(rand() % gl.yres);
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
} g;

//X Windows variables
class X11_wrapper {
    private:
        Display *dpy;
        Window win;
        GLXContext glc;
    public:
        X11_wrapper() {
            GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
            //GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, None };
            XSetWindowAttributes swa;
            setup_screen_res(gl.xres, gl.yres);
            dpy = XOpenDisplay(NULL);
            if (dpy == NULL) {
                std::cout << "\n\tcannot connect to X server" << std::endl;
                exit(EXIT_FAILURE);
            }
            Window root = DefaultRootWindow(dpy);
            XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
            if (vi == NULL) {
                std::cout << "\n\tno appropriate visual found\n" << std::endl;
                exit(EXIT_FAILURE);
            } 
            Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
            swa.colormap = cmap;
            swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
                PointerMotionMask | MotionNotify | ButtonPress | ButtonRelease |
                StructureNotifyMask | SubstructureNotifyMask;
            win = XCreateWindow(dpy, root, 0, 0, gl.xres, gl.yres, 0,
                    vi->depth, InputOutput, vi->visual,
                    CWColormap | CWEventMask, &swa);
            set_title();
            glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
            glXMakeCurrent(dpy, win, glc);
            //show_mouse_cursor(0);//--------------------------------Not needed
        }
        ~X11_wrapper() {
            XDestroyWindow(dpy, win);
            XCloseDisplay(dpy);
        }
        void set_title() {
            //Set the window title bar.
            XMapWindow(dpy, win);
            XStoreName(dpy, win, "Project Freedom");//------------
        }
        void check_resize(XEvent *e) {
            //The ConfigureNotify is sent by the
            //server if the window is resized.
            if (e->type != ConfigureNotify)
                return;
            XConfigureEvent xce = e->xconfigure;
            if (xce.width != gl.xres || xce.height != gl.yres) {
                //Window size did change.
                reshape_window(xce.width, xce.height);
            }
        }
        void reshape_window(int width, int height) {
            //window has been resized.
            setup_screen_res(width, height);
            glViewport(0, 0, (GLint)width, (GLint)height);
            glMatrixMode(GL_PROJECTION); glLoadIdentity();
            glMatrixMode(GL_MODELVIEW); glLoadIdentity();
            glOrtho(0, gl.xres, 0, gl.yres, -1, 1);
            set_title();
        }
        void setup_screen_res(const int w, const int h) {
            gl.xres = w;
            gl.yres = h;
        }
        void swapBuffers() {
            glXSwapBuffers(dpy, win);
        }
        bool getXPending() {
            return XPending(dpy);
        }
        XEvent getXNextEvent() {
            XEvent e;
            XNextEvent(dpy, &e);
            return e;
        }
        void set_mouse_position(int x, int y) {
            XWarpPointer(dpy, None, win, 0, 0, 0, 0, x, y);
        }
        void show_mouse_cursor(const int onoff) {
            if (onoff) {
                //this removes our own blank cursor.
                XUndefineCursor(dpy, win);
                return;
            }
            //vars to make blank cursor
            Pixmap blank;
            XColor dummy;
            char data[1] = {0};
            Cursor cursor;
            //make a blank cursor
            blank = XCreateBitmapFromData (dpy, win, data, 1, 1);
            if (blank == None)
                std::cout << "error: out of memory." << std::endl;
            cursor = XCreatePixmapCursor(dpy, blank, blank, &dummy, &dummy, 0, 0);
            XFreePixmap(dpy, blank);
            //this makes you the cursor. then set it using this function
            XDefineCursor(dpy, win, cursor);
            //after you do not need the cursor anymore use this function.
            //it will undo the last change done by XDefineCursor
            //(thus do only use ONCE XDefineCursor and then XUndefineCursor):
        }
} x11;

//function prototypes
void init_opengl();
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void physics();
void render();
extern void newScoreboard();
extern void showMainMenu(int, GLuint, int);
extern void showGameOver(int, GLuint, int);
extern void showPauseMenu();
extern void addPauseTime(struct timespec*);
enum {
    MAIN_MENU,
    GAME_RUNNING,
    GAME_OVER,
    PAUSE_GAME,
};
void show_credits() 
{
    glClear(GL_COLOR_BUFFER_BIT);    
    Rect r;
    r.bot = gl.yres - 100;
    r.left = 530;
    r.center = 0;
    ggprint16(&r, 16, 0x00ffffff, "Credits");

    extern void creditsD(int x, int y);
    creditsD(500,gl.yres-150);
    extern void ShowCredits(int x, int y);
    ShowCredits(500,gl.yres-250);
    extern void creditJosh(int x, int y);
    creditJosh(500,gl.yres-350);
    extern void showAndresName(int x, int y);
    showAndresName(500,gl.yres-450);

    extern void showDPic(int, int, GLuint);
    showDPic(400, gl.yres-150, gl.perfectTexture);
    extern void JuanPicture(int, int, GLuint);
    JuanPicture(400, gl.yres-250, gl.spongebobTexture);
    extern void showJoshPicture(int, int, GLuint);
    showJoshPicture(400, gl.yres-350, gl.nobleTexture);
    extern void showAndresPic(int, int, GLuint);
    showAndresPic(400, gl.yres-450, gl.tigerTexture);

}
extern void newTime(struct timespec);
extern void runningTime(struct timespec);

//==========================================================================
// M A I N
//==========================================================================
int main()
{
    //int pauseTimes = 0;
    int limitScoreboard = 0;
    bool paused = 0;
    logOpen();
    init_opengl();
    srand(time(NULL));
    x11.set_mouse_position(100, 100);
    int done=0;
    while (!done) {
        while (x11.getXPending()) {
            XEvent e = x11.getXNextEvent();
            x11.check_resize(&e);
            check_mouse(&e);
            done = check_keys(&e);
        }
        switch(gl.menuState) {
            case MAIN_MENU:
                showMainMenu(gl.xres, gl.tex.titleTexture, gl.yres);
                if (gl.credits) {
                    show_credits();
                }
                x11.swapBuffers();
                break;
            case GAME_RUNNING: {
                                   if (limitScoreboard==0) {
                                       newTime(g.gTime);
                                       limitScoreboard++;
                                   }
                                   printf("g.lives: %d\n", g.lives);
                                   paused = 0;
                                   physics();
                                   render();
                                   x11.swapBuffers();
                                   break;
                               }
            case GAME_OVER:
                               if (limitScoreboard) {
                                   newScoreboard();
                                   limitScoreboard--;
                               }
                               showGameOver(gl.xres, gl.gameOverTexture,gl.yres);
                               x11.swapBuffers();
                               break;
            case PAUSE_GAME:
                               if (!paused) {
                                   runningTime(g.gTime);
                                   //GameTime *lt = g.pauseTime;
                                   //clock_gettime(CLOCK_REALTIME, &lt->rTime);
                                   //timeCopy(&pt->rTime, &lt->rTime);
                                   printf("Copied TIME!\n");
                                   paused++;
                               }
                               //addPauseTime(&pt->rTime);
                               showPauseMenu();
                               x11.swapBuffers();
                               break;

        }
    }
    cleanup_fonts();
    logClose();
    return 0;
}

//--------------------------------From Rainforrest-----------------------------
unsigned char* buildAlphaData(Image* img)
{
    int i;
    int a,b,c;
    unsigned char *newdata, *ptr;
    unsigned char *data = (unsigned char*)img->data;
    newdata = (unsigned char*)malloc(img->width * img->height * 4);
    ptr = newdata;
    for (i = 0; i< img->width * img->height *3; i+=3) {
        a = *(data+0);
        b = *(data+1);
        c = *(data+2);
        *(ptr+0) = a;
        *(ptr+1) = b;
        *(ptr+2) = c;

        *(ptr+3) = (a|b|c);
        ptr += 4;
        data += 3;
    }
    return newdata;
}
//-----------------------------------------------------------------------------

void init_opengl()
{
    //
    //OpenGL initialization
    glViewport(0, 0, gl.xres, gl.yres);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    //This sets 2D mode (no perspective)
    glOrtho(0, gl.xres, 0, gl.yres, -1, 1);
    //
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FOG);
    glDisable(GL_CULL_FACE);
    //
    //Clear the screen to black
    glClearColor(0.0, 0.0, 0.0, 1.0);
    //Do this to allow fonts
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
    //--------------------------------From Rainforrest-----------------------------
    //background framework
    gl.tex.backImage = &img[5];
    gl.tex.titleImage = &img[6];
    //OpenGL initialization

    glGenTextures(1, &gl.tex.backTexture);
    glGenTextures(1, &g.eagleNone);
    glGenTextures(1, &g.enemyNone);
    glGenTextures(1, &gl.nobleTexture);
    glGenTextures(1, &gl.perfectTexture);
    glGenTextures(1, &gl.tigerTexture);
    glGenTextures(1, &gl.spongebobTexture);
    glGenTextures(1, &gl.tex.titleTexture);
    glGenTextures(1, &gl.gameOverTexture);
    glGenTextures(1, &gl.exp.tex);
    //glGenTextures(1, &g.eagleSprite);
    //glGenTextures(1, &g.backgroundTexture);

    //noble texture elements
    int w1, w2, w3, w4, w5, w6, w7, w8,w9,w10;
    int h1, h2, h3, h4, h5, h6, h7, h8,h9,h10;
    w1 = img[0].width;
    h1 = img[0].height;
    w2 = img[1].width;
    h2 = img[1].height;
    w3 = img[2].width;
    h3 = img[2].height;
    w4 = img[3].width;
    h4 = img[3].height;
    w5 = img[4].width;
    h5 = img[4].height;
    w6 = img[5].width;
    h6 = img[5].height;
    w7 = img[6].width;
    h7 = img[6].height;
    w8 = img[7].width;
    h8 = img[7].height;
    w9 = img[8].width;
    h9 = img[8].height;
    w10= img[9].width;
    h10= img[9].height;

    //------------------------------
    glBindTexture(GL_TEXTURE_2D, gl.perfectTexture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, w1, h1, 0,
            GL_RGB, GL_UNSIGNED_BYTE, img[0].data);

    //------------------------------
    glBindTexture(GL_TEXTURE_2D, gl.spongebobTexture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, w2, h2, 0,
            GL_RGB, GL_UNSIGNED_BYTE, img[1].data);

    //------------------------------
    glBindTexture(GL_TEXTURE_2D, gl.nobleTexture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, w3, h3, 0,
            GL_RGB, GL_UNSIGNED_BYTE, img[2].data);

    //------------------------------
    glBindTexture(GL_TEXTURE_2D, gl.tigerTexture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, w4, h4, 0,
            GL_RGB, GL_UNSIGNED_BYTE, img[3].data);

    //------------------------------NOT NEEDED
    /* glBindTexture(GL_TEXTURE_2D, g.eagleSprite);
       glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
       glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
       glTexImage2D(GL_TEXTURE_2D, 0, 3, w5, h5, 0,
       GL_RGB, GL_UNSIGNED_BYTE, img[4].data);
       */
    //------------------------------

    //background framework
    //gl.tex.backImage = &img[5];
    glGenTextures(1, &gl.tex.backTexture);
    //------------------------------
    glBindTexture(GL_TEXTURE_2D, g.eagleNone);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    unsigned char* eagleData = buildAlphaData(&img[4]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w5, h5, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, eagleData);
    free(eagleData);


    glBindTexture(GL_TEXTURE_2D, gl.tex.backTexture);
    unsigned char* backgroundData = buildAlphaData(&img[5]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w6, h6-50, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, backgroundData);
    gl.tex.xc[0] = 0.0;
    gl.tex.xc[1] = 0.25;
    gl.tex.yc[0] = 0.0;
    gl.tex.yc[1] = 1.0;    

    glBindTexture(GL_TEXTURE_2D, gl.tex.titleTexture);
    unsigned char* titleData = buildAlphaData(&img[6]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w7, h7-50, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, titleData); 

    //output Enemies
    glBindTexture(GL_TEXTURE_2D, g.enemyNone);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    unsigned char* enemyData = buildAlphaData(&img[7]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w8, h8, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, enemyData);
    free(enemyData); 
    // game over screen
    glBindTexture(GL_TEXTURE_2D, gl.gameOverTexture);
    unsigned char* gameOverData = buildAlphaData(&img[8]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w9, h9-50, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, gameOverData);

    //fireball
    glBindTexture(GL_TEXTURE_2D, gl.exp.tex);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    //must build a new set of data...
    unsigned char *xData = buildAlphaData(&img[9]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w10, h10, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, xData);
    free(xData);


}

void normalize2d(Vec v)
{
    Flt len = v[0]*v[0] + v[1]*v[1];
    if (len == 0.0f) {
        v[0] = 1.0;
        v[1] = 0.0;
        return;
    }
    len = 1.0f / sqrt(len);
    v[0] *= len;
    v[1] *= len;
}

void check_mouse(XEvent *e)
{
    //Was a mouse button clicked?
    static int savex = 0;
    static int savey = 0;
    //static int ct=0;
    if (e->type != ButtonPress &&
            e->type != ButtonRelease &&
            e->type != MotionNotify)
        return;
    if (e->type == ButtonRelease)
        return;
    if (e->type == ButtonPress) {
        if (e->xbutton.button==1) {
            //Left button is down
            //a little time between each bullet
            struct timespec bt;
            clock_gettime(CLOCK_REALTIME, &bt);
            double ts = timeDiff(&g.bulletTimer, &bt);
            if (ts > 0.1) {
                timeCopy(&g.bulletTimer, &bt);
                //shoot a bullet...
                if (g.nbullets < MAX_BULLETS) {
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
                    ++g.nbullets;
                }
            }
        }
        if (e->xbutton.button==3) {
            //Right button is down
        }
    }
    if (e->type == MotionNotify) {
        if (savex != e->xbutton.x || savey != e->xbutton.y) {
            //----------------------NOT NEEDED-----------------------
            /*
            //Mouse moved
            int xdiff = savex - e->xbutton.x;
            int ydiff = savey - e->xbutton.y;
            if (++ct < 10)
            return;		
            if (xdiff > 0) {
            //mouse moved along the x-axis.
            g.ship.angle += 0.05f * (float)xdiff;
            if (g.ship.angle >= 360.0f)
            g.ship.angle -= 360.0f;
            }
            else if (xdiff < 0) {
            g.ship.angle += 0.05f * (float)xdiff;
            if (g.ship.angle < 0.0f)
            g.ship.angle += 360.0f;
            }
            if (ydiff > 0) {
            //mouse moved along the y-axis.
            //apply thrust
            //convert ship angle to radians
            Flt rad = ((g.ship.angle+90.0) / 360.0f) * PI * 2.0;
            //convert angle to a vector
            Flt xdir = cos(rad);
            Flt ydir = sin(rad);
            g.ship.vel[0] += xdir * (float)ydiff * 0.001f;
            g.ship.vel[1] += ydir * (float)ydiff * 0.001f;
            Flt speed = sqrt(g.ship.vel[0]*g.ship.vel[0]+
            g.ship.vel[1]*g.ship.vel[1]);
            if (speed > 15.0f) {
            speed = 15.0f;
            normalize2d(g.ship.vel);
            g.ship.vel[0] *= speed;
            g.ship.vel[1] *= speed;
            }
            g.mouseThrustOn = true;
            clock_gettime(CLOCK_REALTIME, &g.mouseThrustTimer);
            }
            x11.set_mouse_position(100, 100);
            savex = savey = 100;*/
        }
    }
}

int check_keys(XEvent *e)
{
    //keyboard input?
    static int shift=0;
    if (e->type != KeyPress && e->type != KeyRelease)
        return 0;
    int key = (XLookupKeysym(&e->xkey, 0) & 0x0000ffff);
    //Log("key: %i\n", key);
    if (e->type == KeyRelease) {
        gl.keys[key]=0;
        if (key == XK_Shift_L || key == XK_Shift_R)
            shift=0;
        return 0;
    }
    gl.keys[key]=1;
    if (key == XK_Shift_L || key == XK_Shift_R) {
        shift=1;
        return 0;
    }
    (void)shift;
    switch (key) {
        case XK_Escape:
            return 1;
        case XK_c:
            if(gl.menuState == MAIN_MENU) {
                gl.credits ^= true;
            }
            break;
        case XK_f:
            gl.menuState = MAIN_MENU;
            break;
        case XK_s:
            gl.menuState = GAME_OVER;
            break;
        case XK_p:
            if (gl.menuState == GAME_RUNNING || gl.menuState == PAUSE_GAME) {
                if (gl.menuState == GAME_RUNNING) {
                    gl.menuState = PAUSE_GAME;
                } else {
                    gl.menuState = GAME_RUNNING;
                }
            } else { break; }
            break;
            //-------------DF--------------moving the eagle around-------
        case XK_Up:
            if (g.ship.pos[1] >= gl.yres - 15) {
                g.ship.pos[1] = 15;
            } else {
                g.ship.pos[1] += 10;
            }
            break;
        case XK_Down:
            //if (g.ship.pos[1] == 125) 
            //  g.ship.pos[1] = 725;
            if (g.ship.pos[1] <= 15) {
                g.ship.pos[1] = gl.yres - 15;        
            } else {
                g.ship.pos[1] -= 10;
            }
            break;
        case XK_Left:
            if (g.ship.pos[0] >= 20) {
                g.ship.pos[0] -= 10;
            }
            break;
        case XK_Right:
            if (g.ship.pos[0] <= gl.xres - 50) {
                g.ship.pos[0] += 10;
            }
            break;
        case XK_Return:
            if (gl.menuState == MAIN_MENU) 
                gl.menuState = GAME_RUNNING;
            break;
        case XK_d:
            bullet.rate ^= 1;
            break;
        case XK_equal:
            break;
        case XK_minus:
            break;
    }
    return 0;
}

void deleteAsteroid(Game *g, Asteroid *node)
{
    //Remove a node from doubly-linked list.
    //Must look at 4 special cases below.
    if (node->prev == NULL) {
        if (node->next == NULL) {
            //only 1 item in list.
            g->ahead = NULL;
        } else {
            //at beginning of list.
            node->next->prev = NULL;
            g->ahead = node->next;
        }
    } else {
        if (node->next == NULL) {
            //at end of list.
            node->prev->next = NULL;
        } else {
            //in middle of list.
            node->prev->next = node->next;
            node->next->prev = node->prev;
        }
    }
    delete node;
    node = NULL;
}

//------------------------ENEMEY Health???-----------------------
void buildAsteroidFragment(Asteroid *ta, Asteroid *a)
{
    //build ta from a
    ta->nverts = 8;
    ta->radius = a->radius / 2.0;
    Flt r2 = ta->radius / 2.0;
    Flt angle = 0.0f;
    Flt inc = (PI * 2.0) / (Flt)ta->nverts;
    for (int i=0; i<ta->nverts; i++) {
        ta->vert[i][0] = sin(angle) * (r2 + rnd() * ta->radius);
        ta->vert[i][1] = cos(angle) * (r2 + rnd() * ta->radius);
        angle += inc;
    }
    ta->pos[0] = a->pos[0] + rnd()*10.0-5.0;
    ta->pos[1] = a->pos[1] + rnd()*10.0-5.0;
    ta->pos[2] = 0.0f;
    ta->angle = 0.0;
    ta->rotate = a->rotate + (rnd() * 4.0 - 2.0);
    ta->color[0] = 0.8;
    ta->color[1] = 0.8;
    ta->color[2] = 0.7;
    ta->vel[0] = a->vel[0] + (rnd()*2.0-1.0);
    ta->vel[1] = a->vel[1] + (rnd()*2.0-1.0);
}

void physics()
{
    //-------------DF--------------From Background-------------------
    //move the background 
    gl.tex.xc[0] += 0.001;
    gl.tex.xc[1] += 0.001;

    Flt d0,d1,dist,bist,b0,b1;
    //Update ship position
    //g.ship.pos[0] += g.ship.vel[0];
    //g.ship.pos[1] += g.ship.vel[1];
    //Check for collision with window edges
    if (g.ship.pos[0] < 0.0) {
        g.ship.pos[0] += (float)gl.xres;
    }
    else if (g.ship.pos[0] > (float)gl.xres) {
        g.ship.pos[0] -= (float)gl.xres;
    }
    else if (g.ship.pos[1] < 0.0) {
        g.ship.pos[1] += (float)gl.yres;
    }
    else if (g.ship.pos[1] > (float)gl.yres) {
        g.ship.pos[1] -= (float)gl.yres;
    }
    //
    //Update bullet positions
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
        b->pos[0] += b->vel[0];
        b->pos[1] += b->vel[1];
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
    //
    //Update asteroid positions
    Asteroid *a = g.ahead;
    while (a) {
        a->pos[0] += a->vel[0];
        a->pos[1] += a->vel[1];
        //Check for collision with window edges
        if (a->pos[0] < -100.0) {
            a->pos[0] += (float)gl.xres+500;
        }
        /*if (a->pos[0] > (float)gl.xres+100) {
          a->pos[0] -= (float)gl.xres+200;
          }
          else if (a->pos[1] < -100.0) {
          a->pos[1] += (float)gl.yres+200;
          }*/
        else if (a->pos[1] > (float)gl.yres+100) {
            a->pos[1] -= (float)gl.yres+200;
        }
        a->angle = 0;
        a = a->next;
    }













    //
    //Asteroid collision with bullets?
    //If collision detected:
    //   1. delete the bullet
    //   2. break the asteroid into pieces
    //      if asteroid small, delete it
    a = g.ahead;
    while (a) {
        //If we hit the alien
        d0 = g.ship.pos[0] - a->pos[0]; 
        d1 = g.ship.pos[1] - a->pos[1];
        dist = (d0*d0 + d1*d1);
        if (dist < (a->radius*a->radius)) {
            //GAME Over
            if (g.lives <= 0) {
                gl.menuState = GAME_OVER;
                g.gameover = true;
            } else {
                printf("\n\n\nHIT\n\n\n");
                g.lives--;
            }
        }/*
            if (g.died) {
            Asteroid *savea = a->next;
            deleteAsteroid(&g, a);
            a = savea;
            g.nasteroids--;
            g.died=0;
            }*/

        //is there a bullet within its radius?
        int i=0;
        while (i < g.nbullets) {

            printf("\n\n\nBullet created\n\n\n");
            Bullet *b = &g.barr[i];
            b0 = b->pos[0] - a->pos[0]; 
            b1 = b->pos[1] - a->pos[1];
            bist = (b0*b0 + b1*b1);

            //GAME OVER
            //cout << "asteroid hit." << endl;
            //this asteroid is hit.
            if (bist < (a->radius*a->radius)) {
                //if (a->life != 0) {
                if (bullet.rate) {
                    a->life -=3;
                    g.died = 1;
                } else {
                    a->life--;
                }
                //} else {
                if (a->life == 0) {
                    Asteroid *savea = a->next;
                    deleteAsteroid(&g, a);
                    a = savea;
                    g.nasteroids--;
                }

                //delete the bullet...
                memcpy(&g.barr[i], &g.barr[g.nbullets-1], sizeof(Bullet));
                g.nbullets--;
                if (a == NULL)
                    break;
            }
            i++;
            }
            if (a == NULL)
                break;
            a = a->next;
        }
        //-------------o-------------------------------------
        //check keys pressed now
        if (gl.keys[XK_Left]) {
            //g.ship.angle = 90.0;
            /*if (g.ship.angle >= 360.0f)
              g.ship.angle -= 360.0f;*/
        }
        if (gl.keys[XK_Right]) {
            //g.ship.angle = 4.0;
            /*        if (g.ship.angle < 0.0f)
                      g.ship.angle += 360.0f;*/
        }
        if (gl.keys[XK_Up]) {
            //apply thrust
            //convert ship angle to radians
            Flt rad = ((g.ship.angle+90.0) / 360.0f) * PI * 2.0;
            //convert angle to a vector
            Flt xdir = cos(rad);
            Flt ydir = sin(rad);
            g.ship.vel[0] += xdir*0.02f;
            g.ship.vel[1] += ydir*0.02f;
            Flt speed = sqrt(g.ship.vel[0]*g.ship.vel[0]+
                    g.ship.vel[1]*g.ship.vel[1]);
            if (speed > 10.0f) {
                speed = 10.0f;
                normalize2d(g.ship.vel);
                g.ship.vel[0] *= speed;
                g.ship.vel[1] *= speed;
            }
        }
        if (gl.keys[XK_space]) {
            //a little time between each bullet
            if (bullet.rate == 1) {
                extern int deagle_time(Game&, int);
                g.lives = deagle_time(g, g.lives);
            } else {
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
                        b->color[0] = 0.0f;
                        b->color[1] = 0.0f;
                        b->color[2] = 0.0f;
                        g.nbullets++;
                    }
                }
            }
        }
        if (g.mouseThrustOn) {
            //should thrust be turned off
            struct timespec mtt;
            clock_gettime(CLOCK_REALTIME, &mtt);
            double tdif = timeDiff(&mtt, &g.mouseThrustTimer);
            if (tdif < -0.3)
                g.mouseThrustOn = false;
        }
    }
    //Juan Orozco???

    extern int getScore();
    extern int timeTotal(struct timespec*);
    extern void scoreAccumulator(int, int, struct timespec*);
    void render()
    {
        extern void showBackground(int ,GLuint);
        extern void showEagle(float, float, GLuint);

        //extern void showEnemy();
        scoreAccumulator(1, 0, &g.gTime);
        Rect r;
        glClear(GL_COLOR_BUFFER_BIT);
        //
        //--------Group
        /*if (gl.credits) {
          show_credits(); 
          return;
          }*/
        r.bot = gl.yres - 20;
        r.left = 10;
        r.center = 0;

        //----------------Juan
        //showBackground();
        showBackground(gl.xres,gl.tex.backTexture);

        //---------------------------DF
        glClear(GL_COLOR_BUFFER_BIT);
        glColor3f(1.0, 1.0, 1.0);
        glPushMatrix();
        glBindTexture(GL_TEXTURE_2D, gl.tex.backTexture);
        glBegin(GL_QUADS);
        glTexCoord2f(gl.tex.xc[0], gl.tex.yc[1]); glVertex2i(0, 0);
        glTexCoord2f(gl.tex.xc[0], gl.tex.yc[0]); glVertex2i(0, gl.yres);
        glTexCoord2f(gl.tex.xc[1], gl.tex.yc[0]); glVertex2i(gl.xres, gl.yres);
        glTexCoord2f(gl.tex.xc[1], gl.tex.yc[1]); glVertex2i(gl.xres, 0);
        glEnd();
        glPopMatrix();

        //--------------------Juan
        //showEagle();
        showEagle(g.ship.pos[0], g.ship.pos[1], g.eagleNone);

        ggprint8b(&r, 16, 0x00ff0000, "3350 - Asteroids");
        ggprint8b(&r, 16, 0x00ffff00, "n bullets: %i", g.nbullets);
        ggprint8b(&r, 16, 0x00ffff00, "n asteroids: %i", g.nasteroids);
        ggprint8b(&r, 16, 0x00ffff00, "n asteroids destroyed: ");
        ggprint8b(&r, 16, 0x00ffff00, "D for Desert Eagle,");
        //---------------------------Josh
		ggprint8b(&r, 16, 0x00ffff00, "Score: %d", getScore());
		//-------------

		if(g.lives <= 0)
		{
			float cx = gl.xres/2.0;
			float cy = gl.yres/2.0;

			float h = 80.0;
			float w = 80.0;
			/////////////////////////////////////////////////////////////////////////////////

			timers.recordTime(&gl.exp.time);
			gl.exp.onoff ^= 1;
			gl.exp.pos[0] = g.ship.pos[0];
			gl.exp.pos[1] = g.ship.pos[1];
			//gl.exp.pos[2] = 0;
			glPushMatrix();
			glColor3f(1.0, 1.0, 1.0);
			glBindTexture(GL_TEXTURE_2D, gl.exp.tex);
			glEnable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GREATER, 0.0f);
			glColor4ub(255,255,255,255);
			//glTranslated(gl.exp.pos[0], gl.exp.pos[1], gl.exp.pos[2]);
			glTranslatef(g.ship.pos[0], g.ship.pos[1], gl.exp.pos[2]);

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

		//Draw the ship
		/*
		   glColor3fv(g.ship.color);
		   glPushMatrix();
		   glTranslatef(g.ship.pos[0], g.ship.pos[1], g.ship.pos[2]);
		   glRotatef(g.ship.angle, 0.0f, 0.0f, 1.0f);
		   glBegin(GL_TRIANGLES);
		   glVertex2f(-12.0f, -10.0f);
		   glVertex2f(  0.0f, 20.0f);
		   glVertex2f(  0.0f, -6.0f);
		   glVertex2f(  0.0f, -6.0f);
		   glVertex2f(  0.0f, 20.0f);
		   glVertex2f( 12.0f, -10.0f);
		   glEnd();
		   glColor3f(1.0f, 0.0f, 0.0f);
		   glBegin(GL_POINTS);
		   glVertex2f(0.0f, 0.0f);
		   glEnd();
		   glPopMatrix();
		   if (gl.keys[XK_Up] || g.mouseThrustOn) {
		   int i;
		//draw thrust
		Flt rad = ((g.ship.angle+90.0) / 360.0f) * PI * 2.0;
		//convert angle to a vector
		Flt xdir = cos(rad);
		Flt ydir = sin(rad);
		Flt xs,ys,xe,ye,r;
		glBegin(GL_LINES);
		for (i=0; i<16; i++) {
		xs = -xdir * 11.0f + rnd() * 4.0 - 2.0;
		ys = -ydir * 11.0f + rnd() * 4.0 - 2.0;
		r = rnd()*40.0+40.0;
		xe = -xdir * r + rnd() * 18.0 - 9.0;
		ye = -ydir * r + rnd() * 18.0 - 9.0;
		glColor3f(rnd()*.3+.7, rnd()*.3+.7, 0);
		glVertex2f(g.ship.pos[0]+xs,g.ship.pos[1]+ys);
		glVertex2f(g.ship.pos[0]+xe,g.ship.pos[1]+ye);
		}
		glEnd();
		}
		*/
		//----------------
		//Draw the bullets

		Bullet *b = &g.barr[0];
		for (int i=0; i<g.nbullets; i++) {
			//Log("draw bullet...\n");
			glColor3f(0.0, 0.0, 0.0);
			glBegin(GL_POINTS);
			glVertex2f(b->pos[0],      b->pos[1]);
			glVertex2f(b->pos[0]-1.0f, b->pos[1]);
			glVertex2f(b->pos[0]+1.0f, b->pos[1]);
			glVertex2f(b->pos[0],      b->pos[1]-1.0f);
			glVertex2f(b->pos[0],      b->pos[1]+1.0f);
			glColor3f(0.8, 0.8, 0.8);
			glVertex2f(b->pos[0]-1.0f, b->pos[1]-1.0f);
			glVertex2f(b->pos[0]-1.0f, b->pos[1]+1.0f);
			glVertex2f(b->pos[0]+1.0f, b->pos[1]-1.0f);
			glVertex2f(b->pos[0]+1.0f, b->pos[1]+1.0f);
			glEnd();
			++b;
		}
		//------------------
		//Draw the asteroids
		Asteroid *a = g.ahead;
		while (a) {
			//showEnemy();

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
			int wid = 80;
			glPushMatrix();
			glTranslatef(a->pos[0] , a->pos[1] , a->pos[2]);
			glRotatef(a->angle, 0.0f, 0.0f, 1.0f);
			glBindTexture(GL_TEXTURE_2D, g.enemyNone);
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
			//
			//
			glPushMatrix();
			glColor3f(1.0f, 0.0f, 0.0f);
			glBegin(GL_POINTS);
			glVertex2f(a->pos[0], a->pos[1]);
			glEnd();
			glPopMatrix();
			a = a->next;
		}
	}


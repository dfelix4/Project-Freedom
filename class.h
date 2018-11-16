#ifndef _class
#define _class
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
Image img[6] = {
    "./images/notperfect.jpg",
    "./images/spongebob.jpg",
    "./images/Noblelogo.jpg",
    "./images/tiger.jpg",
    "./images/eagle.jpg",
    "./images/city.jpg"
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
        int speed;
        int durability;
    public:
        Bullet() { 
            rate = speed = durability = 0;
        }
}bullet;

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
#endif

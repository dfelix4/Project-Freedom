//Author: Joshua Rodriguez
//Date: 9-26-18
// To do list:
//      DONE-ish Setup in game score based on time survived + enemies killed
//                          - Use existing time from asteroids
//      DONE: Setup File I/O to save and read high scores to screen
//                          - Display 10 scores
//                          - If new score > old score, delete 10th score
//                          - Sorting algorithm
//      Pause Menu: Working on it!
//      Setup sound to be able to play sounds for hit, music, shoot and powerups
//
#include <ctgmath>
#include <iostream> //For bug testing purposes
#include <cmath> //Math functions
#include <ctime> //For time calculations
#include <unistd.h>
#include "fonts.h"
#include <GL/glx.h>
#include <fstream>
#include <X11/keysym.h>
#include <X11/Xlib.h>
//#include <log.h>
#include <GL/glx.h>

class Gametime {
    public:
        Gametime *nt;
        struct timespec newT;
        Gametime() {
            clock_gettime(CLOCK_REALTIME, &newT);
        }
};
using namespace std;
extern void timeCopy(struct timespec *dest, struct timespec *source);
extern double timeDiff(struct timespec *start, struct timespec *end);
int score = 0;
int scoreboard[11];
int passedTime = 0;
int oldTime = 0;
ifstream fin;
ofstream fout;
//show credits function
void creditJosh(int x, int y) 
{
    Rect r;
    r.bot = y;
    r.left = x;
    r.center = 0;
    ggprint16(&r, 16, 0x00ffff00, "\nThe Best Around - Joshua Rodriguez\n");
}
//Show credit picture and animate it
void showJoshPicture(int x, int y, GLuint texid) 
{
    static float direction = 0.0f;
    float fx = 0;
    direction +=0.1f;
    fx += sin(direction)*400.0f;
    glColor3ub(255,255,255);
    int wid = 40;

    glPushMatrix();
    glTranslatef(x+fx,y,0);
    glBindTexture(GL_TEXTURE_2D, texid);
    glColor3ub(255,255,255);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex2i(-wid,-wid);
    glTexCoord2f(0.0f, 0.0f); glVertex2i(-wid, wid);
    glTexCoord2f(1.0f, 0.0f); glVertex2i( wid, wid);
    glTexCoord2f(1.0f, 1.0f); glVertex2i( wid,-wid);
    glEnd();
    glPopMatrix();

    return;
}
//Function to calculate current time to an int
//Add timespec struct in global class and use
//clock_gettime(CLOCK_REALTIME, &'structName')
//to pass in for g_time
void newTime(struct timespec oldtime) {
    Gametime *t = new Gametime;
    timeCopy(&oldtime, &t->newT);
}
void runningTime(struct timespec oldtime) {
    Gametime *t = new Gametime;
    oldTime = timeDiff(&oldtime, &t->newT);
    timeCopy(&oldtime, &t->newT);
}
int timeTotal(struct timespec* gTime) 
{  
    struct timespec rTime;
    clock_gettime(CLOCK_REALTIME, &rTime);
    double time = timeDiff(gTime, &rTime);
    int timeRound = round(time);
    return(timeRound);
}
//Add score over time + enemies defeated + any multipliers
void scoreAccumulator(int multiplier, int kills, struct timespec* global) 
{   
    int scoreOvertime;
    scoreOvertime = timeTotal(global) + kills - oldTime;
    score = scoreOvertime*multiplier;
    cout << "Pause: " << passedTime << " time total: " << timeTotal(global) << endl;
    //oldTime = 0;
    return;
}
//Getter function for score
int getScore()
{                  
    return(score);
}
void addPauseTime(struct timespec* pause) {
    struct timespec pTime;
    clock_gettime(CLOCK_REALTIME, &pTime);
    passedTime = (int)timeDiff(pause, &pTime) +1;
}
//Display scoreboard to screen
void displayScoreboard(int x, int y, int o)
{
    Rect s;
    s.bot = y;
    s.left = x;
    s.center = o;
    fin.open("scoreBoard.txt");
    if(fin.fail() ) {
        cout << "Error: Cannot open/find scoreboard.txt for reading.\n";
    }
    for(int i = 0; i<10; i++) {
        fin >> scoreboard[i];
        ggprint16(&s, 16, 0x00ffff00, "\n%d. %d", i+1, scoreboard[i]);
    }
    fin.close();
}
//Bubble sort scoreboard
void sortScoreboard() 
{
    int temp;
    int first;
    int second;
    for (int i =9; i>=0; i--) {
        first = scoreboard[i];
        second = scoreboard[i+1];
        if (first < second) {
            temp = first;
            first = second;
            second = temp;
            scoreboard[i] = first;
            scoreboard[i+1] = second;
        }
    } 
}
//Update scoreboard with new value and file I/O to txt file
void newScoreboard() 
{
    fin.open("scoreBoard.txt");
    for (int i = 0; i<10; i++) {
        fin >> scoreboard[i];
    }
    fin.close();

    cout << "Making scoreboard\n";
    scoreboard[10] = getScore();
    sortScoreboard();
    if (fin.fail() ) {
        cout << "Error: Cannot read to file" << endl;
    }
    fout.open("scoreBoard.txt");
    if (fout.fail() ) {
        cout << "Error: cannot write to file\n";
    }
    for (int i=0; i<11; i++) {
        fout << scoreboard[i] << endl;
    }
    fout.close();
}
void showMainMenu(int x, GLuint mainScreen, int y) {   
    
    glColor3ub(255,255,255);
    glClear(GL_COLOR_BUFFER_BIT);
    //int back = x - 200;
    glPushMatrix();
    glTranslatef(255, 255, 0);
    glBindTexture(GL_TEXTURE_2D, mainScreen);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex2i(0,0);
    glTexCoord2f(0.0f, 0.0f); glVertex2i(0, y);
    glTexCoord2f(1.0f, 0.0f); glVertex2i(x, y);
    glTexCoord2f(1.0f, 1.0f); glVertex2i(x, 0);

    glEnd();
    glPopMatrix();

    Rect m;
    m.bot = 200;
    m.left = 200;
    m.center = 0;
    ggprint8b(&m, 16, 0x00ff0000, "THIS IS MAIN MENU PRESS ENTER TO START, C to get into credits");
}
void showGameOver(int x, GLuint screen) {
    glColor3ub(255,255,255);
    glClear(GL_COLOR_BUFFER_BIT);

    int back = x -200 ;
    glPushMatrix();
    glTranslatef(255, 255, 0);
    glBindTexture(GL_TEXTURE_2D, screen);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex2i(-back,-back);
    glTexCoord2f(0.0f, 0.0f); glVertex2i(-back, back);
    glTexCoord2f(1.0f, 0.0f); glVertex2i( back, back);
    glTexCoord2f(1.0f, 1.0f); glVertex2i( back,-back);

    glEnd();
    glPopMatrix();
    Rect m;
    glPushMatrix();
    glBegin(GL_QUADS);
    glColor3ub(255, 0, 0);
    m.bot = 600;
    m.left = 200;
    m.center = 0;
    //glTexCoord2f(0.0f, 1.0f); 
    glVertex2i(200,-700);
    //glTexCoord2f(0.0f, 0.0f); 
    glVertex2i(200, 700);
    //glTexCoord2f(1.0f, 0.0f); 
    glVertex2i( 400, 700);
    //glTexCoord2f(1.0f, 1.0f); 
    glVertex2i( 400,-400);
    glEnd();
    glPopMatrix();
    ggprint8b(&m, 16, 0x00ff0000, "THIS IS Game Over screen");
    displayScoreboard(m.left, m.bot, m.center);
}
void showPauseMenu() {
    //To be done...
}

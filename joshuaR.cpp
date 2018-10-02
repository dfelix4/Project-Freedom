//Author: Joshua Rodriguez
//Date: 9-26-18
// To do list:
//      DONE-ish: Setup in game score based on time survived + enemies killed
//                          - Use existing time from asteroids
//      Setup File I/O to save and read high scores to screen
//                          - Display 10 scores
//                          - If new score > old score, delete 10th score
//                          - Sorting algorithm
//      Setup sound to be able to play sounds for hit, music, shoot and powerups
//
#include <ctgmath>
#include <iostream>
#include <cmath>
#include <ctime>
#include <unistd.h>
#include <iostream>
#include "fonts.h"
#include <GL/glx.h>
using namespace std;
extern double timeDiff(struct timespec *start, struct timespec *end);
int score = 0;
//Show Credits function - finished
void creditJosh(int x, int y) 
{
    Rect r;
    r.bot = y;
    r.left = x;
    r.center = 0;
    ggprint16(&r, 16, 0x00ffff00, "\nThe Best Around - Joshua Rodriguez\n");
}
void showJoshPicture(int x, int y, GLuint texid) {
	//show Noble team logo
    	int wid = 50;
	glPushMatrix();
	glTranslatef(x,y,0);
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
//Setup score based on timer - Unfinished
int timeTotal(struct timespec* gTime) 
{                       //Function to calculate current time to an int
                        //Add timespec struct in global class and use
                        //clock_gettime(CLOCK_REALTIME, &'structName')
                        //to pass in for g_time
    struct timespec rTime;
    clock_gettime(CLOCK_REALTIME, &rTime);
    double time = timeDiff(gTime, &rTime);
    int timeRound = round(time);
    return(timeRound);
}
void scoreAccumulator(int multiplier, int kills, struct timespec* global) 
{                       //Function to auto add score over time
    int scoreOvertime;
    scoreOvertime = timeTotal(global) + kills;
    score = scoreOvertime*multiplier;
    return;
}               
int getScore()
                        //Call anytime you need score 
{               
    return(score);
}

    

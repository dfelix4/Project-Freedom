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

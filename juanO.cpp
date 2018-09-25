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
     ggprint16b(&r, 16, 0x00ff0000, "Credits: \n Juan Orozco\n ");

}

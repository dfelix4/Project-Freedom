//Author: Joshua Rodriguez
//Date: 9-26-18


#include <iostream>
#include "fonts.h"
using namespace std;
void creditJosh(int x, int y) 
{
    Rect r;
    r.bot = y;
    r. left = x;
    r.center = 0;

    ggprint16(&r, 16, 0x000000ff, "\nThe Best Around - Joshua Rodriguez\n");
}
    

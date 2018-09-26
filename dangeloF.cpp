//Author: D'Angelo Felix
//Date: 9/26/18

#include "fonts.h"
#include<iostream>
using namespace std;
void creditsD(int x, int y)
{
    Rect r;
    r.bot = y;
    r.left = x;
    r.center = 0;
    ggprint16(&r, 16, 0x00ff0000, "D'Angelo Felix");
}

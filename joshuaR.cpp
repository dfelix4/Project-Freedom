#include <iostream>
#include "fonts.h"
using namespace std;
void creditJosh(int x, int y) {
    Rect r;
    r.bot = y;
    r. left = x;
    r.center = 0;

    ggprint16(&r, 16, 0x000000ff, "\n The Best Around - Joshua Rodriguez\n");
}
    

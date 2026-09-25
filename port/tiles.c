/* Map drawing: glyphs only until tiles (RVIP stage 4). */
#include "hack.h"
#include "vt.h"

int vt_cooked(void) { return !flags.cbreak; }

void vt_map(chtype s[][80], int box[4], void (*cell)(int y, int x, int tile, int und, int ch))
{
    for (int y = MAP0; y <= MAP1; y++)
        for (int x = 0; x < 80; x++) cell(y, x, -1, -1, s[y][x] & A_CHARTEXT);
    box[1] = -1;
}

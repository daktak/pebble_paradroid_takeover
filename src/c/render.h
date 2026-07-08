#ifndef RENDER_H
#define RENDER_H
#include "takeover.h"

void draw_board(GContext *ctx, GameState *gs);
void draw_hud(GContext *ctx, GameState *gs);
void draw_phase_screen(GContext *ctx, GameState *gs);
void draw_high_scores(GContext *ctx, HighScoreData *hsd);
void load_droid_bitmap(int idx);
void unload_droid_bitmap(void);

#endif

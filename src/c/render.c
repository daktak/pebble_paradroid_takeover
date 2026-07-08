#include "render.h"

static GBitmap *s_droid_bitmap;

static const uint32_t DROID_IDS[NUM_DROID_TYPES] = {
  RESOURCE_ID_DROID_001, RESOURCE_ID_DROID_123, RESOURCE_ID_DROID_139,
  RESOURCE_ID_DROID_247, RESOURCE_ID_DROID_249, RESOURCE_ID_DROID_296,
  RESOURCE_ID_DROID_302, RESOURCE_ID_DROID_329, RESOURCE_ID_DROID_420,
  RESOURCE_ID_DROID_476, RESOURCE_ID_DROID_493, RESOURCE_ID_DROID_516,
  RESOURCE_ID_DROID_571, RESOURCE_ID_DROID_598, RESOURCE_ID_DROID_614,
  RESOURCE_ID_DROID_615, RESOURCE_ID_DROID_629, RESOURCE_ID_DROID_711,
  RESOURCE_ID_DROID_742, RESOURCE_ID_DROID_751, RESOURCE_ID_DROID_821,
  RESOURCE_ID_DROID_834, RESOURCE_ID_DROID_883, RESOURCE_ID_DROID_999,
};

void load_droid_bitmap(int idx) {
  if (s_droid_bitmap) gbitmap_destroy(s_droid_bitmap);
  s_droid_bitmap = gbitmap_create_with_resource(DROID_IDS[idx]);
}

void unload_droid_bitmap(void) {
  if (s_droid_bitmap) {
    gbitmap_destroy(s_droid_bitmap);
    s_droid_bitmap = NULL;
  }
}

static GColor phase_color(GColor base, int phase) {
  if (phase == INACTIVE) return GColorDarkGray;
  return base;
}

static void draw_tile(GContext *ctx, int x, int y, int elem, int color, int phase, int selected) {
  GColor col = color == GELB ? GColorYellow : GColorVividViolet;
  col = phase_color(col, phase);
  graphics_context_set_stroke_color(ctx, col);
  graphics_context_set_fill_color(ctx, col);
  graphics_context_set_stroke_width(ctx, phase >= ACTIVE1 ? 2 : 1);

  int mx = x + CELL_W / 2;
  int my = y + CELL_H / 2;

  switch (elem) {
    case KABEL:
      graphics_fill_rect(ctx, GRect(x + 1, my - 1, CELL_W - 2, 3), 0, GCornerNone);
      break;
    case KABELENDE: {
      GColor c = selected
        ? (color == GELB ? GColorYellow : GColorVividViolet)
        : (phase == INACTIVE ? GColorDarkGray
          : (color == GELB ? GColorYellow : GColorVividViolet));
      graphics_context_set_fill_color(ctx, c);
      graphics_context_set_stroke_width(ctx, 1);
      if (x < GRID2_X)
        graphics_fill_rect(ctx, GRect(x + 1, my - 1, CELL_W / 2 - 1, 3), 0, GCornerNone);
      else
        graphics_fill_rect(ctx, GRect(mx + 1, my - 1, CELL_W / 2 - 1, 3), 0, GCornerNone);
      graphics_fill_rect(ctx, GRect(mx - 3, my - 3, 6, 6), 0, GCornerNone);
      break;
    }
    case VERSTAERKER: {
      GColor c = color == GELB ? GColorYellow : GColorVividViolet;
      if (x < GRID2_X) {
        graphics_fill_rect(ctx, GRect(x + 1, my - 1, CELL_W - 8, 3), 0, GCornerNone);
        graphics_context_set_fill_color(ctx, c);
        graphics_fill_rect(ctx, GRect(x + CELL_W - 7, my - 4, 6, 8), 0, GCornerNone);
      } else {
        graphics_fill_rect(ctx, GRect(x + 8, my - 1, CELL_W - 8, 3), 0, GCornerNone);
        graphics_context_set_fill_color(ctx, c);
        graphics_fill_rect(ctx, GRect(x + 1, my - 4, 6, 8), 0, GCornerNone);
      }
      break;
    }
    case FARBTAUSCHER: {
      int right = x < GRID2_X;
      graphics_fill_rect(ctx, GRect(right ? x + 1 : x + 8, my - 1, CELL_W - 8, 3), 0, GCornerNone);
      GColor swap = color == GELB ? GColorVividViolet : GColorYellow;
      graphics_context_set_fill_color(ctx, swap);
      graphics_fill_rect(ctx, GRect(right ? x + CELL_W - 7 : x + 1, my - 4, 6, 8), 0, GCornerNone);
      break;
    }
    case VERZWEIGUNG_U:
      graphics_fill_rect(ctx, GRect(x + 1, my - 1, CELL_W - 2, 3), 0, GCornerNone);
      graphics_fill_rect(ctx, GRect(mx - 1, y + 1, 3, my - y - 2), 0, GCornerNone);
      break;
    case VERZWEIGUNG_M:
      graphics_fill_rect(ctx, GRect(mx - 1, y + 1, 3, CELL_H - 1), 0, GCornerNone);
      graphics_fill_rect(ctx, GRect(x + 1, my - 1, CELL_W - 2, 3), 0, GCornerNone);
      break;
    case VERZWEIGUNG_O:
      graphics_fill_rect(ctx, GRect(x + 1, my - 1, CELL_W - 2, 3), 0, GCornerNone);
      graphics_fill_rect(ctx, GRect(mx - 1, my + 2, 3, CELL_H / 2 - 2), 0, GCornerNone);
      break;
    case GATTER_O:
      graphics_fill_rect(ctx, GRect(x + 1, my, CELL_W - 8, 3), 0, GCornerNone);
      graphics_draw_line(ctx, GPoint(x + CELL_W - 7, my), GPoint(x + CELL_W - 2, y + 2));
      break;
    case GATTER_M:
      graphics_fill_rect(ctx, GRect(x + 1, my - 1, CELL_W - 2, 3), 0, GCornerNone);
      break;
    case GATTER_U:
      graphics_fill_rect(ctx, GRect(x + 1, my - 2, CELL_W - 8, 3), 0, GCornerNone);
      graphics_draw_line(ctx, GPoint(x + CELL_W - 7, my), GPoint(x + CELL_W - 2, y + CELL_H - 2));
      break;
    case LEER:
      break;
  }
}

static void draw_cell(GContext *ctx, int x, int y, int elem, int color, int phase, int selected) {
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(x, y, CELL_W, CELL_H), 0, GCornerNone);
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_line(ctx, GPoint(x + CELL_W - 1, y), GPoint(x + CELL_W - 1, y + CELL_H - 1));
  draw_tile(ctx, x, y, elem, color, phase, selected);
}

static void draw_led_col(GContext *ctx, int x, int y, int display_column[], int leader) {
  int led_h = (CELL_H - 4) / 2;
  int led_gap = (CELL_H - led_h * 2);
  for (int r = 0; r < NUM_LINES; r++) {
    int ly = y + r * CELL_H + led_gap / 2;
    GColor col;
    if (display_column[r] == GELB) col = GColorYellow;
    else if (display_column[r] == VIOLETT) col = GColorVividViolet;
    else col = GColorDarkGray;
    graphics_context_set_fill_color(ctx, col);
    graphics_fill_rect(ctx, GRect(x, ly, COL_W, led_h), 2, GCornersAll);
  }
}

static void draw_grid(GContext *ctx, GameState *gs, int color) {
  int is_yellow = (color == GELB);
  for (int r = 0; r < NUM_LINES; r++) {
    for (int l = 0; l < NUM_LAYERS; l++) {
      int x = is_yellow ? GRID_X + l * CELL_W : GRID2_X + (NUM_LAYERS - 1 - l) * CELL_W;
      int y = GRID_Y + r * CELL_H;
      int elem = gs->board[color][l][r];
      int phase = gs->activation[color][l][r];
      int col = color;
      if (elem == KABELENDE && phase == INACTIVE
          && gs->board[color][2][r] == FARBTAUSCHER)
        col = is_yellow ? VIOLETT : GELB;
      if (elem == KABEL && l == 3
          && gs->board[color][2][r] == FARBTAUSCHER
          && gs->activation[color][2][r] >= ACTIVE1)
        col = is_yellow ? VIOLETT : GELB;
      int sel = (gs->your_color == color) && (gs->capsule_countdown[color][r] > 0);
      draw_cell(ctx, x, y, elem, col, phase, sel);
    }
  }
}

void draw_board(GContext *ctx, GameState *gs) {
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, 0, SCREEN_W, SCREEN_H), 0, GCornerNone);

  draw_grid(ctx, gs, GELB);
  draw_led_col(ctx, COL_X, GRID_Y, gs->display_column, gs->leader_color);
  draw_grid(ctx, gs, VIOLETT);
}

void draw_hud(GContext *ctx, GameState *gs) {
  char buf[32];

  snprintf(buf, sizeof(buf), "%s", droid_name_str(gs->player.num));
  graphics_context_set_text_color(ctx, GColorYellow);
  graphics_draw_text(ctx, buf, fonts_get_system_font(FONT_KEY_GOTHIC_14),
      GRect(2, 2, 60, 16), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);

  if (gs->phase == PHASE_PLAYING) {
    snprintf(buf, sizeof(buf), "%d", gs->player.caps);
    graphics_draw_text(ctx, buf, fonts_get_system_font(FONT_KEY_GOTHIC_14),
        GRect(2, 16, 60, 14), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  }

  snprintf(buf, sizeof(buf), "%s", droid_name_str(gs->enemy.num));
  graphics_context_set_text_color(ctx, GColorVividViolet);
  graphics_draw_text(ctx, buf, fonts_get_system_font(FONT_KEY_GOTHIC_14),
      GRect(SCREEN_W - 62, 2, 60, 16), GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);

  if (gs->phase == PHASE_PLAYING) {
    snprintf(buf, sizeof(buf), "%d", gs->enemy.caps);
    graphics_draw_text(ctx, buf, fonts_get_system_font(FONT_KEY_GOTHIC_14),
        GRect(SCREEN_W - 62, 16, 60, 14), GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);
  }

  GColor timer_col;
  switch (gs->leader_color) {
    case GELB: timer_col = GColorYellow; break;
    case VIOLETT: timer_col = GColorVividViolet; break;
    default: timer_col = GColorWhite; break;
  }
  graphics_context_set_text_color(ctx, timer_col);
  if (gs->phase == PHASE_COLOR_SEL) {
    int left = COLOR_COUNTDOWN - gs->countdown;
    snprintf(buf, sizeof(buf), "Color %d", left);
    graphics_draw_text(ctx, buf, fonts_get_system_font(FONT_KEY_GOTHIC_14),
        GRect((SCREEN_W - 80) / 2, 8, 80, 16), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  } else if (gs->phase == PHASE_PLAYING) {
    int left = GAME_COUNTDOWN - gs->countdown;
    snprintf(buf, sizeof(buf), "%d", left);
    graphics_draw_text(ctx, buf, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
        GRect((SCREEN_W - 80) / 2, 4, 80, 20), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  }

  if (gs->phase == PHASE_PLAYING || gs->phase == PHASE_COLOR_SEL) {
    int max_count = (gs->phase == PHASE_COLOR_SEL) ? COLOR_COUNTDOWN : GAME_COUNTDOWN;
    int left = max_count - gs->countdown;
    int bar_w = (SCREEN_W - 20) * left / max_count;
    GColor bar_col;
    switch (gs->leader_color) {
      case GELB: bar_col = GColorYellow; break;
      case VIOLETT: bar_col = GColorVividViolet; break;
      default: bar_col = (gs->tick & 1) ? GColorYellow : GColorVividViolet; break;
    }
    graphics_context_set_fill_color(ctx, bar_col);
    graphics_fill_rect(ctx, GRect(10, BOT_BAR_Y + 4, bar_w, 6), 2, GCornersAll);
    graphics_context_set_stroke_color(ctx, GColorDarkGray);
    graphics_context_set_stroke_width(ctx, 1);
    graphics_draw_rect(ctx, GRect(10, BOT_BAR_Y + 4, SCREEN_W - 20, 6));
  }

  if (gs->phase == PHASE_RESULT) {
    int mid_x = SCREEN_W / 2;
    int mid_y = WIN_MSG_MID_Y;
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(mid_x - 50, mid_y - 16, 100, 32), 4, GCornersAll);
    const char *msg = gs->won == 2 ? "DEADLOCK"
                    : gs->won      ? "CAPTURED"
                    :                "REJECTED";
    graphics_context_set_text_color(ctx, gs->won == 2 ? GColorWhite
                                       : gs->won      ? GColorYellow
                                       :                GColorRed);
    snprintf(buf, sizeof(buf), "%s", msg);
    graphics_draw_text(ctx, buf, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
        GRect(mid_x - 46, mid_y - 10, 92, 20), GTextOverflowModeTrailingEllipsis,
        GTextAlignmentCenter, NULL);
  }
}

static void draw_droid_show(GContext *ctx, const char *label, int droid_num, GColor col) {
  char buf[16];
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, 0, SCREEN_W, SCREEN_H), 0, GCornerNone);
  if (s_droid_bitmap) {
    graphics_draw_bitmap_in_rect(ctx, s_droid_bitmap, GRect(DROID_X, DROID_Y, DROID_W, DROID_H));
  }
  snprintf(buf, sizeof(buf), "%s: %s", label, droid_name_str(droid_num));
  graphics_context_set_text_color(ctx, col);
  graphics_draw_text(ctx, buf, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
      GRect(0, 10, SCREEN_W, 24), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(ctx, "Press SELECT", fonts_get_system_font(FONT_KEY_GOTHIC_14),
      GRect(0, PRESS_SEL_Y, SCREEN_W, 20), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

void draw_phase_screen(GContext *ctx, GameState *gs) {
  if (gs->phase == PHASE_SHOW_PLAYER) {
    draw_droid_show(ctx, "You", gs->player.num, GColorYellow);
  } else if (gs->phase == PHASE_SHOW_ENEMY) {
    draw_droid_show(ctx, "Enemy", gs->enemy.num, GColorVividViolet);
  } else if (gs->phase == PHASE_COLOR_SEL) {
    draw_board(ctx, gs);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(COLOR_SEL_OX, COLOR_SEL_OY, COLOR_SEL_W, COLOR_SEL_H), 4, GCornersAll);
    char buf[32];
    snprintf(buf, sizeof(buf), "Choose color");
    graphics_context_set_text_color(ctx, GColorWhite);
    graphics_draw_text(ctx, buf, fonts_get_system_font(FONT_KEY_GOTHIC_14),
        GRect(COLOR_SEL_TX, COLOR_SEL_TY, COLOR_SEL_TW, 16), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    snprintf(buf, sizeof(buf), "%s",
        gs->your_color == GELB ? "YELLOW" : "VIOLET");
    graphics_context_set_text_color(ctx,
        gs->your_color == GELB ? GColorYellow : GColorVividViolet);
    graphics_draw_text(ctx, buf, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
        GRect(COLOR_SEL_TX, COLOR_SEL_TY + 20, COLOR_SEL_TW, 24), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  }
}

void draw_high_scores(GContext *ctx, HighScoreData *hsd) {
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, 0, SCREEN_W, SCREEN_H), 0, GCornerNone);

  char buf[32], date_buf[16];

  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(ctx, "HIGH SCORES", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
      GRect(0, HS_TITLE_Y, SCREEN_W, 22), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);

  graphics_context_set_text_color(ctx, GColorLightGray);
  graphics_draw_text(ctx, "#  DROID  DATE", fonts_get_system_font(FONT_KEY_GOTHIC_14),
      GRect(HS_COL1_X, HS_HEADER_Y, SCREEN_W - 2 * HS_COL1_X, 16), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);

  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  graphics_draw_line(ctx, GPoint(HS_COL1_X, HS_SEP_Y), GPoint(SCREEN_W - HS_COL1_X, HS_SEP_Y));

  int y = HS_ROW_Y;
  for (uint32_t i = 0; i < hsd->count; i++) {
    struct tm *lt = localtime(&hsd->entries[i].timestamp);
    if (lt) strftime(date_buf, sizeof(date_buf), "%x", lt);
    else snprintf(date_buf, sizeof(date_buf), "???");

    int is_new = hsd->last_made_it
      && hsd->entries[i].droid_num == hsd->last_droid
      && hsd->entries[i].timestamp == hsd->last_timestamp;

    graphics_context_set_text_color(ctx, is_new ? GColorYellow : GColorWhite);

    snprintf(buf, sizeof(buf), "%lu", (unsigned long)(i + 1));
    graphics_draw_text(ctx, buf, fonts_get_system_font(FONT_KEY_GOTHIC_14),
        GRect(HS_COL1_X, y, HS_COL1_W, 16), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);

    snprintf(buf, sizeof(buf), "%03lu", (unsigned long)hsd->entries[i].droid_num);
    graphics_draw_text(ctx, buf, fonts_get_system_font(FONT_KEY_GOTHIC_14),
        GRect(HS_COL2_X, y, HS_COL2_W, 16), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);

    graphics_draw_text(ctx, date_buf, fonts_get_system_font(FONT_KEY_GOTHIC_14),
        GRect(HS_COL3_X, y, HS_COL3_W, 16), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);

    y += HS_ROW_H;
  }

  if (!hsd->last_made_it && hsd->last_droid > 0) {
    struct tm *lt = localtime(&hsd->last_timestamp);
    if (lt) strftime(date_buf, sizeof(date_buf), "%x", lt);
    else snprintf(date_buf, sizeof(date_buf), "???");

    y += HS_LATEST_OFF;
    graphics_context_set_text_color(ctx, GColorLightGray);
    graphics_draw_text(ctx, "Latest:", fonts_get_system_font(FONT_KEY_GOTHIC_14),
        GRect(HS_LATEST_X, y, HS_LATEST_W, 16), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);

    snprintf(buf, sizeof(buf), "%03lu", (unsigned long)hsd->last_droid);
    graphics_context_set_text_color(ctx, GColorWhite);
    graphics_draw_text(ctx, buf, fonts_get_system_font(FONT_KEY_GOTHIC_14),
        GRect(HS_LATEST_DROID_X, y, HS_LATEST_DROID_W, 16), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);

    graphics_draw_text(ctx, date_buf, fonts_get_system_font(FONT_KEY_GOTHIC_14),
        GRect(HS_LATEST_DATE_X, y, HS_LATEST_DATE_W, 16), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  }

  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(ctx, "Press SELECT", fonts_get_system_font(FONT_KEY_GOTHIC_14),
      GRect(0, PRESS_SEL_Y, SCREEN_W, 20), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

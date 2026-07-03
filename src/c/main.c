#include "takeover.h"

// Layout constants for emery (200×228)
#define CELL_W 16
#define CELL_H 20
#define GRID_L 4
#define COL_W 10
#define GAP 5
#define GRID_W (GRID_L * CELL_W)
#define TOTAL_W (GRID_W * 2 + GAP * 2 + COL_W)
#define GRID_X ((200 - TOTAL_W) / 2)
#define COL_X (GRID_X + GRID_W + GAP)
#define GRID2_X (COL_X + COL_W + GAP)
#define TOP_BAR_H 32
#define GRID_Y (TOP_BAR_H + 4)
#define BOARD_H (NUM_LINES * CELL_H)
#define BOT_BAR_Y (GRID_Y + BOARD_H + 2)


#define STORAGE_KEY_PLAYER 0
#define STORAGE_KEY_ENEMY_IDX 1
#define STORAGE_KEY_PLAYER_IDX 2

static Window *s_win;
static Layer *s_canvas;
static AppTimer *s_timer;
static GameState s_gs;
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


static GColor phase_color(GColor base, int phase) {
  if (phase == INACTIVE) return GColorDarkGray;
  // For active phases, use the base color (animation handled by brightness via draw style)
  return base;
}

static void load_droid_bitmap(int idx) {
  if (s_droid_bitmap) gbitmap_destroy(s_droid_bitmap);
  s_droid_bitmap = gbitmap_create_with_resource(DROID_IDS[idx]);
}

static void unload_droid_bitmap(void) {
  if (s_droid_bitmap) {
    gbitmap_destroy(s_droid_bitmap);
    s_droid_bitmap = NULL;
  }
}

static void draw_tile(GContext *ctx, int x, int y, int elem, int color, int phase) {
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
    case KABELENDE:
      graphics_fill_rect(ctx, GRect(x + 1, my - 1, CELL_W - 5, 3), 0, GCornerNone);
      graphics_fill_rect(ctx, GRect(x + CELL_W - 5, my - 2, 4, 4), 0, GCornerNone);
      break;
    case VERSTAERKER:
      graphics_fill_rect(ctx, GRect(x + 1, my - 1, CELL_W - 8, 3), 0, GCornerNone);
      graphics_fill_rect(ctx, GRect(x + CELL_W - 7, my - 4, 6, 8), 0, GCornerNone);
      break;
    case FARBTAUSCHER:
      graphics_fill_rect(ctx, GRect(x + 1, my - 1, CELL_W - 2, 3), 0, GCornerNone);
      graphics_draw_line(ctx, GPoint(x + 3, y + 2), GPoint(x + CELL_W - 3, y + CELL_H - 2));
      graphics_draw_line(ctx, GPoint(x + CELL_W - 3, y + 2), GPoint(x + 3, y + CELL_H - 2));
      break;
    case VERZWEIGUNG_O:
      graphics_fill_rect(ctx, GRect(x + 1, my, CELL_W - 2, 3), 0, GCornerNone);
      graphics_fill_rect(ctx, GRect(mx - 1, y + 1, 3, my - y), 0, GCornerNone);
      break;
    case VERZWEIGUNG_M:
      graphics_fill_rect(ctx, GRect(x + 1, my - 1, CELL_W - 2, 3), 0, GCornerNone);
      graphics_fill_rect(ctx, GRect(mx - 1, y + 1, 3, CELL_H - 2), 0, GCornerNone);
      break;
    case VERZWEIGUNG_U:
      graphics_fill_rect(ctx, GRect(x + 1, my - 2, CELL_W - 2, 3), 0, GCornerNone);
      graphics_fill_rect(ctx, GRect(mx - 1, my + 2, 3, CELL_H - my - 2), 0, GCornerNone);
      break;
    case GATTER_O:
      graphics_fill_rect(ctx, GRect(x + 1, my, CELL_W - 8, 3), 0, GCornerNone);
      graphics_draw_line(ctx, GPoint(x + CELL_W - 7, y + 2), GPoint(x + CELL_W - 2, my));
      break;
    case GATTER_M:
      graphics_fill_rect(ctx, GRect(x + 1, my - 1, CELL_W - 2, 3), 0, GCornerNone);
      break;
    case GATTER_U:
      graphics_fill_rect(ctx, GRect(x + 1, my - 2, CELL_W - 8, 3), 0, GCornerNone);
      graphics_draw_line(ctx, GPoint(x + CELL_W - 7, y + CELL_H - 2), GPoint(x + CELL_W - 2, my));
      break;
    case LEER:
      break;
  }
}

static void draw_cell(GContext *ctx, int x, int y, int elem, int color, int phase) {
  // Cell background
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(x, y, CELL_W, CELL_H), 0, GCornerNone);
  // Grid line
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_line(ctx, GPoint(x + CELL_W - 1, y), GPoint(x + CELL_W - 1, y + CELL_H - 1));
  // Tile content
  draw_tile(ctx, x, y, elem, color, phase);
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

static void draw_board(GContext *ctx, GameState *gs) {
  int by = GRID_Y;

  // Draw yellow grid (left side)
  for (int r = 0; r < NUM_LINES; r++) {
    for (int l = 0; l < NUM_LAYERS; l++) {
      int x = GRID_X + l * CELL_W;
      int y = by + r * CELL_H;
      int elem = gs->board[GELB][l][r];
      int phase = gs->activation[GELB][l][r];
      draw_cell(ctx, x, y, elem, GELB, phase);
    }
  }

  // Draw display column
  draw_led_col(ctx, COL_X, by, gs->display_column, gs->leader_color);

  // Draw violet grid (right side) — layers reversed so L3 (connection) is nearest the column
  for (int r = 0; r < NUM_LINES; r++) {
    for (int l = 0; l < NUM_LAYERS; l++) {
      int x = GRID2_X + (NUM_LAYERS - 1 - l) * CELL_W;
      int y = by + r * CELL_H;
      int elem = gs->board[VIOLETT][l][r];
      int phase = gs->activation[VIOLETT][l][r];
      draw_cell(ctx, x, y, elem, VIOLETT, phase);
    }
  }
}

static void draw_hud(GContext *ctx, GameState *gs) {
  // Top bar: player info vs enemy info
  char buf[32];

  // Player name
  snprintf(buf, sizeof(buf), "%s", droid_name_str(gs->player.num));
  graphics_context_set_text_color(ctx, GColorYellow);
  graphics_draw_text(ctx, buf, fonts_get_system_font(FONT_KEY_GOTHIC_14),
      GRect(2, 2, 60, 16), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);

  // Player capsules
  if (gs->phase == PHASE_PLAYING) {
    snprintf(buf, sizeof(buf), "%d", gs->player.caps);
    graphics_draw_text(ctx, buf, fonts_get_system_font(FONT_KEY_GOTHIC_14),
        GRect(2, 16, 60, 14), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  }

  // Enemy name
  snprintf(buf, sizeof(buf), "%s", droid_name_str(gs->enemy.num));
  graphics_context_set_text_color(ctx, GColorVividViolet);
  graphics_draw_text(ctx, buf, fonts_get_system_font(FONT_KEY_GOTHIC_14),
      GRect(200 - 62, 2, 60, 16), GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);

  // Enemy capsules
  if (gs->phase == PHASE_PLAYING) {
    snprintf(buf, sizeof(buf), "%d", gs->enemy.caps);
    graphics_draw_text(ctx, buf, fonts_get_system_font(FONT_KEY_GOTHIC_14),
        GRect(200 - 62, 16, 60, 14), GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);
  }

  // Center: countdown or phase indicator
  graphics_context_set_text_color(ctx, GColorWhite);
  if (gs->phase == PHASE_COLOR_SEL) {
    int left = COLOR_COUNTDOWN - gs->countdown;
    snprintf(buf, sizeof(buf), "Color %d", left);
    graphics_draw_text(ctx, buf, fonts_get_system_font(FONT_KEY_GOTHIC_14),
        GRect(60, 8, 80, 16), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  } else if (gs->phase == PHASE_PLAYING) {
    int left = GAME_COUNTDOWN - gs->countdown;
    snprintf(buf, sizeof(buf), "%d", left);
    graphics_draw_text(ctx, buf, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
        GRect(60, 4, 80, 20), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  }

  // Bottom: timer bar
  if (gs->phase == PHASE_PLAYING || gs->phase == PHASE_COLOR_SEL) {
    int max_count = (gs->phase == PHASE_COLOR_SEL) ? COLOR_COUNTDOWN : GAME_COUNTDOWN;
    int left = max_count - gs->countdown;
    int bar_w = 180 * left / max_count;
    graphics_context_set_fill_color(ctx, gs->your_color == GELB ? GColorYellow : GColorVividViolet);
    graphics_fill_rect(ctx, GRect(10, BOT_BAR_Y + 4, bar_w, 6), 2, GCornersAll);
    graphics_context_set_stroke_color(ctx, GColorDarkGray);
    graphics_context_set_stroke_width(ctx, 1);
    graphics_draw_rect(ctx, GRect(10, BOT_BAR_Y + 4, 180, 6));
  }

  // Result text
  if (gs->phase == PHASE_RESULT) {
    int mid_x = 100;
    int mid_y = 114;
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(mid_x - 50, mid_y - 16, 100, 32), 4, GCornersAll);
    graphics_context_set_text_color(ctx, gs->won ? GColorYellow : GColorRed);
    snprintf(buf, sizeof(buf), "%s", gs->won ? "CAPTURED" : "REJECTED");
    graphics_draw_text(ctx, buf, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
        GRect(mid_x - 46, mid_y - 10, 92, 20), GTextOverflowModeTrailingEllipsis,
        GTextAlignmentCenter, NULL);
  }
}

static void draw_phase_screen(GContext *ctx, GameState *gs) {
  if (gs->phase == PHASE_SHOW_PLAYER) {
    char buf[16];
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(0, 0, 200, 228), 0, GCornerNone);
    if (s_droid_bitmap) {
      graphics_draw_bitmap_in_rect(ctx, s_droid_bitmap, GRect(50, 40, 100, 136));
    }
    snprintf(buf, sizeof(buf), "You: %s", droid_name_str(gs->player.num));
    graphics_context_set_text_color(ctx, GColorYellow);
    graphics_draw_text(ctx, buf, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
        GRect(0, 10, 200, 24), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    graphics_context_set_text_color(ctx, GColorWhite);
    graphics_draw_text(ctx, "Press SELECT", fonts_get_system_font(FONT_KEY_GOTHIC_14),
        GRect(0, 196, 200, 20), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  } else if (gs->phase == PHASE_SHOW_ENEMY) {
    char buf[16];
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(0, 0, 200, 228), 0, GCornerNone);
    if (s_droid_bitmap) {
      graphics_draw_bitmap_in_rect(ctx, s_droid_bitmap, GRect(50, 40, 100, 136));
    }
    snprintf(buf, sizeof(buf), "Enemy: %s", droid_name_str(gs->enemy.num));
    graphics_context_set_text_color(ctx, GColorVividViolet);
    graphics_draw_text(ctx, buf, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
        GRect(0, 10, 200, 24), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    graphics_context_set_text_color(ctx, GColorWhite);
    graphics_draw_text(ctx, "Press SELECT", fonts_get_system_font(FONT_KEY_GOTHIC_14),
        GRect(0, 196, 200, 20), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  } else if (gs->phase == PHASE_COLOR_SEL) {
    // Draw game board in background
    draw_board(ctx, gs);
    // Overlay
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(40, 50, 120, 60), 4, GCornersAll);
    char buf[32];
    snprintf(buf, sizeof(buf), "Choose color");
    graphics_context_set_text_color(ctx, GColorWhite);
    graphics_draw_text(ctx, buf, fonts_get_system_font(FONT_KEY_GOTHIC_14),
        GRect(44, 54, 112, 16), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    snprintf(buf, sizeof(buf), "%s",
        gs->your_color == GELB ? "YELLOW" : "VIOLET");
    graphics_context_set_text_color(ctx,
        gs->your_color == GELB ? GColorYellow : GColorVividViolet);
    graphics_draw_text(ctx, buf, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
        GRect(44, 74, 112, 24), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  }
}

static void canvas_update(Layer *layer, GContext *ctx) {
  if (s_gs.phase == PHASE_PLAYING) {
    draw_board(ctx, &s_gs);
    draw_hud(ctx, &s_gs);

    // Draw capsule selector on the player's side
    int row = s_gs.capsule_row;
    int sel_x = s_gs.your_color == GELB ? GRID_X : GRID2_X;
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_stroke_width(ctx, 2);
    graphics_draw_rect(ctx, GRect(sel_x - 1, GRID_Y + row * CELL_H - 1, CELL_W * NUM_LAYERS + 2, CELL_H + 2));
  } else if (s_gs.phase == PHASE_RESULT) {
    draw_board(ctx, &s_gs);
    draw_hud(ctx, &s_gs);
  } else {
    draw_phase_screen(ctx, &s_gs);
    draw_hud(ctx, &s_gs);
  }
}

// Game timer callback
static void timer_cb(void *data) {
  GameState *gs = &s_gs;

  if (gs->phase == PHASE_PLAYING) {
    gs->tick++;

    int move_tick = (gs->tick % 3 == 0);
    int anim_tick = (gs->tick % 2 == 0);

    if (move_tick) {
      gs->countdown++;
      process_playground(gs->board, gs->activation);
      process_playground(gs->board, gs->activation);
      process_playground(gs->board, gs->activation);
      process_playground(gs->board, gs->activation);
      process_capsules(gs->board, gs->activation, gs->capsule_countdown);
      process_display_column(gs->board, gs->activation, gs->display_column, &gs->leader_color);

      enemy_movements(gs->capsule_countdown, gs->capsule_cur_row,
          gs->board, gs->activation, gs->opp_color, &gs->enemy.caps);
    }

    if (anim_tick) {
      animate_currents(gs->activation);
    }

    if (gs->countdown >= GAME_COUNTDOWN) {
      if (count_leds(gs->display_column, gs->your_color) >
          count_leds(gs->display_column, gs->opp_color)) {
        gs->won = 1;
      } else if (count_leds(gs->display_column, gs->opp_color) >
          count_leds(gs->display_column, gs->your_color)) {
        gs->won = 0;
      } else {
        gs->won = 1; // tie goes to player
      }
      gs->phase = PHASE_RESULT;
      gs->countdown = 0;
    }

    layer_mark_dirty(s_canvas);
    s_timer = app_timer_register(SHOW_TICK_MS, timer_cb, NULL);
  } else if (gs->phase == PHASE_COLOR_SEL) {
    gs->tick++;
    if (gs->tick % 3 == 0) gs->countdown++;
    layer_mark_dirty(s_canvas);
    if (gs->countdown >= COLOR_COUNTDOWN) {
        gs->countdown = 0;
        gs->phase = PHASE_PLAYING;
      }
    s_timer = app_timer_register(SHOW_TICK_MS, timer_cb, NULL);
  } else if (gs->phase == PHASE_RESULT) {
    gs->countdown++;
    layer_mark_dirty(s_canvas);
    if (gs->countdown > 10) {
      // Don't auto-advance, wait for button
    }
    s_timer = app_timer_register(SHOW_TICK_MS, timer_cb, NULL);
  }
}

// Button handlers
static void advance_show(void) {
  GameState *gs = &s_gs;
  if (gs->phase == PHASE_SHOW_PLAYER) {
    gs->phase = PHASE_SHOW_ENEMY;
    load_droid_bitmap(gs->enemy_idx);
    layer_mark_dirty(s_canvas);
  } else if (gs->phase == PHASE_SHOW_ENEMY) {
    unload_droid_bitmap();
    gs->phase = PHASE_COLOR_SEL;
    gs->countdown = 0;
    invent_playground(gs->board, gs->activation);
    for (int r = 0; r < NUM_LINES; r++)
      gs->display_column[r] = r % 2;
    layer_mark_dirty(s_canvas);
    if (s_timer) app_timer_cancel(s_timer);
    s_timer = app_timer_register(SHOW_TICK_MS, timer_cb, NULL);
  } else if (gs->phase == PHASE_RESULT) {
    unload_droid_bitmap();
    if (gs->won) {
      gs->player.num = gs->enemy.num;
      gs->player.idx = gs->enemy_idx;
      gs->player.cls = droid_class(gs->player.num, 6);
      gs->player.caps = PLAYER_CAPSULES(gs->player.cls);
      gs->enemy_idx++;
      if (gs->enemy_idx >= NUM_DROID_TYPES) {
        gs->enemy_idx = NUM_DROID_TYPES - 1;
        // You win! All droids captured
        gs->phase = PHASE_SHOW_PLAYER;
        gs->player.num = 1;
        gs->player.idx = 0;
        gs->player.cls = droid_class(1, 6);
        gs->player.caps = PLAYER_CAPSULES(gs->player.cls);
        gs->enemy_idx = 1;
      }
      gs->enemy.num = s_droid_types[gs->enemy_idx];
      gs->enemy.cls = droid_class(gs->enemy.num, 7);
      gs->enemy.caps = ENEMY_CAPSULES(gs->enemy.cls);
    } else {
      gs->player.num = 1;
      gs->player.idx = 0;
      gs->player.cls = droid_class(1, 6);
      gs->player.caps = PLAYER_CAPSULES(gs->player.cls);
      gs->enemy_idx = 1;
      gs->enemy.num = s_droid_types[1];
      gs->enemy.cls = droid_class(gs->enemy.num, 7);
      gs->enemy.caps = ENEMY_CAPSULES(gs->enemy.cls);
    }
    // Persist
    persist_write_int(STORAGE_KEY_PLAYER, gs->player.num);
    persist_write_int(STORAGE_KEY_PLAYER_IDX, gs->player.idx);
    persist_write_int(STORAGE_KEY_ENEMY_IDX, gs->enemy_idx);
    // Reset game state
    gs->your_color = GELB;
    gs->opp_color = VIOLETT;
    gs->capsule_row = 0;
    gs->countdown = 0;
    gs->leader_color = REMIS;
    gs->result = 0;
    gs->tick = 0;
    gs->color_chosen = 0;
    gs->capsule_cur_row[GELB] = 0;
    gs->capsule_cur_row[VIOLETT] = 0;
    memset(gs->capsule_countdown, -1, sizeof(gs->capsule_countdown));
    for (int r = 0; r < NUM_LINES; r++) gs->display_column[r] = r % 2;
    if (s_timer) app_timer_cancel(s_timer);
    s_timer = NULL;
    load_droid_bitmap(gs->player.idx);
    gs->phase = PHASE_SHOW_PLAYER;
    layer_mark_dirty(s_canvas);
  }
}

static void up_handler(ClickRecognizerRef recognizer, void *ctx) {
  GameState *gs = &s_gs;
  if (gs->phase == PHASE_COLOR_SEL) {
    gs->your_color = VIOLETT;
    gs->opp_color = GELB;
    layer_mark_dirty(s_canvas);
  } else if (gs->phase == PHASE_PLAYING) {
    gs->capsule_row--;
    if (gs->capsule_row < 0) gs->capsule_row = NUM_LINES - 1;
    layer_mark_dirty(s_canvas);
  }
}

static void down_handler(ClickRecognizerRef recognizer, void *ctx) {
  GameState *gs = &s_gs;
  if (gs->phase == PHASE_COLOR_SEL) {
    gs->your_color = GELB;
    gs->opp_color = VIOLETT;
    layer_mark_dirty(s_canvas);
  } else if (gs->phase == PHASE_PLAYING) {
    gs->capsule_row++;
    if (gs->capsule_row >= NUM_LINES) gs->capsule_row = 0;
    layer_mark_dirty(s_canvas);
  }
}

static void select_handler(ClickRecognizerRef recognizer, void *ctx) {
  GameState *gs = &s_gs;
  if (gs->phase == PHASE_SHOW_PLAYER || gs->phase == PHASE_SHOW_ENEMY
      || gs->phase == PHASE_RESULT) {
    advance_show();
  } else if (gs->phase == PHASE_PLAYING) {
    int row = gs->capsule_row;
    if (gs->player.caps > 0
        && gs->board[gs->your_color][0][row] != KABELENDE
        && gs->activation[gs->your_color][0][row] == INACTIVE) {
      gs->player.caps--;
      gs->board[gs->your_color][0][row] = VERSTAERKER;
      gs->activation[gs->your_color][0][row] = ACTIVE1;
      gs->capsule_countdown[gs->your_color][row] = CAPSULE_COUNTDOWN * 2;
      vibes_short_pulse();
    }
    layer_mark_dirty(s_canvas);
  }
}

static void click_config(void *ctx) {
  window_single_repeating_click_subscribe(BUTTON_ID_UP, 100, up_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 100, down_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_handler);
}

static void window_load(Window *w) {
  Layer *root = window_get_root_layer(w);
  GRect bounds = layer_get_bounds(root);

  s_canvas = layer_create(bounds);
  layer_set_update_proc(s_canvas, canvas_update);
  layer_add_child(root, s_canvas);

  window_set_click_config_provider(w, click_config);

  // Restore persisted state
  int saved = persist_read_int(STORAGE_KEY_PLAYER);
  if (saved > 0) {
    s_gs.player.num = saved;
    s_gs.player.idx = persist_read_int(STORAGE_KEY_PLAYER_IDX);
    if (s_gs.player.idx < 0 || s_gs.player.idx >= NUM_DROID_TYPES)
      s_gs.player.idx = 0;
    s_gs.player.cls = droid_class(saved, 6);
    s_gs.player.caps = PLAYER_CAPSULES(s_gs.player.cls);
    s_gs.enemy_idx = persist_read_int(STORAGE_KEY_ENEMY_IDX);
    if (s_gs.enemy_idx < 1 || s_gs.enemy_idx >= NUM_DROID_TYPES)
      s_gs.enemy_idx = 1;
    s_gs.enemy.num = s_droid_types[s_gs.enemy_idx];
    s_gs.enemy.cls = droid_class(s_gs.enemy.num, 7);
    s_gs.enemy.caps = ENEMY_CAPSULES(s_gs.enemy.cls);
  }
  load_droid_bitmap(s_gs.player.idx);
}

static void window_unload(Window *w) {
  if (s_timer) app_timer_cancel(s_timer);
  unload_droid_bitmap();
  layer_destroy(s_canvas);
}

static void init(void) {
  srand(time(NULL));
  init_game(&s_gs);

  s_win = window_create();
  window_set_background_color(s_win, GColorBlack);
  window_set_window_handlers(s_win, (WindowHandlers){
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_win, true);
}

static void deinit(void) {
  window_destroy(s_win);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
  return 0;
}

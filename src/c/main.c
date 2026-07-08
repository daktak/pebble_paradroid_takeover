#include <locale.h>
#include <string.h>
#include "takeover.h"
#include "render.h"

static Window *s_win;
static Layer *s_canvas;
static AppTimer *s_timer;
static GameState s_gs;
static HighScoreData s_hsd;

static void set_player_droid(GameState *gs, int num, int idx) {
  gs->player.num = num;
  gs->player.idx = idx;
  gs->player.cls = droid_class(num, 6);
  gs->player.caps = PLAYER_CAPSULES(gs->player.cls);
}

static void set_enemy_droid(GameState *gs, int idx) {
  gs->enemy.num = s_droid_types[idx];
  gs->enemy.cls = droid_class(gs->enemy.num, 7);
  gs->enemy.caps = ENEMY_CAPSULES(gs->enemy.cls);
}

static void reset_game_state(GameState *gs) {
  gs->your_color = GELB;
  gs->opp_color = VIOLETT;
  gs->capsule_row = 0;
  gs->countdown = 0;
  gs->leader_color = REMIS;
  gs->tick = 0;
  gs->flicker = 0;
  gs->direction = 1;
  gs->capsule_cur_row[GELB] = 0;
  gs->capsule_cur_row[VIOLETT] = 0;
  memset(gs->capsule_countdown, -1, sizeof(gs->capsule_countdown));
  for (int r = 0; r < NUM_LINES; r++)
    gs->display_column[r] = r % 2;
}

static void canvas_update(Layer *layer, GContext *ctx) {
  if (s_gs.phase == PHASE_PLAYING) {
    draw_board(ctx, &s_gs);
    draw_hud(ctx, &s_gs);

    int row = s_gs.capsule_row;
    int sel_x = s_gs.your_color == GELB ? GRID_X : GRID2_X;
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_stroke_width(ctx, 2);
    graphics_draw_rect(ctx, GRect(sel_x - 1, GRID_Y + row * CELL_H - 1, CELL_W * NUM_LAYERS + 2, CELL_H + 2));
  } else if (s_gs.phase == PHASE_RESULT) {
    draw_board(ctx, &s_gs);
    draw_hud(ctx, &s_gs);
  } else if (s_gs.phase == PHASE_HIGH_SCORES) {
    draw_high_scores(ctx, &s_hsd);
  } else {
    draw_phase_screen(ctx, &s_gs);
    draw_hud(ctx, &s_gs);
  }
}

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
      process_display_column(gs->board, gs->activation, gs->display_column, &gs->leader_color, &gs->flicker);

      enemy_movements(gs->capsule_countdown, gs->capsule_cur_row,
          gs->board, gs->activation, gs->opp_color, &gs->enemy.caps, &gs->direction);
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
        gs->won = 2;
      }
      gs->phase = PHASE_RESULT;
      gs->countdown = 0;
    }

    layer_mark_dirty(s_canvas);
  } else if (gs->phase == PHASE_COLOR_SEL) {
    gs->tick++;
    if (gs->tick % 3 == 0) gs->countdown++;
    layer_mark_dirty(s_canvas);
    if (gs->countdown >= COLOR_COUNTDOWN) {
        gs->countdown = 0;
        gs->phase = PHASE_PLAYING;
      }
  } else if (gs->phase == PHASE_RESULT) {
    gs->countdown++;
    layer_mark_dirty(s_canvas);
  }

  s_timer = app_timer_register(SHOW_TICK_MS, timer_cb, NULL);
}

static void advance_show(void) {
  GameState *gs = &s_gs;

  switch (gs->phase) {
    case PHASE_SHOW_PLAYER:
      gs->phase = PHASE_SHOW_ENEMY;
      load_droid_bitmap(gs->enemy_idx);
      layer_mark_dirty(s_canvas);
      break;

    case PHASE_SHOW_ENEMY:
      unload_droid_bitmap();
      gs->phase = PHASE_COLOR_SEL;
      gs->countdown = 0;
      invent_playground(gs->board, gs->activation);
      for (int r = 0; r < NUM_LINES; r++)
        gs->display_column[r] = r % 2;
      layer_mark_dirty(s_canvas);
      if (s_timer) app_timer_cancel(s_timer);
      s_timer = app_timer_register(SHOW_TICK_MS, timer_cb, NULL);
      break;

    case PHASE_RESULT: {
      int show_high_scores = 0;
      unload_droid_bitmap();

      if (gs->won == 1) {
        set_player_droid(gs, gs->enemy.num, gs->enemy_idx);
        gs->enemy_idx++;
        if (gs->enemy_idx >= NUM_DROID_TYPES) {
          highscores_add(&s_hsd, gs->player.num, time(NULL));
          highscores_save(&s_hsd);
          show_high_scores = 1;
          set_player_droid(gs, 1, 0);
          gs->enemy_idx = 1;
        }
        set_enemy_droid(gs, gs->enemy_idx);
        vibes_double_pulse();
      } else if (gs->won == 2) {
        reset_game_state(gs);
        gs->player.caps = PLAYER_CAPSULES(gs->player.cls);
        gs->enemy.caps = ENEMY_CAPSULES(gs->enemy.cls);
        invent_playground(gs->board, gs->activation);
        if (s_timer) app_timer_cancel(s_timer);
        s_timer = app_timer_register(SHOW_TICK_MS, timer_cb, NULL);
        gs->phase = PHASE_COLOR_SEL;
        layer_mark_dirty(s_canvas);
        return;
      } else {
        highscores_add(&s_hsd, gs->player.num, time(NULL));
        highscores_save(&s_hsd);
        show_high_scores = 1;
        set_player_droid(gs, 1, 0);
        gs->enemy_idx = 1;
        set_enemy_droid(gs, 1);
      }

      persist_write_int(STORAGE_KEY_PLAYER, gs->player.num);
      persist_write_int(STORAGE_KEY_PLAYER_IDX, gs->player.idx);
      persist_write_int(STORAGE_KEY_ENEMY_IDX, gs->enemy_idx);

      reset_game_state(gs);
      if (s_timer) app_timer_cancel(s_timer);
      s_timer = NULL;
      load_droid_bitmap(gs->player.idx);
      gs->phase = show_high_scores ? PHASE_HIGH_SCORES : PHASE_SHOW_PLAYER;
      layer_mark_dirty(s_canvas);
      break;
    }

    case PHASE_HIGH_SCORES:
      gs->phase = PHASE_SHOW_PLAYER;
      layer_mark_dirty(s_canvas);
      break;

    default:
      break;
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
      || gs->phase == PHASE_RESULT || gs->phase == PHASE_HIGH_SCORES) {
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
    set_enemy_droid(&s_gs, s_gs.enemy_idx);
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
  setlocale(LC_ALL, "");
  init_game(&s_gs);
  highscores_init(&s_hsd);

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

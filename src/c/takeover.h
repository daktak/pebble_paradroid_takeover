#ifndef TAKEOVER_H
#define TAKEOVER_H

#include <pebble.h>
#include <time.h>

#define NUM_LAYERS 4
#define NUM_LINES 8

#define NUM_PHASES 5
#define TO_BLOCKS 11
#define TO_ELEMENTS 6
#define TO_COLORS 2

#define COLOR_COUNTDOWN 30
#define GAME_COUNTDOWN 80
#define CAPSULE_COUNTDOWN 20
#define SHOW_TICK_MS 30

enum to_elements {
  EL_KABEL,
  EL_KABELENDE,
  EL_VERSTAERKER,
  EL_FARBTAUSCHER,
  EL_VERZWEIGUNG,
  EL_GATTER
};

enum to_blocks {
  KABEL,
  KABELENDE,
  VERSTAERKER,
  FARBTAUSCHER,
  VERZWEIGUNG_O,
  VERZWEIGUNG_M,
  VERZWEIGUNG_U,
  GATTER_O,
  GATTER_M,
  GATTER_U,
  LEER
};

enum condition {
  INACTIVE,
  ACTIVE1,
  ACTIVE2,
  ACTIVE3,
  ACTIVE4
};

enum to_colors { GELB, VIOLETT, REMIS };
enum game_phase {
  PHASE_SHOW_PLAYER,
  PHASE_SHOW_ENEMY,
  PHASE_COLOR_SEL,
  PHASE_PLAYING,
  PHASE_RESULT,
  PHASE_HIGH_SCORES
};

#define CONNECTOR 0
#define NON_CONNECTOR 1

typedef int playground_t[TO_COLORS][NUM_LAYERS][NUM_LINES];

#define NUM_DROID_TYPES 24
extern const int s_droid_types[NUM_DROID_TYPES];

#define PLAYER_CAPSULES(c) (3 + (c))
#define ENEMY_CAPSULES(c) (4 + (c))

typedef struct {
  int idx;
  int num;
  int cls;
  int caps;
} Droid;

#define MAX_HIGH_SCORES 5
#define STORAGE_KEY_HIGH_SCORES 3
#define STORAGE_KEY_PLAYER 0
#define STORAGE_KEY_ENEMY_IDX 1
#define STORAGE_KEY_PLAYER_IDX 2

typedef struct {
  uint32_t droid_num;
  time_t timestamp;
} HighScoreEntry;

typedef struct {
  uint32_t count;
  uint32_t last_droid;
  time_t last_timestamp;
  uint32_t last_made_it;
  HighScoreEntry entries[MAX_HIGH_SCORES];
} HighScoreData;

typedef struct {
  int phase;
  Droid player;
  int enemy_idx;
  Droid enemy;
  int your_color;
  int opp_color;
  int capsule_row;
  int countdown;
  int leader_color;
  int display_column[NUM_LINES];
  playground_t board;
  playground_t activation;
  int capsule_countdown[TO_COLORS][NUM_LINES];
  int capsule_cur_row[TO_COLORS];
  int tick;
  int color_chosen;
  int won;
} GameState;

// Layout constants — platform aware
#if PBL_DISPLAY_WIDTH > 180
// Emery (200x228)
#define CELL_W 22
#define CELL_H 20
#define DROID_W 100
#define DROID_H 136
#define DROID_X 50
#define DROID_Y 40
#define PRESS_SEL_Y 196
#define WIN_MSG_MID_Y 114
#define HS_TITLE_Y 8
#define HS_HEADER_Y 36
#define HS_SEP_Y 53
#define HS_ROW_Y 58
#define HS_ROW_H 18
#define HS_LATEST_OFF 4
#define COLOR_SEL_OX 40
#define COLOR_SEL_OY 50
#define COLOR_SEL_W 120
#define COLOR_SEL_H 60
#define COLOR_SEL_TX 44
#define COLOR_SEL_TY 54
#define COLOR_SEL_TW 112
#define HS_COL1_X 20
#define HS_COL1_W 20
#define HS_COL2_X 50
#define HS_COL2_W 50
#define HS_COL3_X 110
#define HS_COL3_W 70
#define HS_LATEST_X 20
#define HS_LATEST_W 60
#define HS_LATEST_DROID_X 80
#define HS_LATEST_DROID_W 40
#define HS_LATEST_DATE_X 130
#define HS_LATEST_DATE_W 50
#else
// Basalt (144x168)
#define CELL_W 15
#define CELL_H 14
#define DROID_W 72
#define DROID_H 98
#define DROID_X 36
#define DROID_Y 35
#define PRESS_SEL_Y 148
#define WIN_MSG_MID_Y 74
#define HS_TITLE_Y 6
#define HS_HEADER_Y 30
#define HS_SEP_Y 46
#define HS_ROW_Y 50
#define HS_ROW_H 15
#define HS_LATEST_OFF 2
#define COLOR_SEL_OX 22
#define COLOR_SEL_OY 40
#define COLOR_SEL_W 100
#define COLOR_SEL_H 60
#define COLOR_SEL_TX 26
#define COLOR_SEL_TY 44
#define COLOR_SEL_TW 92
#define HS_COL1_X 14
#define HS_COL1_W 18
#define HS_COL2_X 34
#define HS_COL2_W 36
#define HS_COL3_X 72
#define HS_COL3_W 58
#define HS_LATEST_X 14
#define HS_LATEST_W 44
#define HS_LATEST_DROID_X 60
#define HS_LATEST_DROID_W 32
#define HS_LATEST_DATE_X 94
#define HS_LATEST_DATE_W 40
#endif

#define TOP_BAR_H 32
#define SCREEN_W PBL_DISPLAY_WIDTH
#define SCREEN_H PBL_DISPLAY_HEIGHT
#define GRID_L 4
#define COL_W 10
#define GAP 5
#define GRID_W (GRID_L * CELL_W)
#define TOTAL_W (GRID_W * 2 + GAP * 2 + COL_W)
#define GRID_X ((SCREEN_W - TOTAL_W) / 2)
#define COL_X (GRID_X + GRID_W + GAP)
#define GRID2_X (COL_X + COL_W + GAP)
#define GRID_Y (TOP_BAR_H + 4)
#define BOARD_H (NUM_LINES * CELL_H)
#define BOT_BAR_Y (GRID_Y + BOARD_H + 2)

int droid_class(int num, int max_class);
void clear_playground(playground_t board, playground_t activation, int display_column[]);
void invent_playground(playground_t board, playground_t activation);
void process_playground(playground_t board, playground_t activation);
void process_display_column(playground_t board, playground_t activation, int display_column[], int *leader);
void process_capsules(playground_t board, playground_t activation, int capsule_countdown[TO_COLORS][NUM_LINES]);
void animate_currents(playground_t activation);
void enemy_movements(int capsule_countdown[TO_COLORS][NUM_LINES],
    int capsule_cur_row[], playground_t board, playground_t activation, int opp_color, int *enemy_caps);
int count_leds(int display_column[], int color);
void init_game(GameState *gs);
const char *droid_name_str(int num);
void highscores_init(HighScoreData *hsd);
uint32_t highscores_add(HighScoreData *hsd, uint32_t droid_num, time_t timestamp);
void highscores_save(HighScoreData *hsd);

#endif

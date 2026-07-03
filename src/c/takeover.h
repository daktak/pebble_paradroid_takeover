#ifndef TAKEOVER_H
#define TAKEOVER_H

#include <pebble.h>

#define NUM_LAYERS 4
#define NUM_LINES 8

#define NUM_PHASES 5
#define TO_BLOCKS 11
#define TO_ELEMENTS 6
#define NUM_FILL_BLOCKS 3
#define NUM_CAPS_BLOCKS 3
#define NUM_GROUND_BLOCKS 6
#define TO_COLORS 2
#define MAX_CAPSULES 13

#define COLOR_COUNTDOWN 30
#define GAME_COUNTDOWN 80
#define CAPSULE_COUNTDOWN 20
#define MOVE_TICK_MS 90
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
enum to_opponents { YOU, ENEMY };
enum game_phase {
  PHASE_MENU,
  PHASE_SHOW_PLAYER,
  PHASE_SHOW_ENEMY,
  PHASE_COLOR_SEL,
  PHASE_PLAYING,
  PHASE_RESULT
};

#define CONNECTOR 0
#define NON_CONNECTOR 1

typedef int playground_t[TO_COLORS][NUM_LAYERS][NUM_LINES];

#define NUM_DROID_TYPES 24
extern const int s_droid_types[NUM_DROID_TYPES];
extern const char *s_droid_names[NUM_DROID_TYPES];

#define PLAYER_CAPSULES(c) (3 + (c))
#define ENEMY_CAPSULES(c) (4 + (c))

typedef struct {
  int idx;
  int num;
  int cls;
  int caps;
} Droid;

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
  int result;
  int tick;
  int color_chosen;
  int timer_sec;
  int highlight;
  int won;
} GameState;

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
void format_droid_name(char *buf, int len, int num);

#endif

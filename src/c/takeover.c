#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "takeover.h"

const int s_droid_types[NUM_DROID_TYPES] = {
  1, 123, 139, 247, 249, 296, 302, 329, 420, 476, 493,
  516, 571, 598, 614, 615, 629, 711, 742, 751,
  821, 834, 883, 999
};

const int block_class[TO_BLOCKS] = {
  CONNECTOR,      // KABEL
  NON_CONNECTOR,  // KABELENDE
  CONNECTOR,      // VERSTAERKER
  CONNECTOR,      // FARBTAUSCHER
  CONNECTOR,      // VERZWEIGUNG_O
  NON_CONNECTOR,  // VERZWEIGUNG_M
  CONNECTOR,      // VERZWEIGUNG_U
  NON_CONNECTOR,  // GATTER_O
  CONNECTOR,      // GATTER_M
  NON_CONNECTOR,  // GATTER_U
  NON_CONNECTOR   // LEER
};

const int element_prob[TO_ELEMENTS] = {
  100,  // KABEL
  3,    // KABELENDE
  10,   // VERSTAERKER
  6,    // FARBTAUSCHER
  8,    // VERZWEIGUNG
  3     // GATTER
};

static int rand_upto(int max) {
  return rand() % max;
}

int droid_class(int num, int max_class) {
  int c = num / 100;
  return c > max_class ? max_class : c;
}

const char *droid_name_str(int num) {
  static char buf[8];
  snprintf(buf, sizeof(buf), "%03d", num);
  return buf;
}

void clear_playground(playground_t board, playground_t activation, int display_column[]) {
  for (int c = 0; c < TO_COLORS; c++)
    for (int l = 0; l < NUM_LAYERS; l++)
      for (int r = 0; r < NUM_LINES; r++) {
        activation[c][l][r] = INACTIVE;
        board[c][l][r] = KABEL;
      }
  if (display_column)
    for (int r = 0; r < NUM_LINES; r++)
      display_column[r] = r % 2;
}

void invent_playground(playground_t board, playground_t activation) {
  clear_playground(board, activation, NULL);

  for (int c = 0; c < TO_COLORS; c++) {
    for (int l = 1; l < NUM_LAYERS - 1; l++) {
      for (int r = 0; r < NUM_LINES; r++) {
        if (board[c][l][r] != KABEL) continue;

        int el = rand_upto(TO_ELEMENTS);
        if (rand_upto(100) > element_prob[el]) { r--; continue; }

        switch (el) {
          case EL_KABEL:
          case EL_KABELENDE: {
            int prev = board[c][l - 1][r];
            if (block_class[prev] == NON_CONNECTOR)
              board[c][l][r] = LEER;
            else if (el == EL_KABELENDE)
              board[c][l][r] = KABELENDE;
            break;
          }
          case EL_VERSTAERKER: {
            int prev = board[c][l - 1][r];
            if (block_class[prev] == NON_CONNECTOR)
              board[c][l][r] = LEER;
            else
              board[c][l][r] = VERSTAERKER;
            break;
          }
          case EL_FARBTAUSCHER:
            if (l != 2) { r--; continue; }
            if (block_class[board[c][l - 1][r]] == NON_CONNECTOR)
              board[c][l][r] = LEER;
            else
              board[c][l][r] = FARBTAUSCHER;
            break;
          case EL_VERZWEIGUNG:
            if (r > NUM_LINES - 3) { r--; continue; }
            if (block_class[board[c][l - 1][r + 1]] == NON_CONNECTOR)
              { r--; continue; }
            if (board[c][l - 1][r] == VERZWEIGUNG_O || board[c][l - 1][r] == VERZWEIGUNG_U
                || board[c][l - 1][r + 2] == VERZWEIGUNG_O || board[c][l - 1][r + 2] == VERZWEIGUNG_U)
              { r--; continue; }
            if (board[c][l - 1][r + 1] == KABEL)
              board[c][l - 1][r + 1] = KABELENDE;
            if (block_class[board[c][l - 1][r]] == CONNECTOR)
              board[c][l - 1][r] = KABELENDE;
            if (block_class[board[c][l - 1][r + 2]] == CONNECTOR)
              board[c][l - 1][r + 2] = KABELENDE;
            board[c][l][r] = VERZWEIGUNG_O;
            board[c][l][r + 1] = VERZWEIGUNG_M;
            board[c][l][r + 2] = VERZWEIGUNG_U;
            r += 2;
            break;
          case EL_GATTER:
            if (r > NUM_LINES - 3) { r--; continue; }
            if (block_class[board[c][l - 1][r]] == NON_CONNECTOR
                || block_class[board[c][l - 1][r + 2]] == NON_CONNECTOR)
              { r--; continue; }
            if (board[c][l - 1][r + 1] != KABELENDE
                && board[c][l - 1][r + 1] != LEER
                && board[c][l - 1][r + 1] != VERZWEIGUNG_M)
              { r--; continue; }
            board[c][l][r] = GATTER_O;
            board[c][l][r + 1] = GATTER_M;
            board[c][l][r + 2] = GATTER_U;
            r += 2;
            break;
          default: break;
        }
      }
    }
  }
}

void process_playground(playground_t board, playground_t activation) {
  for (int c = 0; c < TO_COLORS; c++) {
    for (int l = 1; l < NUM_LAYERS; l++) {
      for (int r = 0; r < NUM_LINES; r++) {
        if (l == NUM_LAYERS - 1) {
          int active = (activation[c][NUM_LAYERS - 2][r] >= ACTIVE1)
            && (block_class[board[c][NUM_LAYERS - 2][r]] == CONNECTOR);
          activation[c][l][r] = active ? ACTIVE1 : INACTIVE;
          continue;
        }

        int ta = 0;
        switch (board[c][l][r]) {
          case KABEL:
          case FARBTAUSCHER:
          case VERZWEIGUNG_M:
          case GATTER_O:
          case GATTER_U:
            if (activation[c][l - 1][r] >= ACTIVE1) ta = 1;
            break;
          case VERSTAERKER:
            if (activation[c][l - 1][r] >= ACTIVE1) ta = 1;
            if (activation[c][l][r] >= ACTIVE1) ta = 1;
            break;
          case KABELENDE:
          case LEER:
            break;
          case VERZWEIGUNG_O:
            if (r + 1 < NUM_LINES && activation[c][l][r + 1] >= ACTIVE1) ta = 1;
            break;
          case VERZWEIGUNG_U:
            if (r - 1 >= 0 && activation[c][l][r - 1] >= ACTIVE1) ta = 1;
            break;
          case GATTER_M:
            if (r > 0 && r < NUM_LINES - 1
                && activation[c][l][r - 1] >= ACTIVE1
                && activation[c][l][r + 1] >= ACTIVE1)
              ta = 1;
            break;
          default: break;
        }
        activation[c][l][r] = ta ? ACTIVE1 : INACTIVE;
      }
    }
  }
}

void process_capsules(playground_t board, playground_t activation,
    int capsule_countdown[TO_COLORS][NUM_LINES]) {
  for (int c = 0; c < TO_COLORS; c++)
    for (int r = 0; r < NUM_LINES; r++) {
      if (capsule_countdown[c][r] > 0)
        capsule_countdown[c][r]--;
      if (capsule_countdown[c][r] == 0) {
        capsule_countdown[c][r] = -1;
        activation[c][0][r] = INACTIVE;
        board[c][0][r] = KABEL;
      }
    }
}

void animate_currents(playground_t activation) {
  for (int c = 0; c < TO_COLORS; c++)
    for (int l = 0; l < NUM_LAYERS; l++)
      for (int r = 0; r < NUM_LINES; r++)
        if (activation[c][l][r] >= ACTIVE1) {
          activation[c][l][r]++;
          if (activation[c][l][r] >= NUM_PHASES)
            activation[c][l][r] = ACTIVE1;
        }
}

void process_display_column(playground_t board, playground_t activation,
    int display_column[], int *leader, int *flicker) {
  *flicker = !*flicker;
  int conn_layer = NUM_LAYERS - 1;
  int elem_layer = NUM_LAYERS - 2;
  for (int r = 0; r < NUM_LINES; r++) {
    int g_act = activation[GELB][conn_layer][r];
    int v_act = activation[VIOLETT][conn_layer][r];
    int g_elem = board[GELB][elem_layer][r];
    int v_elem = board[VIOLETT][elem_layer][r];

    if (g_act >= ACTIVE1 && v_act < ACTIVE1) {
      display_column[r] = board[GELB][elem_layer][r] == FARBTAUSCHER ? VIOLETT : GELB;
    } else if (g_act < ACTIVE1 && v_act >= ACTIVE1) {
      display_column[r] = board[VIOLETT][elem_layer][r] == FARBTAUSCHER ? GELB : VIOLETT;
    } else if (g_act >= ACTIVE1 && v_act >= ACTIVE1) {
      if (g_elem == FARBTAUSCHER && v_elem != FARBTAUSCHER)
        display_column[r] = VIOLETT;
      else if (g_elem != FARBTAUSCHER && v_elem == FARBTAUSCHER)
        display_column[r] = GELB;
      else
        display_column[r] = *flicker ? GELB : VIOLETT;
    }
  }

  int gc = 0, vc = 0;
  for (int r = 0; r < NUM_LINES; r++) {
    if (display_column[r] == GELB) gc++;
    else if (display_column[r] == VIOLETT) vc++;
  }
  if (vc < gc) *leader = GELB;
  else if (vc > gc) *leader = VIOLETT;
  else *leader = REMIS;
}

void enemy_movements(int capsule_countdown[TO_COLORS][NUM_LINES],
    int capsule_cur_row[], playground_t board, playground_t activation,
    int opp_color, int *enemy_caps, int *direction) {
  if (*enemy_caps <= 0) return;
  int row = capsule_cur_row[opp_color];
  switch (rand_upto(3)) {
    case 0:
      if (rand_upto(100) < 100) {
        row += *direction;
        if (row >= NUM_LINES) row = 0;
        if (row < 0) row = NUM_LINES - 1;
      }
      break;
    case 1:
      if (rand_upto(100) < 10) *direction *= -1;
      break;
    case 2:
      if (rand_upto(100) < 80) {
        if (board[opp_color][0][row] != KABELENDE
            && activation[opp_color][0][row] == INACTIVE) {
          (*enemy_caps)--;
          board[opp_color][0][row] = VERSTAERKER;
          activation[opp_color][0][row] = ACTIVE1;
          capsule_countdown[opp_color][row] = CAPSULE_COUNTDOWN * 2;
        }
      }
      break;
    default: break;
  }
  capsule_cur_row[opp_color] = row;
}

int count_leds(int display_column[], int color) {
  int n = 0;
  for (int r = 0; r < NUM_LINES; r++)
    if (display_column[r] == color) n++;
  return n;
}

void highscores_init(HighScoreData *hsd) {
  int r = persist_read_data(STORAGE_KEY_HIGH_SCORES, hsd, sizeof(HighScoreData));
  if (r < 0) {
    memset(hsd, 0, sizeof(HighScoreData));
  } else {
    if (hsd->count > MAX_HIGH_SCORES) hsd->count = 0;
    if (hsd->last_made_it > 1) hsd->last_made_it = 0;
  }
}

uint32_t highscores_add(HighScoreData *hsd, uint32_t droid_num, time_t timestamp) {
  hsd->last_droid = droid_num;
  hsd->last_timestamp = timestamp;
  hsd->last_made_it = 0;

  int pos = 0;
  while (pos < (int)hsd->count && hsd->entries[pos].droid_num > droid_num) pos++;

  if (pos < MAX_HIGH_SCORES) {
    int end = hsd->count;
    if (end > MAX_HIGH_SCORES - 1) end = MAX_HIGH_SCORES - 1;
    for (int i = end; i > pos; i--)
      hsd->entries[i] = hsd->entries[i - 1];

    hsd->entries[pos].droid_num = droid_num;
    hsd->entries[pos].timestamp = timestamp;
    if (hsd->count < MAX_HIGH_SCORES) hsd->count++;
    hsd->last_made_it = 1;
  }

  return hsd->last_made_it;
}

void highscores_save(HighScoreData *hsd) {
  persist_write_data(STORAGE_KEY_HIGH_SCORES, hsd, sizeof(HighScoreData));
}

void init_game(GameState *gs) {
  // Player starts as 001
  gs->phase = PHASE_SHOW_PLAYER;
  gs->player.num = 1;
  gs->player.idx = 0;
  gs->player.cls = droid_class(1, 6);
  gs->player.caps = PLAYER_CAPSULES(gs->player.cls);

  gs->enemy_idx = 1;
  gs->enemy.num = s_droid_types[1];
  gs->enemy.cls = droid_class(gs->enemy.num, 7);
  gs->enemy.caps = ENEMY_CAPSULES(gs->enemy.cls);

  gs->your_color = GELB;
  gs->opp_color = VIOLETT;
  gs->capsule_row = 0;
  gs->countdown = 0;
  gs->leader_color = REMIS;
  gs->tick = 0;
  gs->won = 0;
  gs->flicker = 0;
  gs->direction = 1;

  for (int r = 0; r < NUM_LINES; r++) gs->display_column[r] = r % 2;
  memset(gs->board, 0, sizeof(gs->board));
  memset(gs->activation, 0, sizeof(gs->activation));
  memset(gs->capsule_countdown, -1, sizeof(gs->capsule_countdown));
  gs->capsule_cur_row[GELB] = 0;
  gs->capsule_cur_row[VIOLETT] = 0;
}

#include "lib.h"

#include <stdio.h>
#include <stdlib.h>

#include "gba.h"
#include "images/bg.h"

int bulletInbounds(int row, int dim) {
  int min = PLAYER_OFFSET - PLAYER_SIZE / 2 - BULLET_SIZE / 2;
  int max = HEIGHT - min;
  return row > (dim == 1 ? 0 : min) && row < max;
}

int collide(struct player *p, int brow, int bcol) {
  int drow = brow - p->row;
  int dcol = bcol - p->col;
  return drow >= -PLAYER_SIZE / 2 && drow <= PLAYER_SIZE / 2 && dcol >= -PLAYER_SIZE / 2 && dcol <= PLAYER_SIZE / 2;
}

void readLeftRight(struct player *p, u16 left, u16 right, u16 currentButtons) {
  if (KEY_DOWN(left, currentButtons)) {
    if (p->col >= PLAYER_SIZE / 2 + PLAYER_SPEED) {
      p->direction = p->direction | 1;
    }
  }
  if (KEY_DOWN(right, currentButtons)) {
    if (p->col <= WIDTH - PLAYER_SIZE / 2 - PLAYER_SPEED) {
      p->direction = p->direction | 2;
    }
  }
}

void readUpDown(struct player *p, u16 up, u16 down, u16 currentButtons) {
  if (KEY_DOWN(up, currentButtons)) {
    if (p->row >= PLAYER_SIZE / 2 + PLAYER_SPEED) {
      p->direction = p->direction | 4;
    }
  }
  if (KEY_DOWN(down, currentButtons)) {
    if (p->row <= HEIGHT - 2 * INDICATOR_OFFSET - PLAYER_SIZE / 2 - PLAYER_SPEED) {
      p->direction = p->direction | 8;
    }
  }
}

void readShoot(struct player *p, u16 shoot, u16 currentButtons, u16 previousButtons) {  
  if (KEY_JUST_PRESSED(shoot, currentButtons, previousButtons)) {
    if (p->ammo > 0) {
      addBullet(p, p->row, p->col, 2);
      if (p->ammo == MAX_AMMO) {
        p->last_reload = vBlankCounter;
      }
      p->ammo--;
      drawAmmo(p, p->ammo, 0);
    }
  }
}

void addBullet(struct player *p, int row, int col, int dim) {
  for (int i = 0; i < BULLET_ARRAY_SIZE; ++i) {
    if (!bulletInbounds(p->bullet_rows[i], dim)) {
      p->bullet_rows[i] = row;
      p->bullet_cols[i] = col;
      return;
    }
  }
}

void updatePlayer(struct player *p) {
  if (p->direction) {
    drawPlayer(p, 0);
    if (p->direction & 1) {
      p->col -= PLAYER_SPEED;
    }
    if (p->direction & 2) {
      p->col += PLAYER_SPEED;
    }
    if (p->direction & 4) {
      p->row -= PLAYER_SPEED;
    }
    if (p->direction & 8) {
      p->row += PLAYER_SPEED;
    }
  }
  drawPlayer(p, 1);
}

void updateAmmo(struct player *p) {
  if (vBlankCounter - p->last_reload > BULLET_RELOAD_FRAMES && p->ammo < MAX_AMMO) {
    drawAmmo(p, p->ammo, 1);

    p->ammo++;
    p->last_reload = vBlankCounter;
  }
}

void drawArrow(int row, int start) {
  undrawImageDMA(row - 4, ARROW_OFFSET, ARROW_TRAVEL + 12, 8, bg);
  undrawImageDMA(row - 4, WIDTH - ARROW_OFFSET - ARROW_TRAVEL - 12, ARROW_TRAVEL + 12, 8, bg);
  
  int arrowPos = abs(((int)vBlankCounter - start) % (2 * ARROW_TRAVEL) - ARROW_TRAVEL);
  drawString(row - 4, ARROW_OFFSET + arrowPos, ">>", WHITE);
  drawString(row - 4, WIDTH - ARROW_OFFSET - arrowPos - 12, "<<", WHITE);
}

void drawPlayer(struct player *p, int show) {
  if (show) {
    drawImageDMA(p->row - PLAYER_SIZE / 2, p->col - PLAYER_SIZE / 2, PLAYER_SIZE, PLAYER_SIZE, p->tank_image);
  } else {
    drawRectDMA(p->row - PLAYER_SIZE / 2, p->col - PLAYER_SIZE / 2, PLAYER_SIZE, PLAYER_SIZE, BLACK);
  }
}

void drawHealth(struct player *p, int val, int show) {
  int col = INDICATOR_OFFSET + val * INDICATOR_SIZE * 3 / 2;
  if (show) {
    drawImageDMA(p->indicator_row - INDICATOR_SIZE / 2, col - INDICATOR_SIZE / 2, INDICATOR_SIZE, INDICATOR_SIZE, p->health_image);
  } else {
    drawRectDMA(p->indicator_row - INDICATOR_SIZE / 2, col - INDICATOR_SIZE / 2, INDICATOR_SIZE, INDICATOR_SIZE, BLACK);
  }
}

void drawAmmo(struct player *p, int val, int show) {
  int col = WIDTH - INDICATOR_OFFSET - val * INDICATOR_SIZE * 3 / 2;
  if (show) {
    drawImageDMA(p->indicator_row - INDICATOR_SIZE / 2, col - INDICATOR_SIZE / 2, INDICATOR_SIZE, INDICATOR_SIZE, p->ammo_image);
  } else {
    drawRectDMA(p->indicator_row - INDICATOR_SIZE / 2, col - INDICATOR_SIZE / 2, INDICATOR_SIZE, INDICATOR_SIZE, BLACK);
  }
}

void drawBullet(struct player *p, int i, int show) {
  if (!show) {
    drawRectDMA(p->bullet_rows[i] - BULLET_SIZE / 2, p->bullet_cols[i] - BULLET_SIZE / 2, BULLET_SIZE, BULLET_SIZE, BLACK);
  } else {
    drawImageDMA(p->bullet_rows[i] - BULLET_SIZE / 2, p->bullet_cols[i] - BULLET_SIZE / 2, BULLET_SIZE, BULLET_SIZE, p->bullet_image);
  }
}

void drawTimer(int start) {
  int time = (vBlankCounter - start) / 60;
  char str[50];
  sprintf(str, "%d:%02d", time / 60, time % 60);
  drawRectDMA(HEIGHT - INDICATOR_OFFSET - 4, WIDTH / 2 - 18, 36, 8, BLACK);
  drawCenteredString(HEIGHT - INDICATOR_OFFSET, 0, WIDTH, 0, str, WHITE);
}

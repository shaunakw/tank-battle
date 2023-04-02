#include "main.h"

#include <stdio.h>

#include "gba.h"
#include "lib.h"

#include "images/bg.h"
#include "images/dead.h"
#include "images/tank_blue.h"
#include "images/tank_red.h"
#include "images/heart_blue.h"
#include "images/heart_red.h"
#include "images/bullet_blue.h"
#include "images/bullet_red.h"
#include "images/ammo_blue.h"
#include "images/ammo_red.h"

enum gba_state {
  INIT,
  START,
  START_1P,
  START_2P,
  PLAY_1P,
  PLAY_2P,
  DEAD,
  BLUE_WIN,
  RED_WIN,
  LEADERBOARD,
};

struct player blue;
struct player red;

int modeSelect;
int leaderboard[5];

void init(void) {
  modeSelect = 1;
  for (int i = 0; i < 5; ++i) {
    leaderboard[i] = -1;
  }

  blue.tank_image = tank_blue;
  blue.health_image = heart_blue;
  blue.ammo_image = ammo_blue;
  blue.bullet_image = bullet_blue;
  blue.indicator_row = INDICATOR_OFFSET;

  red.tank_image = tank_red;
  red.health_image = heart_red;
  red.ammo_image = ammo_red;
  red.bullet_image = bullet_red;
  red.indicator_row = HEIGHT - INDICATOR_OFFSET;
}

void resetPlayers(void) {
  blue.ammo = MAX_AMMO;
  blue.health = MAX_HEALTH;
  blue.row = PLAYER_OFFSET;
  blue.col = WIDTH / 2;
  blue.direction = 0;
  blue.last_reload = vBlankCounter;

  red.ammo = MAX_AMMO;
  red.health = MAX_HEALTH;
  red.row = HEIGHT - PLAYER_OFFSET;
  red.col = WIDTH / 2;
  red.direction = 0;
  red.last_reload = vBlankCounter;

  for (int i = 0; i < BULLET_ARRAY_SIZE; ++i) {
    blue.bullet_rows[i] = 0;
    blue.bullet_cols[i] = 0;
    red.bullet_rows[i] = 0;
    red.bullet_cols[i] = 0;
  }
}

void updateLeaderboard(int start) {
  int time = (vBlankCounter - start) / 60;
  for (int i = 0; i < 5; ++i) {
    if (leaderboard[i] == -1) {
      leaderboard[i] = time;
      break;
    } else if (leaderboard[i] > time) {
      for (int j = 5; j > i; --j) {
        leaderboard[j] = leaderboard[j-1];
      }
      leaderboard[i] = time;
      break;
    }
  }
}

int main(void) {
  // Manipulate REG_DISPCNT here to set Mode 3.
  REG_DISPCNT = MODE3 | BG2_ENABLE;

  // Save current and previous state of button input.
  u32 previousButtons = BUTTONS;
  u32 currentButtons = BUTTONS;

  // Load initial application state
  enum gba_state state = INIT;
  int stateEntered = vBlankCounter;

  init();

  while (1) {
    currentButtons = BUTTONS; // Load the current state of the buttons

    enum gba_state newState = state;
    switch (state) {
      case INIT:
        newState = START;
        break;
      case START:
        if (KEY_JUST_PRESSED(BUTTON_L, currentButtons, previousButtons)) {
          newState = START_1P;
        } else if (KEY_JUST_PRESSED(BUTTON_R, currentButtons, previousButtons)) {
          newState = START_2P;
        }
        break;
      case START_1P:
        if (KEY_JUST_PRESSED(BUTTON_START, currentButtons, previousButtons)) {
          resetPlayers();
          newState = PLAY_1P;
        } else if (KEY_JUST_PRESSED(BUTTON_R, currentButtons, previousButtons)) {
          newState = START;
        }
        break;
      case START_2P:
        if (KEY_JUST_PRESSED(BUTTON_L, currentButtons, previousButtons)) {
          newState = LEADERBOARD;
        } else if (KEY_JUST_PRESSED(BUTTON_START, currentButtons, previousButtons)) {
          resetPlayers();
          newState = PLAY_2P;
        } else if (KEY_JUST_PRESSED(BUTTON_R, currentButtons, previousButtons)) {
          newState = START;
        }
        break;
      case PLAY_1P:
        red.direction = 0;
        readLeftRight(&red, BUTTON_LEFT, BUTTON_RIGHT, currentButtons);
        readUpDown(&red, BUTTON_UP, BUTTON_DOWN, currentButtons);
        if (red.health == 0) {
          newState = DEAD;
        }
        if ((vBlankCounter - stateEntered) % BULLET_RELOAD_FRAMES == 0) {
          addBullet(&blue, BULLET_SIZE / 2, randint(BULLET_SIZE, WIDTH - BULLET_SIZE), 1);
        }
        break;
      case PLAY_2P:
        blue.direction = 0;
        readLeftRight(&blue, BUTTON_A, BUTTON_B, currentButtons);
        readShoot(&blue, BUTTON_R, currentButtons, previousButtons);
        red.direction = 0;
        readLeftRight(&red, BUTTON_LEFT, BUTTON_RIGHT, currentButtons);
        readShoot(&red, BUTTON_UP, currentButtons, previousButtons);
        if (blue.health == 0) {
          updateLeaderboard(stateEntered);
          newState = RED_WIN;
        } else if (red.health == 0) {
          updateLeaderboard(stateEntered);
          newState = BLUE_WIN;
        }
        updateAmmo(&blue);
        updateAmmo(&red);
        break;
      case DEAD:
        if (KEY_JUST_PRESSED(BUTTON_R, currentButtons, previousButtons)) {
          newState = START_1P;
        }
        break;
      case BLUE_WIN:
      case RED_WIN:
      case LEADERBOARD:
        if (KEY_JUST_PRESSED(BUTTON_R, currentButtons, previousButtons)) {
          newState = START_2P;
        }
        break;
    }

    if (KEY_DOWN(BUTTON_SELECT, currentButtons)) {
      init();
      newState = START;
    }

    if (newState != state) {
      stateEntered = vBlankCounter;
    }

    waitForVBlank();

    switch (newState) {
      case INIT:
      case START:
        if (state != START) {
          drawFullScreenImageDMA(bg);
          drawCenteredString(30, 0, WIDTH, 0, "Welcome to Tank Battle!", WHITE);
          drawCenteredString(60, 0, WIDTH, 0, "Press A for 1-Player mode (boring)", WHITE);
          drawCenteredString(75, 0, WIDTH, 0, "Press S for 2-Player mode (fun)", WHITE);
        }
        break;
      case START_1P:
        if (state != START_1P) {
          drawFullScreenImageDMA(bg);
          drawCenteredString(15, 0, WIDTH, 0, "1-Player mode", WHITE);
          drawCenteredString(30, 0, WIDTH, 0, "Press S to return to mode select", WHITE);
          drawCenteredString(45, 0, WIDTH, 0, "Enter", WHITE);

          drawCenteredString(75, 0, WIDTH * 2 / 3, 0, "Controls", WHITE);
          drawCenteredString(90, 0, WIDTH * 2 / 3, 0, "Left", WHITE);
          drawCenteredString(105, 0, WIDTH * 2 / 3, 0, "Right", WHITE);
          drawCenteredString(120, 0, WIDTH * 2 / 3, 0, "Up", WHITE);
          drawCenteredString(135, 0, WIDTH * 2 / 3, 0, "Down", WHITE);

          drawCenteredString(75, 0, WIDTH * 4 / 3, 0, "Red", RED);
          drawCenteredString(90, 0, WIDTH * 4 / 3, 0, "<", RED);
          drawCenteredString(105, 0, WIDTH * 4 / 3, 0, ">", RED);
          drawCenteredString(120, 0, WIDTH * 4 / 3, 0, "/\\", RED);
          drawCenteredString(135, 0, WIDTH * 4 / 3, 0, "\\/", RED);
        }

        drawArrow(45, stateEntered);
        break;
      case START_2P:
        if (state != START_2P) {
          drawFullScreenImageDMA(bg);
          drawCenteredString(15, 0, WIDTH, 0, "2-Player mode ", WHITE);
          drawCenteredString(30, 0, WIDTH, 0, "Press A for leaderboard", WHITE);
          drawCenteredString(45, 0, WIDTH, 0, "Press S to return to mode select", WHITE);
          drawCenteredString(60, 0, WIDTH, 0, "Enter", WHITE);

          drawCenteredString(90, 0, WIDTH, 0, "Controls", WHITE);
          drawCenteredString(105, 0, WIDTH, 0, "Left", WHITE);
          drawCenteredString(120, 0, WIDTH, 0, "Right", WHITE);
          drawCenteredString(135, 0, WIDTH, 0, "Shoot", WHITE);

          drawCenteredString(90, 0, WIDTH / 2, 0, "Blue", BLUE);
          drawCenteredString(105, 0, WIDTH / 2, 0, "Z", BLUE);
          drawCenteredString(120, 0, WIDTH / 2, 0, "X", BLUE);
          drawCenteredString(135, 0, WIDTH / 2, 0, "S", BLUE);

          drawCenteredString(90, 0, WIDTH * 3 / 2, 0, "Red", RED);
          drawCenteredString(105, 0, WIDTH * 3 / 2, 0, "<", RED);
          drawCenteredString(120, 0, WIDTH * 3 / 2, 0, ">", RED);
          drawCenteredString(135, 0, WIDTH * 3 / 2, 0, "/\\", RED);
        }

        drawArrow(60, stateEntered);
        break;
      case PLAY_1P:
        if (state != PLAY_1P) {
          fillScreenDMA(BLACK);
          for (int i = 0; i < MAX_HEALTH; ++i) {
            drawHealth(&red, i, 1);
          }
        }
        for (int i = 0; i < BULLET_ARRAY_SIZE; ++i) {
          if (collide(&red, blue.bullet_rows[i], blue.bullet_cols[i])) {
            red.health--;
            drawHealth(&red, red.health, 0);

            drawBullet(&blue, i, 0);
            blue.bullet_rows[i] = 0;
            blue.bullet_cols[i] = 0;
          } else if (bulletInbounds(blue.bullet_rows[i], 1)) {
            drawBullet(&blue, i, 0);
            blue.bullet_rows[i] += BULLET_SPEED;
            if (bulletInbounds(blue.bullet_rows[i], 1)) {
              drawBullet(&blue, i, 1);
            }
          }
        }
        drawTimer(stateEntered);
        updatePlayer(&red);
        break;
      case PLAY_2P:
        if (state != PLAY_2P) {
          fillScreenDMA(BLACK);
          for (int i = 0; i < MAX_HEALTH; ++i) {
            drawHealth(&blue, i, 1);
            drawHealth(&red, i, 1);
          }
          for (int i = 0; i < MAX_AMMO; ++i) {
            drawAmmo(&blue, i, 1);
            drawAmmo(&red, i, 1);
          }
        }
        for (int i = 0; i < BULLET_ARRAY_SIZE; ++i) {
          if (collide(&red, blue.bullet_rows[i], blue.bullet_cols[i])) {
            red.health--;
            drawHealth(&red, red.health, 0);

            drawBullet(&blue, i, 0);
            blue.bullet_rows[i] = 0;
            blue.bullet_cols[i] = 0;
          } else if (bulletInbounds(blue.bullet_rows[i], 2)) {
            drawBullet(&blue, i, 0);
            blue.bullet_rows[i] += BULLET_SPEED;
            if (bulletInbounds(blue.bullet_rows[i], 2)) {
              drawBullet(&blue, i, 1);
            }
          }
          if (collide(&blue, red.bullet_rows[i], red.bullet_cols[i])) {
            blue.health--;
            drawHealth(&blue, blue.health, 0);

            drawBullet(&red, i, 0);
            red.bullet_rows[i] = 0;
            red.bullet_cols[i] = 0;
          } else if (bulletInbounds(red.bullet_rows[i], 2)) {
            drawBullet(&red, i, 0);
            red.bullet_rows[i] -= BULLET_SPEED;
            if (bulletInbounds(red.bullet_rows[i], 2)) {
              drawBullet(&red, i, 1);
            }
          }
        }
        drawTimer(stateEntered);
        updatePlayer(&blue);
        updatePlayer(&red);
        break;
      case DEAD:
        if (state != DEAD) {
          drawFullScreenImageDMA(dead);
          drawCenteredString(45, 0, WIDTH, 0, "You Died :(", WHITE);
          drawCenteredString(135, 0, WIDTH, 0, "Press S to return to start", WHITE);
        }
        break;
      case BLUE_WIN:
        if (state != BLUE_WIN) {
          fillScreenDMA(BLACK);
          drawCenteredString(45, 0, WIDTH, 0, "Blue Wins!", BLUE);
          drawCenteredString(135, 0, WIDTH, 0, "Press S to return to start", WHITE);
          drawImageFlippedDMA(90 - PLAYER_SIZE / 2, WIDTH / 2 - PLAYER_SIZE / 2, PLAYER_SIZE, PLAYER_SIZE, tank_blue);
        }
        break;
      case RED_WIN:
        if (state != RED_WIN) {
          fillScreenDMA(BLACK);
          drawCenteredString(45, 0, WIDTH, 0, "Red Wins!", RED);
          drawCenteredString(135, 0, WIDTH, 0, "Press S to return to start", WHITE);
          drawImageDMA(90 - PLAYER_SIZE / 2, WIDTH / 2 - PLAYER_SIZE / 2, PLAYER_SIZE, PLAYER_SIZE, tank_red);
        }
        break;
      case LEADERBOARD:
        if (state != LEADERBOARD) {
          drawFullScreenImageDMA(bg);
          drawCenteredString(15, 0, WIDTH, 0, "Leaderboard", WHITE);
          for (int i = 0; i < 5; ++i) {
            if (leaderboard[i] != -1) {
              char str[10];
              snprintf(str, 10, "%d. %d:%02d", i + 1, leaderboard[i] / 60, leaderboard[i] % 60);
              drawCenteredString(15 * (i + 3), 0, WIDTH, 0, str, WHITE);
            }
          }
          drawCenteredString(135, 0, WIDTH, 0, "Press S to return to start", WHITE);
        }
        break;
    }

    state = newState;
    previousButtons = currentButtons; // Store the current state of the buttons
  }

  return 0;
}

#ifndef LIB_H
#define LIB_H

#include "gba.h"

// START
#define ARROW_OFFSET 80
#define ARROW_TRAVEL 10

// PLAY
#define MAX_AMMO 3
#define MAX_HEALTH 3
#define PLAYER_SIZE 20
#define PLAYER_SPEED 2
#define PLAYER_OFFSET 40
#define BULLET_SIZE 6
#define BULLET_SPEED 4
#define BULLET_ARRAY_SIZE 6
#define BULLET_RELOAD_FRAMES 90
#define INDICATOR_SIZE 10
#define INDICATOR_OFFSET 15

struct player {
  int row;
  int col;
  int direction;
  int health;
  int ammo;
  u32 last_reload;
  int bullet_rows[BULLET_ARRAY_SIZE];
  int bullet_cols[BULLET_ARRAY_SIZE];
  int indicator_row;
  const unsigned short *tank_image;
  const unsigned short *health_image;
  const unsigned short *ammo_image;
  const unsigned short *bullet_image;
};

int bulletInbounds(int row, int dim);

int collide(struct player *p, int brow, int bcol);

void readLeftRight(struct player *p, u16 left, u16 right, u16 currentButtons);

void readUpDown(struct player *p, u16 up, u16 down, u16 currentButtons);

void readShoot(struct player *p, u16 shoot, u16 currentButtons, u16 previousButtons);

void updatePlayer(struct player *p);

void updateAmmo(struct player *p);

void addBullet(struct player *p, int row, int col, int dim);

void drawArrow(int row, int start);

void drawPlayer(struct player *p, int show);

void drawHealth(struct player *p, int val, int show);

void drawAmmo(struct player *p, int val, int show);

void drawBullet(struct player *p, int i, int show);

void drawTimer(int start);

#endif
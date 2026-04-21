#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "raylib.h"
#include "raymath.h"

#define MAX_BULLET_AMOUNT 1024

#define ENEMY_ROWS 5
#define ENEMY_COLS 10
#define ENEMY_SPACING 10
#define ENEMY_AMOUNT (ENEMY_ROWS * ENEMY_COLS)

typedef struct {
  Vector2 position;
  Vector2 size;
  bool alive;
} Enemy;

typedef struct {
  Vector2 position;
  Vector2 size;
  Vector2 velocity;
  bool alive;
} Bullet;

typedef struct {
  Vector2 player_position; // top left of player
  Vector2 player_size;

  Bullet bullets[MAX_BULLET_AMOUNT];
  size_t next_bullet;

  Enemy enemies[ENEMY_ROWS * ENEMY_COLS];
} GameState;

const int game_width = 400;
const int game_height = 400;
const float player_height = 50.0f;
GameState state = {.player_position = {0, game_height - player_height},
                   .player_size = {50.0f, player_height},
                   .next_bullet = 0};

int main(void) {
  InitWindow(game_width, game_height, "space invaders");

  {
    const float ENEMY_SIZE = 20.0;

    // init enemies
    for (size_t i = 0; i < ENEMY_AMOUNT; i++) {
      const float row = (float)(i / ENEMY_COLS);
      const float col = (float)(i % ENEMY_COLS);
      Enemy *cur_enemy = &state.enemies[i];
      cur_enemy->position.x = ENEMY_SIZE * col + (col * ENEMY_SPACING);
      cur_enemy->position.y = ENEMY_SIZE * row + (row * ENEMY_SPACING);
      cur_enemy->size.x = ENEMY_SIZE;
      cur_enemy->size.y = ENEMY_SIZE;
      cur_enemy->alive = true;
    }
  }

  while (!WindowShouldClose()) {
    const float dt = GetFrameTime();

    BeginDrawing();

    // UPDATE
    /////////////////

    int dir_x = 0;

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
      dir_x--;
    }
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
      dir_x++;
    }

    if (IsKeyPressed(KEY_SPACE)) {
      Bullet *new_bullet = &state.bullets[state.next_bullet];
      new_bullet->position.y = state.player_position.y;
      new_bullet->size.x = 10;
      new_bullet->size.y = 10;
      const float BULLET_SPEED = 250.0f;
      new_bullet->velocity.y = -1 * BULLET_SPEED;

      new_bullet->position.x = state.player_position.x +
                               state.player_size.x * .5f -
                               new_bullet->size.x * .5f;

      new_bullet->alive = true;

      state.next_bullet++;
      state.next_bullet %= MAX_BULLET_AMOUNT;
    }

    // TODO: put enemy updating here!

    for (size_t i = 0; i < MAX_BULLET_AMOUNT; i++) {
      Bullet *cur_bullet = &state.bullets[i];
      const bool alive = cur_bullet->alive;

      if (!alive) {
        continue;
      }

      cur_bullet->position.y += cur_bullet->velocity.y * dt;

      for (size_t i = 0; i < ENEMY_AMOUNT; i++) {
        Enemy *cur_enemy = &state.enemies[i];
        if (!cur_enemy->alive) {
          continue;
        }

        const float enemy_center_x =
            cur_enemy->position.x + .5f * cur_enemy->size.x;
        const float bullet_center_x =
            cur_bullet->position.x + .5f * cur_bullet->size.x;
        const float x_dist = enemy_center_x - bullet_center_x;
        const bool colliding_in_x = fabsf(x_dist) <= (cur_enemy->size.x * .5f +
                                                      cur_bullet->size.x * .5f);

        const float enemy_center_y =
            cur_enemy->position.y + .5f * cur_enemy->size.y;
        const float bullet_center_y =
            cur_bullet->position.y + .5f * cur_bullet->size.y;
        const float y_dist = enemy_center_y - bullet_center_y;
        const bool colliding_in_y = fabsf(y_dist) <= (cur_enemy->size.y * .5f +
                                                      cur_bullet->size.y * .5f);

        const bool colliding = colliding_in_x && colliding_in_y;

        if (colliding) {
          cur_enemy->alive = false;
          cur_bullet->alive = false;
        }
      }
    }

    const float player_speed = 250.0f;
    state.player_position.x += dir_x * dt * player_speed;

    state.player_position.x =
        Clamp(state.player_position.x, 0, game_width - state.player_size.y);

    // DRAW
    /////////////////

    ClearBackground(RAYWHITE);

    DrawRectangleV(state.player_position, state.player_size, BLUE);

    // DRAW ALL BULLETS
    for (size_t i = 0; i < MAX_BULLET_AMOUNT; i++) {
      const Bullet cur_bullet = state.bullets[i];
      if (!cur_bullet.alive) {
        continue;
      }
      DrawRectangleV(cur_bullet.position, cur_bullet.size, RED);
    }

    // DRAW ALL ENEMIES
    for (size_t i = 0; i < ENEMY_AMOUNT; i++) {
      const Enemy cur_enemy = state.enemies[i];
      if (!cur_enemy.alive) {
        continue;
      }

      DrawRectangleV(cur_enemy.position, cur_enemy.size, GREEN);
    }

    EndDrawing();
  }

  CloseWindow();
  return 0;
}

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
#define ENEMY_SIZE 20.0

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

  float step_size;
  float step_size_y;
  int step_dir;
  float step_acc;
  float step_interval;

  bool game_over;

  Enemy enemies[ENEMY_ROWS * ENEMY_COLS];
} GameState;

const int game_width = 400;
const int game_height = 400;
const float player_height = 50.0f;
GameState state = {
    .player_position = {0, game_height - player_height},
    .player_size = {50.0f, player_height},
    .next_bullet = 0,
    .step_size = 15.0,
    .step_size_y = 25.0,
    .step_dir = 1,
    .step_interval = 0.25,
    .step_acc = 0.0,
    .game_over = false,
};

int main(void) {
  InitWindow(game_width, game_height, "space invaders");

  {
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
    state.step_acc += dt;
    while (state.step_acc >= state.step_interval) {
      state.step_acc -= state.step_interval;
      // advance all enemy positions by some step size in some direction

      float amount_to_move = state.step_size * state.step_dir;

      if (state.step_dir > 0) {

        float max_x = 0.0;

        for (size_t j = 0; j < ENEMY_AMOUNT; j++) {
          Enemy *curr_enemy = &state.enemies[j];
          bool alive = curr_enemy->alive;

          if (!alive) {
            continue;
          }

          if (curr_enemy->position.x > max_x) {
            max_x = curr_enemy->position.x;
          }
        }

        if (game_width - (max_x + ENEMY_SIZE) <= state.step_size) {
          amount_to_move = (game_width - (max_x + ENEMY_SIZE));
          state.step_dir = -1;

          // move all enemies down by step_size_y
          for (size_t j = 0; j < ENEMY_AMOUNT; j++) {
            Enemy *curr_enemy = &state.enemies[j];
            bool alive = curr_enemy->alive;

            if (!alive) {
              continue;
            }

            curr_enemy->position.y += state.step_size_y;
          }
        }
      } else {
        float min_x = game_width;

        for (size_t j = 0; j < ENEMY_AMOUNT; j++) {
          Enemy *curr_enemy = &state.enemies[j];
          bool alive = curr_enemy->alive;

          if (!alive) {
            continue;
          }

          if (curr_enemy->position.x < min_x) {
            min_x = curr_enemy->position.x;
          }
        }

        if (min_x - state.step_size <= 0) {
          amount_to_move = -min_x;
          state.step_dir = 1;

          // move all enemies down by step_size_y
          for (size_t j = 0; j < ENEMY_AMOUNT; j++) {
            Enemy *curr_enemy = &state.enemies[j];
            bool alive = curr_enemy->alive;

            if (!alive) {
              continue;
            }

            curr_enemy->position.y += state.step_size_y;
          }
        }
      }

      for (size_t i = 0; i < ENEMY_AMOUNT; i++) {
        Enemy *curr_enemy = &state.enemies[i];

        curr_enemy->position.x += amount_to_move;

        // check if an enemy is at player height
        if (curr_enemy->position.y + ENEMY_SIZE >= state.player_position.y) {
          state.game_over = true;
        }
      }
    }

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

    bool allDead = true;
    for (size_t i = 0; i < ENEMY_AMOUNT; i++) {
      if (state.enemies[i].alive) {
        allDead = false;
        break;
      }
    }

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

    if (state.game_over) {
      DrawText("GAME OVER", 0, 0, 50.0, RED);
    }

    if (allDead) {
      DrawText("YOU WIN", 0, 0, 50.0, GREEN);
    }

    EndDrawing();
  }

  CloseWindow();
  return 0;
}

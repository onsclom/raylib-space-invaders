#include <stdbool.h>

#include "raylib.h"
#include "raymath.h"

#define MAX_BULLET_AMOUNT 1024
#define ENEMY_ROWS 5
#define ENEMY_COLS 10
#define ENEMY_SPACING 10
#define ENEMY_AMOUNT (ENEMY_ROWS * ENEMY_COLS)
#define ENEMY_SIZE 20.0f

typedef struct {
  Rectangle rect;
  bool alive;
} Enemy;

typedef struct {
  Rectangle rect;
  Vector2 velocity;
  bool alive;
} Bullet;

typedef struct {
  Rectangle player;

  Bullet bullets[MAX_BULLET_AMOUNT];
  size_t next_bullet;

  int step_dir;
  float step_acc;

  bool game_over;

  size_t alive_enemies;
  Enemy enemies[ENEMY_ROWS * ENEMY_COLS];
} GameState;

const float enemy_step_size = 15.0f;
const float enemy_step_size_y = 25.0f;
const float enemy_step_interval = 0.25f;
const int game_width = 400;
const int game_height = 400;
const float player_height = 50.0;

int main(void) {
  static GameState state = (GameState){
      .player = {0.0f, (float)game_height - player_height, 50.0f,
                 player_height},
      .next_bullet = 0,
      .step_dir = 1,
      .step_acc = 0.0,
      .game_over = false,
      .alive_enemies = ENEMY_AMOUNT,
  };
  for (size_t i = 0; i < ENEMY_AMOUNT; i++) {
    const float row = (float)(i / ENEMY_COLS);
    const float col = (float)(i % ENEMY_COLS);
    state.enemies[i].rect = (Rectangle){
        .x = col * (ENEMY_SIZE + ENEMY_SPACING),
        .y = row * (ENEMY_SIZE + ENEMY_SPACING),
        .width = (float)ENEMY_SIZE,
        .height = (float)ENEMY_SIZE,
    };
    state.enemies[i].alive = true;
  }

  InitWindow(game_width, game_height, "space invaders");
  while (!WindowShouldClose()) {
    const float dt = GetFrameTime();

    // UPDATE
    /////////////////

    if (!state.game_over && state.alive_enemies > 0) {
      {
        int dir_x = 0;
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
          dir_x--;
        }
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
          dir_x++;
        }
        const float player_speed = 250.0f;
        state.player.x += (float)dir_x * dt * player_speed;
        state.player.x =
            Clamp(state.player.x, 0.0f, (float)game_width - state.player.width);
      }

      if (IsKeyPressed(KEY_SPACE)) {
        Bullet *new_bullet = &state.bullets[state.next_bullet];
        new_bullet->rect.width = 10;
        new_bullet->rect.height = 10;
        new_bullet->rect.y = state.player.y;
        new_bullet->rect.x = state.player.x + state.player.width * .5f -
                             new_bullet->rect.width * .5f;
        const float bullet_speed = 250.0f;
        new_bullet->velocity.y = -bullet_speed;
        new_bullet->alive = true;
        state.next_bullet++;
        state.next_bullet %= MAX_BULLET_AMOUNT;
      }

      state.step_acc += dt;
      while (state.step_acc >= enemy_step_interval) {
        state.step_acc -= enemy_step_interval;
        float amount_to_move = enemy_step_size * (float)state.step_dir;

        float min_x = (float)game_width;
        float max_x = 0.0f;
        for (size_t j = 0; j < ENEMY_AMOUNT; j++) {
          Enemy *curr_enemy = &state.enemies[j];
          if (!curr_enemy->alive) {
            continue;
          }
          if (curr_enemy->rect.x < min_x) {
            min_x = curr_enemy->rect.x;
          }
          if (curr_enemy->rect.x > max_x) {
            max_x = curr_enemy->rect.x;
          }
        }

        float dist_to_wall =
            (state.step_dir > 0)
                ? (float)game_width - (max_x + (float)ENEMY_SIZE)
                : min_x;

        if (dist_to_wall <= enemy_step_size) {
          amount_to_move = (float)state.step_dir * dist_to_wall;
          state.step_dir = -state.step_dir;
          for (size_t j = 0; j < ENEMY_AMOUNT; j++) {
            Enemy *curr_enemy = &state.enemies[j];
            if (!curr_enemy->alive) {
              continue;
            }
            curr_enemy->rect.y += enemy_step_size_y;
          }
        }

        for (size_t i = 0; i < ENEMY_AMOUNT; i++) {
          Enemy *curr_enemy = &state.enemies[i];
          if (!curr_enemy->alive) {
            continue;
          }

          curr_enemy->rect.x += amount_to_move;

          if (CheckCollisionRecs(curr_enemy->rect, state.player)) {
            state.game_over = true;
          }
        }
      }

      for (size_t i = 0; i < MAX_BULLET_AMOUNT; i++) {
        Bullet *cur_bullet = &state.bullets[i];
        if (!cur_bullet->alive) {
          continue;
        }

        cur_bullet->rect.y += cur_bullet->velocity.y * dt;

        for (size_t j = 0; j < ENEMY_AMOUNT; j++) {
          Enemy *cur_enemy = &state.enemies[j];
          if (!cur_enemy->alive) {
            continue;
          }

          if (CheckCollisionRecs(cur_bullet->rect, cur_enemy->rect)) {
            cur_enemy->alive = false;
            cur_bullet->alive = false;
            state.alive_enemies--;
            break;
          }
        }
      }
    }

    // DRAW
    /////////////////

    BeginDrawing();

    ClearBackground(RAYWHITE);

    DrawRectangleRec(state.player, BLUE);

    // DRAW ALL BULLETS
    for (size_t i = 0; i < MAX_BULLET_AMOUNT; i++) {
      const Bullet cur_bullet = state.bullets[i];
      if (!cur_bullet.alive) {
        continue;
      }
      DrawRectangleRec(cur_bullet.rect, RED);
    }

    // DRAW ALL ENEMIES
    for (size_t i = 0; i < ENEMY_AMOUNT; i++) {
      const Enemy cur_enemy = state.enemies[i];
      if (!cur_enemy.alive) {
        continue;
      }

      DrawRectangleRec(cur_enemy.rect, GREEN);
    }

    if (state.game_over) {
      DrawText("GAME OVER", 0, 0, 50.0, RED);
    }

    if (state.alive_enemies == 0) {
      DrawText("YOU WIN", 0, 0, 50.0, GREEN);
    }

    EndDrawing();
  }

  CloseWindow();
  return 0;
}

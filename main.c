#include <stdio.h>
#include <stdlib.h>

#include "api.h"

#include "imgs.c"

#define UP 0x1
#define DOWN 0x2
#define LEFT 0x4
#define RIGHT 0x8
#define A 0x10
#define B 0x20
#define LOOP_FPS 20
#define PRESSED(button) ((buttons & button) == 0)

struct animation_def {
  int frames;  // TODO convert to byte
  byte width;
  byte height;
  byte reverse;
  const byte* data;
};

struct animation_def dive_def;
struct animation_def get_up_def;
struct animation_def run_def;
struct animation_def slide_def;

struct animation_state {
  int last_x;
  int last_y;
  byte last_width;
  byte last_height;
  int frame;
  const struct animation_def* def;
};

void init_animation_defs() {
  dive_def.frames = 5;
  dive_def.width = 16;
  dive_def.height = 16;
  dive_def.reverse = 0;
  dive_def.data = penguin_dive_;
  get_up_def.frames = 5;
  get_up_def.width = 16;
  get_up_def.height = 16;
  get_up_def.reverse = 1;
  get_up_def.data = penguin_dive_;
  run_def.frames = 5;
  run_def.width = 9;
  run_def.height = 16;
  run_def.reverse = 0;
  run_def.data = penguin_run_;
  slide_def.frames = 4;
  slide_def.width = 15;
  slide_def.height = 8;
  slide_def.reverse = 0;
  slide_def.data = penguin_slide_;
}

void draw_animation(struct animation_state* state, int x, int y) {
  if (state->last_height != 0) {
    CLEAR_SPRITE(state->last_x, state->last_y, state->last_width,
                 state->last_height);
  }
  DRAW_SPRITE((byte*)state->def->data +
                  state->frame * state->def->height / 8 * state->def->width,
              x, y, state->def->width, state->def->height, 0, DRAW_NOP);
  state->last_x = x;
  state->last_y = y;
  state->last_width = state->def->width;
  state->last_height = state->def->height;
}

void next_frame(struct animation_state* state) {
  if (state->def->reverse) {
    --state->frame;
  } else {
    ++state->frame;
  }
  state->frame = (state->frame + state->def->frames) % state->def->frames;
}

void reset(struct animation_state* state, const struct animation_def* def) {
  state->def = def;
  if (def->reverse) {
    state->frame = def->frames - 1;
  } else {
    state->frame = 0;
  }
}

void init_animation_state(struct animation_state* state,
                          const struct animation_def* def) {
  state->last_x = 0;
  state->last_y = 0;
  state->last_width = 0;
  state->last_height = 0;
  reset(state, def);
}

int done(struct animation_state* state) {
  if (state->def->reverse) {
    return state->frame == state->def->frames - 1;
  } else {
    return state->frame == 0;
  }
}

byte intersect(int x1, int y1, byte w1, byte h1, int x2, int y2, byte w2,
               byte h2) {
  return !(x1 + w1 < x2 || x2 + w2 < x1 || y1 + h1 < y2 || y2 + h2 < y1);
}

#define MAX_DY 8

int main() {
  struct animation_state penguin;
  int iters = 0;  // Numbers of iterations in the current state.
  byte state = 0;
  int x = 32;
  int y = 48;
  int dy = 0;
  byte buttons = 0;
  byte i = 0;  // tmp index.

  init_animation_defs();
  init_animation_state(&penguin, &run_def);
  api_init();

  // Clear the graphics in video memory
  CLEAR();

  for (;; ++iters) {
    buttons = READ_BUTTONS();

    y += dy >> 1;
    if (y + penguin.def->height > 64) {
      y = 64 - penguin.def->height;
    }
    ++dy;
    if (dy > MAX_DY) {
      dy = MAX_DY;
    }
    switch (state) {
      case 0:  // Running/Jumping
        if (iters & 1) next_frame(&penguin);
        if (PRESSED(A) && y + penguin.def->height == 64) {
          dy = -MAX_DY;
        } else if (PRESSED(B)) {
          reset(&penguin, &dive_def);
          state = 1;
          iters = 0;
        }
        break;
      case 1:                  // Diving
        next_frame(&penguin);  // TODO Fix this hack. Animation speed should be
                               // data, not code.
        if (done(&penguin)) {
          reset(&penguin, &slide_def);
          state = 2;
          iters = 0;
          y += 8;
        }
        break;
      case 2:  // Sliding
        if (iters & 1) next_frame(&penguin);
        if (iters > 12 && !PRESSED(B)) {
          reset(&penguin, &get_up_def);
          state = 3;
          iters = 0;
          y -= 8;
        }
        break;
      case 3:  // Getting up
        next_frame(&penguin);
        if (done(&penguin)) {
          reset(&penguin, &run_def);
          state = 0;
          iters = 0;
        }
        break;
    }

    draw_animation(&penguin, x, y);

    // Push video memory to the OLED
    DISPLAY();

    // Wait for next interrupt
    WAIT();
  }

  return 0;
}

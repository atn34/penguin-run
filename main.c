#include <stdio.h>
#include <stdlib.h>
#include "api.h"

#define UP 0x1
#define DOWN 0x2
#define LEFT 0x4
#define RIGHT 0x8
#define A 0x10
#define B 0x20
#define LOOP_FPS 20
#define PRESSED(button) ((buttons & button) == 0)

// 5 frames, 16x16
static byte const _dive[] = {0xe0, 0x18, 0x84, 0x02, 0x02, 0x8a, 0x24, 0x48, 0xb0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x08, 0x08, 0x09, 0xfa, 0x8b, 0x88, 0x04, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x0c, 0x42, 0x81, 0x81, 0xc5, 0x12, 0x24, 0xd8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x43, 0x64, 0x54, 0x0c, 0x04, 0x04, 0x04, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x10, 0x10, 0x48, 0x88, 0x04, 0x84, 0x02, 0x82, 0x24, 0xc8, 0x30, 0x00, 0x00, 0x00, 0x00, 0x43, 0x64, 0x54, 0x0c, 0x08, 0x09, 0x09, 0x04, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x20, 0x20, 0x20, 0xa0, 0x20, 0x20, 0x40, 0x40, 0x80, 0x00, 0x00, 0x0e, 0x02, 0x02, 0x02, 0x0f, 0x10, 0x26, 0x25, 0x24, 0x10, 0x28, 0x20, 0x14, 0x08, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x08, 0x08, 0x08, 0x3f, 0x40, 0x98, 0x94, 0x92, 0x40, 0xa0, 0x81, 0x51, 0x22, 0x1c, 0x00};

// 1 frame, 8x8
static byte const _ice_cube[] = {0xff, 0x81, 0x8d, 0x85, 0x81, 0x81, 0x81, 0xff};

// 5 frames, 9x16
static byte const _run[] = {0xe0, 0x18, 0x84, 0x02, 0x02, 0x8a, 0x24, 0x48, 0xb0, 0x07, 0x08, 0x08, 0x09, 0xfa, 0x8b, 0x88, 0x04, 0x03, 0xe0, 0x18, 0x84, 0x02, 0x02, 0x8a, 0x24, 0x48, 0xb0, 0x07, 0x08, 0x08, 0xe9, 0x9a, 0x8b, 0x08, 0x04, 0x03, 0xf0, 0x0c, 0x42, 0x81, 0x81, 0xc5, 0x12, 0x24, 0xd8, 0x03, 0x04, 0x74, 0x4c, 0x44, 0x04, 0x04, 0x02, 0x01, 0xf0, 0x0c, 0x42, 0x81, 0x81, 0xc5, 0x12, 0x24, 0xd8, 0x03, 0x04, 0x04, 0x34, 0x2c, 0x24, 0x04, 0x02, 0x01, 0xf0, 0x0c, 0x42, 0x81, 0x81, 0xc5, 0x12, 0x24, 0xd8, 0x03, 0x04, 0x04, 0x04, 0x7c, 0x44, 0x44, 0x02, 0x01};

// 4 frames, 15x8
static byte const _slide[] = {0x38, 0x08, 0x08, 0x08, 0x3e, 0x41, 0x99, 0xa1, 0x91, 0x89, 0x82, 0x51, 0x65, 0x21, 0x1e, 0x00, 0x38, 0x08, 0x08, 0x3e, 0x41, 0x99, 0x91, 0x91, 0x89, 0x82, 0x51, 0x65, 0x21, 0x1e, 0x00, 0x38, 0x08, 0x08, 0x3e, 0x41, 0x89, 0x89, 0x89, 0x89, 0x82, 0x51, 0x65, 0x21, 0x1e, 0x00, 0x38, 0x08, 0x08, 0x3e, 0x41, 0x99, 0x91, 0x91, 0x89, 0x82, 0x51, 0x65, 0x21, 0x1e};

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
    dive_def.data = _dive;
    get_up_def.frames = 5;
    get_up_def.width = 16;
    get_up_def.height = 16;
    get_up_def.reverse = 1;
    get_up_def.data = _dive;
    run_def.frames = 5;
    run_def.width = 9;
    run_def.height = 16;
    run_def.reverse = 0;
    run_def.data = _run;
    slide_def.frames = 4;
    slide_def.width = 15;
    slide_def.height = 8;
    slide_def.reverse = 0;
    slide_def.data = _slide;
}

void draw_animation(struct animation_state* state, int x, int y) {
    if (state->last_height != 0) {
        CLEAR_SPRITE(state->last_x, state->last_y, state->last_width, state->last_height);
    }
    DRAW_SPRITE((byte*) state->def->data + state->frame * state->def->height / 8 * state->def->width, x, y, state->def->width, state->def->height, 0, DRAW_NOP);
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

void init_animation_state(struct animation_state* state, const struct animation_def* def) {
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

byte intersect(int x1, int y1, byte w1, byte h1, int x2, int y2, byte w2, byte h2) {
    return !(x1 + w1 < x2 || x2 + w2 < x1 || y1 + h1 < y2 || y2 + h2 < y1);
}

#define MAX_DY 8

int main() {
    struct animation_state penguin;
    int iters = 0; // Numbers of iterations in the current state.
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

	for (;;++iters) {
	    
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
		    case 1:  // Diving
		    next_frame(&penguin);  // TODO Fix this hack. Animation speed should be data, not code.
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

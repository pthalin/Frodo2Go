#include <SDL/SDL.h>
#include "font.h"

#define NUM_ROWS 5
#define NUM_KEYS 17

//#ifdef RS97

#define KEY_UP SDLK_UP
#define KEY_DOWN SDLK_DOWN
#define KEY_LEFT SDLK_LEFT
#define KEY_RIGHT SDLK_RIGHT
#define KEY_ENTER SDLK_LCTRL // A
#define KEY_TOGGLE SDLK_LALT // B
#define KEY_BACKSPACE SDLK_BACKSPACE // R
#define KEY_SHIFT SDLK_TAB // L
#define KEY_LOCATION SDLK_LSHIFT // Y
#define KEY_ACTIVATE SDLK_SPACE // X
#define KEY_QUIT SDLK_ESCAPE // SELECT
#define KEY_HELP SDLK_RETURN // START

/*#else

#define KEY_UP SDLK_UP
define KEY_DOWN SDLK_DOWN
#define KEY_LEFT SDLK_LEFT
#define KEY_RIGHT SDLK_RIGHT
#define KEY_ENTER SDLK_RETURN
#define KEY_TOGGLE SDLK_SPACE
#define KEY_BACKSPACE SDLK_b
#define KEY_SHIFT SDLK_s
#define KEY_LOCATION SDLK_l
#define KEY_ACTIVATE SDLK_BACKQUOTE

#endif*/

#define KMOD_SYNTHETIC (1 << 13)

static int row_length[NUM_ROWS] = {15, 15, 15, 14, 7};
static SDLKey keys[2][NUM_ROWS][NUM_KEYS] = {
	{
		{SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5, SDLK_6, SDLK_7, SDLK_8,     SDLK_9,      SDLK_0,     SDLK_MINUS,    SDLK_EQUALS, SDLK_x, SDLK_x,      SDLK_F1},
		{SDLK_q, SDLK_w, SDLK_e, SDLK_r, SDLK_t, SDLK_y, SDLK_u, SDLK_i,     SDLK_o,      SDLK_p,     SDLK_x,        SDLK_x,      SDLK_x, SDLK_DELETE, SDLK_F3},
		{SDLK_a, SDLK_s, SDLK_d, SDLK_f, SDLK_g, SDLK_h, SDLK_j, SDLK_k,     SDLK_l,      SDLK_COLON, SDLK_SEMICOLON,SDLK_x,      SDLK_x, SDLK_x,      SDLK_F5},
		{SDLK_z, SDLK_x, SDLK_c, SDLK_v, SDLK_b, SDLK_n, SDLK_m, SDLK_COMMA, SDLK_PERIOD, SDLK_SLASH, SDLK_x,        SDLK_x,      SDLK_UP,             SDLK_F7},
		{SDLK_x, SDLK_SPACE, SDLK_LEFT, SDLK_DOWN, SDLK_RIGHT,   SDLK_RSHIFT, SDLK_LALT}
	}, {
		{SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5, SDLK_6, SDLK_7, SDLK_8,     SDLK_9,      SDLK_0,     SDLK_MINUS,    SDLK_EQUALS, SDLK_x, SDLK_x,      SDLK_F1},
		{SDLK_q, SDLK_w, SDLK_e, SDLK_r, SDLK_t, SDLK_y, SDLK_u, SDLK_i,     SDLK_o,      SDLK_p,     SDLK_x,        SDLK_x,      SDLK_x, SDLK_DELETE, SDLK_F3},
		{SDLK_a, SDLK_s, SDLK_d, SDLK_f, SDLK_g, SDLK_h, SDLK_j, SDLK_k,     SDLK_l,      SDLK_COLON, SDLK_SEMICOLON,SDLK_x,      SDLK_x, SDLK_x,      SDLK_F5},
		{SDLK_z, SDLK_x, SDLK_c, SDLK_v, SDLK_b, SDLK_n, SDLK_m, SDLK_COMMA, SDLK_PERIOD, SDLK_SLASH, SDLK_x,        SDLK_x,      SDLK_UP,             SDLK_F7},
		{SDLK_x, SDLK_SPACE, SDLK_LEFT, SDLK_DOWN, SDLK_RIGHT,   SDLK_RSHIFT, SDLK_LALT}
	}
};

char const *syms[2][NUM_ROWS][NUM_KEYS] = {
	{
	  {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0",    "+",     "-", "S:28", "HOM", "f1", NULL},
	  {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",    "S:256", "*", "S:31", "DEL", "f3", NULL},
	  {"A", "S", "D", "F", "G", "H", "J", "K", "L", ":",    ";",     "=", "S:30", "RES", "f5", NULL},
	  {"Z", "X", "C", "V", "B", "N", "M", ",", ".", "/",    "S:4096",     "   ",    "CTL", "f7", NULL},
	  {"R/S", "    SPACE    ",                            "S:4098", "S:4097", "S:4099","SHIFT", "C=",NULL}
	}, {
	  {"!", "\"","#", "$", "%", "&", "'", "(", ")", "0",    "+",     "-", "S:28", "CLR", "f2", NULL},
	  {"q", "w", "e", "r", "t", "y", "u", "i", "o", "p",    "S:256", "*", "S:31", "INS", "f4", NULL},
	  {"a", "s", "d", "f", "g", "h", "j", "k", "l", "S:27", "S:29",  "=", "S:30", "RES", "f6", NULL},
	  {"z", "x", "c", "v", "b", "n", "m", "<", ">", "?",      "U",     "   ",    "CTL", "f7", NULL},
	  {"R/S", "    SPACE    ",                            "L", "D", "R","SHIFT", "C=",NULL}

	}
};

static unsigned char toggled[NUM_ROWS][NUM_KEYS];

static int selected_i = 0, selected_j = 0;
static int shifted = 0;
static int location = 0;
static int active = 1;
static int mod_state = 0;
static int show_help = 0;

void init_keyboard() {
  for(int j = 0; j < NUM_ROWS; j++)
    for(int i = 0; i < NUM_KEYS; i++)
      toggled[j][i] = 0;
  selected_i = selected_j = shifted = location = 0;
  active = 1;
  mod_state = 0;
}

char const *help = 
"How to use:\n"
"  ARROWS: select key from keyboard\n"
"  A: press key\n"
"  B: toggle key (useful for shift/ctrl...)\n"
"  L: shift\n"
"  R: backspace\n"
"  Y: change keyboard location (top/bottom)\n"
"  X: show / hide keyboard\n"
"  SELECT: quit\n"
"  START: show this help\n"
;

int strlensym(char const *str) {
  if(strncmp(str,"S:",2) == 0) return 1;
  else return strlen(str);
}

void draw_keyboard(SDL_Surface* surface) {
  //	unsigned short bg_color = SDL_MapRGB(surface->format, 64, 64, 64);
  	unsigned short bg_color = SDL_MapRGB(surface->format, 180, 180, 180);
	unsigned short key_color = SDL_MapRGB(surface->format, 128, 128, 128);
	unsigned short text_color = SDL_MapRGB(surface->format, 0, 0, 0);
	unsigned short sel_color = SDL_MapRGB(surface->format, 128, 255, 128);
	unsigned short sel_toggled_color = SDL_MapRGB(surface->format, 255, 255, 128);
	unsigned short toggled_color = SDL_MapRGB(surface->format, 192, 192, 0);
	int length;
	int is_sym = 0;
	if(show_help) {
	  SDL_FillRect(surface, NULL, bg_color);
	  //draw_string(surface, "SDL Terminal by Benob, based on st-sdl", 42, 10, sel_toggled_color);
	  draw_string(surface, help, 8, 28, sel_color);
	  return;
	}
	if(!active) return;
	int total_length = -1;
	for(int i = 0; i < NUM_KEYS && syms[0][0][i]; i++) {
	  total_length += (1 + strlensym(syms[0][0][i])) * 6;
	}

	int center_x = (surface->w - total_length) / 2;
	int x = center_x, y = surface->h - 8 * (NUM_ROWS) - 16;
	if(location == 1) y = 16;

	SDL_Rect rect;
	rect.x = x - 4;
	rect.y = y - 3;
	rect.w = total_length + 3;
	rect.h = NUM_ROWS * 8 + 3;
	SDL_FillRect(surface, &rect, bg_color);

	for(int j = 0; j < NUM_ROWS; j++) {
	  x = center_x;
	  for(int i = 0; i < row_length[j]; i++) {
	    if(strncmp(syms[shifted][j][i],"S:",2) == 0) {
	      length = 1;
	      is_sym = 1;
	    } else {
	      length = strlensym(syms[shifted][j][i]);
	      is_sym = 0;
	    }	
	    SDL_Rect r2;
	    r2.x = x - 2;
	    r2.y = y - 1;
	    r2.w = length * 6 + 4;
	    r2.h = 7;
	    if(toggled[j][i]) {
	      if(selected_i == i && selected_j == j) {
		SDL_FillRect(surface, &r2, sel_toggled_color);
	      } else {
		SDL_FillRect(surface, &r2, toggled_color);
	      }
	    } else if(selected_i == i && selected_j == j) {
	      SDL_FillRect(surface, &r2, sel_color);
	    } else {
	      SDL_FillRect(surface, &r2, key_color);
	    }
	    if (is_sym) {
	      unsigned int sym_num = 0;
	      sscanf(syms[shifted][j][i],"S:%u", &sym_num);
	      draw_sym(surface, sym_num, x, y, text_color);
	    } else {
	      draw_string(surface, syms[shifted][j][i], x, y, text_color);
	    }
	    x += 6 * (length + 1);
	  }
	  y += 8;
	}
}

enum { STATE_TYPED, STATE_UP, STATE_DOWN };

void update_modstate(int key, int state) {
	//SDLMod mod_state = SDL_GetModState();
	if(state == STATE_DOWN) {
		if(key == SDLK_LSHIFT) mod_state |= KMOD_LSHIFT;
		else if(key == SDLK_RSHIFT) mod_state |= KMOD_RSHIFT;
		else if(key == SDLK_LCTRL) mod_state |= KMOD_LCTRL;
		else if(key == SDLK_RCTRL) mod_state |= KMOD_RCTRL;
		else if(key == SDLK_LALT) mod_state |= KMOD_LALT;
		else if(key == SDLK_RALT) mod_state |= KMOD_RALT;
		else if(key == SDLK_LMETA) mod_state |= KMOD_LMETA;
		else if(key == SDLK_RMETA) mod_state |= KMOD_RMETA;
		//else if(key == SDLK_NUM) mod_state |= KMOD_NUM;
		else if(key == SDLK_CAPSLOCK) mod_state |= KMOD_CAPS;
		else if(key == SDLK_MODE) mod_state |= KMOD_MODE;
	} else if(state == STATE_UP) {
		if(key == SDLK_LSHIFT) mod_state &= ~KMOD_LSHIFT;
		else if(key == SDLK_RSHIFT) mod_state &= ~KMOD_RSHIFT;
		else if(key == SDLK_LCTRL) mod_state &= ~KMOD_LCTRL;
		else if(key == SDLK_RCTRL) mod_state &= ~KMOD_RCTRL;
		else if(key == SDLK_LALT) mod_state &= ~KMOD_LALT;
		else if(key == SDLK_RALT) mod_state &= ~KMOD_RALT;
		else if(key == SDLK_LMETA) mod_state &= ~KMOD_LMETA;
		else if(key == SDLK_RMETA) mod_state &= ~KMOD_RMETA;
		//else if(key == SDLK_NUM) mod_state &= ~KMOD_NUM;
		else if(key == SDLK_CAPSLOCK) mod_state &= ~KMOD_CAPS;
		else if(key == SDLK_MODE) mod_state &= ~KMOD_MODE;
	}
	SDL_SetModState((SDLMod) mod_state);
}

void simulate_key(int key, int state) {
	update_modstate(key, state);
	unsigned short unicode = 0;
	SDL_Event event;
	if(key < 128) {
	  unicode = key;
	}
	event.key.type = SDL_KEYDOWN;
	event.key.state = SDL_PRESSED;
	event.key.keysym.scancode = 0;
	event.key.keysym.sym = (SDLKey) key;
	event.key.keysym.mod = (SDLMod) KMOD_SYNTHETIC;
	event.key.keysym.unicode = unicode;
	
	if(state == STATE_TYPED) {
		SDL_PushEvent(&event);
		event.key.type = SDL_KEYUP;
		event.key.state = SDL_RELEASED;
	} else if(state == STATE_UP) {
		event.key.type = SDL_KEYUP;
		event.key.state = SDL_RELEASED;
	}
	SDL_PushEvent(&event);
	printf("%d\n", key);
}

int compute_visual_offset(int col, int row) {
	int sum = 0;
	for(int i = 0; i < col; i++) sum += 1 + strlensym(syms[0][row][i]);
	sum += (1 + strlensym(syms[0][row][col])) / 2;
	return sum;
}

int compute_new_col(int visual_offset, int old_row, int new_row) {
	int new_sum = 0;
	int new_col = 0;
	while(new_col < row_length[new_row] - 1 && new_sum + (1 + strlensym(syms[0][new_row][new_col])) / 2 < visual_offset) {
		new_sum += 1 + strlensym(syms[0][new_row][new_col]);
		new_col++;
	}
	return new_col;
}

int handle_keyboard_event(SDL_Event* event) {
	static int visual_offset = 0;

	if(  event->key.type       == SDL_KEYDOWN &&
	   !(event->key.keysym.mod  & KMOD_SYNTHETIC) &&
	     event->key.keysym.sym == KEY_ACTIVATE) {
	  active = ! active;
	  return 1;
	}
	
	if(!active) {
	  return 0;
	}
	
	if((event->key.type == SDL_KEYUP ||
	    event->key.type == SDL_KEYDOWN)
	   &&
	   event->key.keysym.mod & KMOD_SYNTHETIC) {
	  return 0;
	}
	  
	if(event->key.type  == SDL_KEYDOWN &&
	   event->key.state == SDL_PRESSED) {
	  
	  if(event->key.keysym.sym == KEY_QUIT) {
	    exit(0);
	  } else if(event->key.keysym.sym == KEY_HELP) {
	    show_help = 1;
	  } else if(event->key.keysym.sym == KEY_UP && selected_j > 0) {
	    selected_i = compute_new_col(visual_offset, selected_j, selected_j - 1);
	    selected_j--;
	    //selected_i = selected_i * row_length[selected_j] / row_length[selected_j + 1];
	  } else if(event->key.keysym.sym == KEY_DOWN && selected_j < NUM_ROWS - 1) {
	    selected_i = compute_new_col(visual_offset, selected_j, selected_j + 1);
	    selected_j++;
	    //selected_i = selected_i * row_length[selected_j] / row_length[selected_j - 1];
	  } else if(event->key.keysym.sym == KEY_LEFT && selected_i > 0) {
	    selected_i--;
	    visual_offset = compute_visual_offset(selected_i, selected_j);
	  } else if(event->key.keysym.sym == KEY_RIGHT && selected_i < row_length[selected_j] - 1) {
	    selected_i++;
	    visual_offset = compute_visual_offset(selected_i, selected_j);
	  } else if(event->key.keysym.sym == KEY_SHIFT) {
	    shifted = 1;
	    toggled[4][5] = 1;
	    update_modstate(SDLK_LSHIFT, STATE_DOWN);
	  } else if(event->key.keysym.sym == KEY_LOCATION) {
	    location = !location;
	  } else if(event->key.keysym.sym == KEY_BACKSPACE) {
	    simulate_key(SDLK_BACKSPACE, STATE_TYPED);
	  } else if(event->key.keysym.sym == KEY_TOGGLE) {
	    toggled[selected_j][selected_i] = 1 - toggled[selected_j][selected_i];
	    if(toggled[selected_j][selected_i]) simulate_key(keys[shifted][selected_j][selected_i], STATE_DOWN);
	    else simulate_key(keys[shifted][selected_j][selected_i], STATE_UP);
	    if(selected_j == 4 && (selected_i == 0 || selected_i == 11)) shifted = toggled[selected_j][selected_i];
	  } else if(event->key.keysym.sym == KEY_ENTER) {
	    int key = keys[shifted][selected_j][selected_i];
	    if(mod_state & KMOD_CTRL) {
	      if (key >= 64 && key < 64 + 32) simulate_key(key - 64, STATE_DOWN);
	      else if (key >= 97 && key < 97 + 31) simulate_key(key - 96, STATE_DOWN);
	    } else if(mod_state & KMOD_SHIFT && key >= SDLK_a && key <= SDLK_z) {
	      simulate_key(key - SDLK_a + 'A', STATE_TYPED);
	    } else {
	      simulate_key(key, STATE_TYPED);
	    }
	  }
	} else if(event->key.type == SDL_KEYUP || event->key.state == SDL_RELEASED) {
	  if(show_help) {
	    show_help = 0;
	  } else if(event->key.keysym.sym == KEY_SHIFT) {
	    shifted = 0;
	    toggled[4][5] = 0;
	    update_modstate(SDLK_LSHIFT, STATE_UP);
	  }
	}
	return 1;
}

#ifdef TEST_KEYBOARD

int main() {
	SDL_Init( SDL_INIT_EVERYTHING );
	SDL_Surface* screen = SDL_SetVideoMode(320*2, 240 * 2, 16, SDL_SWSURFACE );
	SDL_Surface* buffer = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
	while(1) {
		SDL_Event event;
		while( SDL_PollEvent( &event ) ) {
			if( event.type == SDL_QUIT ) { 
				return 0;
			} else {
				handle_keyboard_event(&event);
			}
		}
		SDL_Rect r;
		r.x = 0;
		r.y = 0;
		r.w = buffer->w;
		r.h = buffer->h;
		SDL_FillRect(buffer, &r, 0);
		draw_keyboard(buffer);
		SDL_LockSurface(buffer);
		for(int j = 0; j < buffer->h; j++) {
			for(int i = 0; i < buffer->w; i++) {
			  SDL_Rect rect;
			  rect.x = i * 2;
			  rect.y = j * 2;
			  rect.w = 2;
			  rect.h = 2;
			  SDL_FillRect(screen, &rect, ((unsigned short*)buffer->pixels)[j * (buffer->pitch >> 1) + i]);
			}
		}
		SDL_UnlockSurface(buffer);
		SDL_Flip(screen);
		SDL_Delay(1000 / 30);
	}
	SDL_Quit();
}

#endif

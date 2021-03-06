#include <SDL/SDL.h>
#include "keyboard.h"
#include "font.h"

#define NUM_ROWS 5
#define NUM_KEYS 17

#define KEY_UP SDLK_UP
#define KEY_DOWN SDLK_DOWN
#define KEY_LEFT SDLK_LEFT
#define KEY_RIGHT SDLK_RIGHT

#define KEY_ENTER SDLK_LALT 
#define KEY_TOGGLE SDLK_LCTRL 
#define KEY_BACKSPACE SDLK_BACKSPACE // R
#define KEY_SHIFT SDLK_TAB // L

static int row_length[NUM_ROWS] = {15, 15, 15, 14, 7};
static SDLKey keys[NUM_ROWS][NUM_KEYS] = {
  {SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5, SDLK_6, SDLK_7, SDLK_8,     SDLK_9,      SDLK_0,     SDLK_PLUS,     SDLK_MINUS,    SDLK_END,       SDLK_PRINT,     SDLK_F1},
  {SDLK_q, SDLK_w, SDLK_e, SDLK_r, SDLK_t, SDLK_y, SDLK_u, SDLK_i,     SDLK_o,      SDLK_p,     SDLK_AT,       SDLK_ASTERISK, SDLK_BACKQUOTE, SDLK_BACKSPACE, SDLK_F3},
  {SDLK_a, SDLK_s, SDLK_d, SDLK_f, SDLK_g, SDLK_h, SDLK_j, SDLK_k,     SDLK_l,      SDLK_COLON, SDLK_SEMICOLON,SDLK_EQUALS,   SDLK_BACKSLASH, SDLK_F13,       SDLK_F5},
  {SDLK_z, SDLK_x, SDLK_c, SDLK_v, SDLK_b, SDLK_n, SDLK_m, SDLK_COMMA, SDLK_PERIOD, SDLK_SLASH, SDLK_UP,       SDLK_KP_ENTER, SDLK_F10,                       SDLK_F7},
  {SDLK_BREAK, SDLK_SPACE,                                                          SDLK_LEFT,  SDLK_DOWN,     SDLK_RIGHT,    SDLK_RSHIFT,    SDLK_F14}
};


char const *syms[4][NUM_ROWS][NUM_KEYS] = {
  
  {
    {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "+",      "-", "S:28", "HOM", "f1", NULL},
    {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "S:256",  "*", "S:31", "DEL", "f3", NULL},
    {"A", "S", "D", "F", "G", "H", "J", "K", "L", ":", ";",      "=", "S:30", "RES", "f5", NULL},
    {"Z", "X", "C", "V", "B", "N", "M", ",", ".", "/", "S:4096", "RET",       "CTL", "f7", NULL},
    {"R/S", "    SPACE    ",                 "S:4098", "S:4097", "S:4099","SHIFT",    "C=",NULL}
  },
  {
    {"!", "\"","#", "$", "%", "&", "'", "(", ")", "0", "S:91", "S:71", "S:105", "CLR", "f2", NULL},
    {"P:Q", "P:W", "P:E", "P:R", "P:T", "P:Y", "P:U",  "P:I",  "P:O",  "P:P",  "S:122", "P:C", "S:31", "INS", "f4", NULL},
    {"P:A", "P:S", "P:D", "P:F", "P:B", "P:H", "P:J",  "P:K",  "P:L",  "S:27", "S:29",  "=",   "S:94", "RES", "f6", NULL},
    {"P:Z", "P:X", "P:C", "P:V", "P:B", "P:N", "P:M",  "<",    ">",    "?",    "S:4096", "RET",        "CTL", "f8", NULL},
    {"R/S", "    SPACE    ",                                         "S:4098", "S:4097", "S:4099","SHIFT",     "C=",NULL}
  },

  {
    {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "+",      "-", "S:28", "HOM", "f1", NULL},
    {"q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "S:256",  "*", "S:31", "DEL", "f3", NULL},
    {"a", "s", "d", "f", "g", "h", "j", "k", "l", ":", ";",      "=", "S:30", "RES", "f5", NULL},
    {"z", "x", "c", "v", "b", "n", "m", ",", ".", "/", "S:4096", "RET",       "CTL", "f7", NULL},
    {"R/S", "    SPACE    "      ,           "S:4098", "S:4097", "S:4099","SHIFT", "C=",NULL}
  }, {
    {"!", "\"","#", "$", "%", "&", "'", "(", ")", "0",    "+",     "-", "S:28", "CLR", "f2", NULL},
    {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",    "S:256", "*", "S:31", "INS", "f4", NULL},
    {"A", "S", "D", "F", "G", "H", "J", "K", "L", "S:27", "S:29",  "=", "S:30", "RES", "f6", NULL},
    {"Z", "X", "C", "V", "B", "N", "M", "<", ">", "?",    "S:4096","RET",       "CTL", "f8", NULL},
    {"R/S", "    SPACE    ",                    "S:4098", "S:4097", "S:4099","SHIFT",   "C=",NULL}
  }
  
};

static unsigned char toggled[NUM_ROWS][NUM_KEYS];

static int selected_i = 0, selected_j = 0;
static int shifted = 0;
static int mod_state = 0;
static int text_mode = 0;
void init_keyboard() {
  for(int j = 0; j < NUM_ROWS; j++)
    for(int i = 0; i < NUM_KEYS; i++)
      toggled[j][i] = 0;
  selected_i = selected_j = shifted = 0;
  mod_state = 0;
}

int strlensym(char const *str) {
  if(strncmp(str,"S:",2) == 0) return 1;
  else if(strncmp(str,"P:",2) == 0) return 1;
  else return strlen(str);
}


void draw_test(SDL_Surface* surface) {
  unsigned short bg_color = SDL_MapRGB(surface->format, 180, 180, 180);
  unsigned short key_color = SDL_MapRGB(surface->format, 128, 128, 128);
  unsigned short text_color = SDL_MapRGB(surface->format, 0, 0, 0);
  unsigned short sel_color = SDL_MapRGB(surface->format, 128, 255, 128);
  unsigned short sel_toggled_color = SDL_MapRGB(surface->format, 255, 255, 128);
  unsigned short toggled_color = SDL_MapRGB(surface->format, 212, 212, 0);
  int length;
  int is_sym = 0;
  int i,j,sp = 0;
  
  SDL_Rect rect;
  rect.x = 0;
  rect.y = 0;
  rect.w = 2*320;
  rect.h = 2*240;
  SDL_FillRect(surface, &rect, bg_color);
  for(j=0; j<4; j++) {
    for(i=0; i<64; i++) {
      if((i%8)==7) sp+=5;
      draw_sym(surface,i+128*j, 9*i+sp, 18*j+9, text_color);
      draw_sym(surface,i+64+128*j, 9*i+sp, 18*j+18, text_color);
      
    }
    sp = 0;
    
  }
  draw_sym(surface,4*64+1, 100, 150, text_color);
}

void draw_keyboard(SDL_Surface* surface) {
  unsigned short bg_color = SDL_MapRGB(surface->format, 180, 180, 180);
  unsigned short key_color = SDL_MapRGB(surface->format, 128, 128, 128);
  unsigned short text_color = SDL_MapRGB(surface->format, 0, 0, 0);
  unsigned short sel_color = SDL_MapRGB(surface->format, 128, 255, 128);
  unsigned short sel_toggled_color = SDL_MapRGB(surface->format, 255, 255, 128);
  unsigned short toggled_color = SDL_MapRGB(surface->format, 212, 212, 0);
  int length;
  int is_sym = 0;
  
  int total_length = -1;
  for(int i = 0; i < NUM_KEYS && syms[0][0][i]; i++) {
    total_length += (1 + strlensym(syms[0][0][i])) * 6;
  }
  
  int center_x =4;
  int x = center_x, y = 3; //surface->h - 8 * (NUM_ROWS) - 16;
  
  SDL_Rect rect;
  rect.x = x - 4;
  rect.y = y - 3;
  rect.w = total_length + 3;
  rect.h = NUM_ROWS * 8 + 3;
  SDL_FillRect(surface, &rect, bg_color);
  
  for(int j = 0; j < NUM_ROWS; j++) {
    x = center_x;
    for(int i = 0; i < row_length[j]; i++) {
      if((strncmp(syms[shifted+text_mode][j][i],"P:",2) == 0) || 
	 (strncmp(syms[shifted+text_mode][j][i],"S:",2) == 0)) {
	length = 1;
	is_sym = 1;
      } else {
	length = strlensym(syms[shifted+text_mode][j][i]);
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
	char sym_char = 0;
	if (sscanf(syms[shifted+text_mode][j][i],"S:%u", &sym_num) > 0)
	  draw_sym(surface, sym_num, x, y, text_color);
	else if(sscanf(syms[shifted+text_mode][j][i],"P:%c", &sym_char) > 0)
	  draw_sym(surface, ((unsigned int) sym_char), x, y, text_color);
      } else {
	draw_string(surface, syms[shifted+text_mode][j][i], x, y, text_color);
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
	  //if(key == SDLK_LSHIFT) mod_state |= KMOD_LSHIFT;
	  //	else
	  //	  if(key == SDLK_RSHIFT) mod_state |= KMOD_RSHIFT;
	  //	  else
		    if(key == SDLK_LCTRL) mod_state |= KMOD_LCTRL;
		else if(key == SDLK_RCTRL) mod_state |= KMOD_RCTRL;
		else if(key == SDLK_LALT) mod_state |= KMOD_LALT;
		else if(key == SDLK_RALT) mod_state |= KMOD_RALT;
		else if(key == SDLK_LMETA) mod_state |= KMOD_LMETA;
		else if(key == SDLK_RMETA) mod_state |= KMOD_RMETA;
		//else if(key == SDLK_NUM) mod_state |= KMOD_NUM;
		else if(key == SDLK_CAPSLOCK) mod_state |= KMOD_CAPS;
		else if(key == SDLK_MODE) mod_state |= KMOD_MODE;
	} else if(state == STATE_UP) {
	  //if(key == SDLK_LSHIFT) mod_state &= ~KMOD_LSHIFT;
	  //	else

	  //if(key == SDLK_RSHIFT) mod_state &= ~KMOD_RSHIFT;
	  //		else
		  if(key == SDLK_LCTRL) mod_state &= ~KMOD_LCTRL;
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
	int result = 1;
	
	if((event->key.type == SDL_KEYUP ||
	    event->key.type == SDL_KEYDOWN)
	   &&
	   event->key.keysym.mod & KMOD_SYNTHETIC) {
	  return 3;
	}
	  
	if(event->key.type  == SDL_KEYDOWN &&
	   event->key.state == SDL_PRESSED) {

	  //UP
	  if(event->key.keysym.sym == KEY_UP && selected_j > 0) {
	    selected_i = compute_new_col(visual_offset, selected_j, selected_j - 1);
	    selected_j--;
	    //DOWN
	  } else if(event->key.keysym.sym == KEY_DOWN && selected_j < NUM_ROWS - 1) {
	    selected_i = compute_new_col(visual_offset, selected_j, selected_j + 1);
	    selected_j++;
	    //LEFT
	  } else if(event->key.keysym.sym == KEY_LEFT && selected_i > 0) {
	    selected_i--;
	    visual_offset = compute_visual_offset(selected_i, selected_j);
	    //RIGHT
	  } else if(event->key.keysym.sym == KEY_RIGHT && selected_i < row_length[selected_j] - 1) {
	    selected_i++;
	    visual_offset = compute_visual_offset(selected_i, selected_j);
	    //SHIFT
	  } else if(event->key.keysym.sym == KEY_SHIFT) {
	    shifted = 1;
	    toggled[4][5] = 1;
	    update_modstate(SDLK_LSHIFT, STATE_DOWN);
	    simulate_key(SDLK_LSHIFT, STATE_DOWN);
	    //BACKSPACE
	  } else if(event->key.keysym.sym == KEY_BACKSPACE) {
	    simulate_key(SDLK_BACKSPACE, STATE_TYPED);
	    result = 2;
	    //TOGGLE STATE
	  } else if(event->key.keysym.sym == KEY_TOGGLE) {
	    toggled[selected_j][selected_i] = 1 - toggled[selected_j][selected_i];
	    if(toggled[selected_j][selected_i]) {
	      simulate_key(keys[selected_j][selected_i], STATE_DOWN);
	      result = 2;
	    } else {
	      simulate_key(keys[selected_j][selected_i], STATE_UP);
	      result = 2;
	    }
	    if(selected_j == 4 && (selected_i == 5)) {
	      shifted = toggled[selected_j][selected_i];
	    }
	    //TYPED
	  } else if(event->key.keysym.sym == KEY_ENTER) {
	    int key = keys[selected_j][selected_i];
	    //CTRL
	    /*
	    if(mod_state & KMOD_CTRL) {
	      if (key >= 64 && key < 64 + 32) {
		simulate_key(key - 64, STATE_DOWN);
		result = 2;
	      }
	      else if (key >= 97 && key < 97 + 31) {
		simulate_key(key - 96, STATE_DOWN);
		result = 2;
	      }
	    } else 
	    */
	    simulate_key(key, STATE_TYPED);
	    result = 2;
	    if ((key == SDLK_F14) && shifted) {
	      if (text_mode > 0) text_mode = 0; else text_mode=2; //SHIFT + C=x
	      //	      printf("text mode = %d\n", text_mode);
	    }
	  }
	} else if(event->key.type == SDL_KEYUP || event->key.state == SDL_RELEASED) {
	  if(event->key.keysym.sym == KEY_SHIFT) {
	    shifted = 0;
	    toggled[4][5] = 0;
	    update_modstate(SDLK_LSHIFT, STATE_UP);
	    simulate_key(SDLK_LSHIFT, STATE_UP);
	  }
	}
	return result;
}

#ifdef TEST_KEYBOARD

int main() {
	SDL_Init( SDL_INIT_EVERYTHING );
	SDL_Surface* screen = SDL_SetVideoMode(320*2, 240 * 2, 16, SDL_DOUBLEBUF);
	SDL_Surface* buffer = SDL_CreateRGBSurface(SDL_SWSURFACE, 2*320, 2*240, 16, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
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
		//draw_keyboard(buffer);
		draw_test(buffer);
		SDL_LockSurface(buffer);
		for(int j = 0; j < buffer->h; j++) {
			for(int i = 0; i < buffer->w; i++) {
			  SDL_Rect rect;
			  rect.x = i;
			  rect.y = j;
			  rect.w = 1;
			  rect.h = 1;
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

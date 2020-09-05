/*
 *  Display.cpp - C64 graphics display, emulator window handling
 *
 *  Frodo (C) 1994-1997,2002-2005 Christian Bauer
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "sysdeps.h"

#include "Display.h"
#include "main.h"
#include "Prefs.h"
#include "C64.h"

#include "keyboard.h"
#include "main.h"
#include "Version.h"

#include "menu_main.h"

#include <png.h>
#include <SDL/SDL.h>

int sdl_save_png(SDL_Surface* my_surface, char* filename, int scale);

// LED states
enum {
  LED_OFF,	// LED off
  LED_ON,	// LED on (green)
  LED_ERROR_ON,	// LED blinking (red), currently on
  LED_ERROR_OFF	// LED blinking, currently off
};


#define USE_PEPTO_COLORS 1

#ifdef USE_PEPTO_COLORS

// C64 color palette
// Values based on measurements by Philip "Pepto" Timmermann <pepto@pepto.de>
// (see http://www.pepto.de/projects/colorvic/)
const uint8 palette_red[16] = {
  0x00, 0xff, 0x86, 0x4c, 0x88, 0x35, 0x20, 0xcf, 0x88, 0x40, 0xcb, 0x34, 0x68, 0x8b, 0x68, 0xa1
};

const uint8 palette_green[16] = {
  0x00, 0xff, 0x19, 0xc1, 0x17, 0xac, 0x07, 0xf2, 0x3e, 0x2a, 0x55, 0x34, 0x68, 0xff, 0x4a, 0xa1
};

const uint8 palette_blue[16] = {
  0x00, 0xff, 0x01, 0xe3, 0xbd, 0x0a, 0xc0, 0x2d, 0x00, 0x00, 0x37, 0x34, 0x68, 0x59, 0xff, 0xa1
};

#else

// C64 color palette (traditional Frodo colors)
const uint8 palette_red[16] = {
  0x00, 0xff, 0x99, 0x00, 0xcc, 0x44, 0x11, 0xff, 0xaa, 0x66, 0xff, 0x40, 0x80, 0x66, 0x77, 0xc0
};

const uint8 palette_green[16] = {
  0x00, 0xff, 0x00, 0xff, 0x00, 0xcc, 0x00, 0xdd, 0x55, 0x33, 0x66, 0x40, 0x80, 0xff, 0x77, 0xc0
};

const uint8 palette_blue[16] = {
  0x00, 0xff, 0x00, 0xcc, 0xcc, 0x44, 0x99, 0x00, 0x00, 0x00, 0x66, 0x40, 0x80, 0x66, 0xff, 0xc0
};

#endif


/*
 *  Update drive LED display (deferred until Update())
 */

void C64Display::UpdateLEDs(int l0, int l1, int l2, int l3)
{
  led_state[0] = l0;
  led_state[1] = l1;
  led_state[2] = l2;
  led_state[3] = l3;
}

// Display surface
static SDL_Surface *screen = 0;
static SDL_Surface *surf = 0;

static SDL_Surface *output_surf = 0;
static SDL_Surface* m_buffer = 0;
static SDL_Surface* kb_buffer = 0;
static SDL_mutex *screenLock = 0;

// Mode of Joystick emulation. 0 = none, 1 = Joyport 1, 2 = Joyport 2
static short joy_emu = 1;

// Keyboard
static bool tab_pressed = false;

// For LED error blinking
static C64Display *c64_disp;
static struct sigaction pulse_sa;
static itimerval pulse_tv;

// SDL joysticks
static SDL_Joystick *joy[2] = {NULL, NULL};

static Prefs DialogPrefs;

// Colors for speedometer/drive LEDs
enum 
{
  black = 0,
  white = 1,
  fill_gray = 16,
  shine_gray = 17,
  shadow_gray = 18,
  red = 19,
  green = 20,
  PALETTE_SIZE = 21
};

/*
  C64 keyboard matrix:

    Bit 7   6   5   4   3   2   1   0  b
a 0    CUD  F5  F3  F1  F7 CLR RET DEL
  1    SHL  E   S   Z   4   A   W   3
  2     X   T   F   C   6   D   R   5
  3     V   U   H   B   8   G   Y   7
  4     N   O   K   M   0   J   I   9
  5     ,   @   :   .   -   L   P   +
  6     /   ^   =  SHR HOM  ;   *   
  7    R/S  Q   C= SPC  2  CTL  <-  1
*/

#define MATRIX(a,b) (((a) << 3) | (b))

char *buffer;

const int width = 320;
const int height = 240;

bool keyboard_enable = false;
bool keyboard_pos = true;

static SDL_Thread *GUIthread = NULL;
static const int GUI_RETURN_INFO = (SDL_USEREVENT+1);
static bool doUpdate = true;

void update(int32 x, int32 y, int32 w, int32 h, bool forced)
{
  if ( !forced && !doUpdate ) // the HW surface is available
    return;
  SDL_UpdateRect(screen, x, y, w, h);
}

void update(bool forced)
{
  update( 0, 0, width, height, forced );
}

void update()
{
  update( 0, 0, width, height, false );
}



void screenlock() 
{
  while (SDL_mutexP(screenLock)==-1) 
    {
      SDL_Delay(20);
      fprintf(stderr, "Couldn't lock mutex\n");
    }
}

void screenunlock() 
{
  while (SDL_mutexV(screenLock)==-1) 
    {
      SDL_Delay(20);
      fprintf(stderr, "Couldn't unlock mutex\n");
    }
}

int init_graphics(void)
{
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "Couldn't initialize SDL (%s)\n", SDL_GetError());
    return 0;
  }

  screenLock = SDL_CreateMutex();
  buffer = new char[DISPLAY_X*DISPLAY_Y];
  screen = SDL_SetVideoMode(width, height, 8, SDL_DOUBLEBUF);
  SDL_ShowCursor(SDL_DISABLE);
  surf = screen;
  if (screen == NULL)  {
    fprintf(stderr, "SDL Couldn't set video mode to %d x %d\n", width, height);
    return 0;
  } else {
    m_buffer = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
    if (m_buffer == NULL)  {
      printf("Failed to allocate m_buffer");
      return 0;
    }
    init_menu(screen);

    kb_buffer = SDL_CreateRGBSurface(SDL_SWSURFACE, 200, 50, 16, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
    if (kb_buffer == NULL)  {
      printf("Failed to allocate kb_buffer");
      return 0;
    }

  }
  
  return 1;
}

C64Display::C64Display(C64 *the_c64) : TheC64(the_c64)
{
  quit_requested = false;
  speedometer_string[0] = 0;
  
  // LEDs off
  for (int i=0; i<4; i++)
    led_state[i] = old_led_state[i] = LED_OFF;
  
  // Start timer for LED error blinking
#ifdef STATUS_DISP
  c64_disp = this;
  pulse_sa.sa_handler = (void (*)(int))pulse_handler;
  pulse_sa.sa_flags = SA_RESTART;
  sigemptyset(&pulse_sa.sa_mask);
  sigaction(SIGALRM, &pulse_sa, NULL);
  pulse_tv.it_interval.tv_sec = 0;
  pulse_tv.it_interval.tv_usec = 400000;
  pulse_tv.it_value.tv_sec = 0;
  pulse_tv.it_value.tv_usec = 400000;
  setitimer(ITIMER_REAL, &pulse_tv, NULL);
#endif
  
}

C64Display::~C64Display()
{
  SDL_Quit();
  delete[] buffer;
}

void C64Display::NewPrefs(Prefs *prefs)
{
}

void C64Display::Update(void)
{
  static int display_timer = -1;
  static int prev_joy_emu = 1;

  if (prev_joy_emu != joy_emu) {
    display_timer = 60;
  }
  prev_joy_emu = joy_emu;
  
  screenlock();
  if (surf == NULL)
    return;
  int iOffsetX = (DISPLAY_X - surf->w) / 2;
  int iOffsetY = (DISPLAY_Y - surf->h) / 2;
  
#ifdef STATUS_DISP
  for (int j=0; j < surf->h - 17; j++)
#else
    for (int j=0; j < surf->h; j++)
#endif
      {
	memcpy(static_cast<char*>(surf->pixels)+surf->w*j, buffer+iOffsetX+DISPLAY_X*(j+iOffsetY), surf->w);
      }
  if (keyboard_enable) {
    draw_keyboard(kb_buffer);
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = 200;
    rect.h = 42;
    
    SDL_Rect drect;
    drect.x = 60;
    if(keyboard_pos)
      drect.y = 197;
    else
      drect.y = 2;
    SDL_BlitSurface(kb_buffer, &rect, screen, &drect);
  }
	
#ifdef STATUS_DISP
  // Draw speedometer/LEDs
  SDL_Rect r = {0, (surf->h - 17), DISPLAY_X, 15};
  SDL_FillRect(surf, &r, fill_gray);
  r.w = DISPLAY_X; r.h = 1;
  SDL_FillRect(surf, &r, shine_gray);
  r.y = (surf->h - 17) + 14;
  SDL_FillRect(surf, &r, shadow_gray);
  r.w = 16;
  for (int i=2; i<5; i++) 
    {
      r.x = DISPLAY_X * i/5 - 24; r.y = (surf->h - 17) + 4;
      SDL_FillRect(surf, &r, shadow_gray);
      r.y = (surf->h - 17) + 10;
      SDL_FillRect(surf, &r, shine_gray);
    }
  r.y = (surf->h - 17); r.w = 1; r.h = 15;
  for (int i=0; i<4; i++) 
    {
      r.x = DISPLAY_X * i / 5;
      SDL_FillRect(surf, &r, shine_gray);
      r.x = DISPLAY_X * (i+1) / 5 - 1;
      SDL_FillRect(surf, &r, shadow_gray);
    }
  r.y = (surf->h - 17) + 4; r.h = 7;
  for (int i=2; i<5; i++)
    {
      r.x = DISPLAY_X * i/5 - 24;
      SDL_FillRect(surf, &r, shadow_gray);
      r.x = DISPLAY_X * i/5 - 9;
      SDL_FillRect(surf, &r, shine_gray);
    }
  r.y = (surf->h - 17) + 5; r.w = 14; r.h = 5;
  for (int i=0; i<3; i++) 
    {
      r.x = DISPLAY_X * (i+2) / 5 - 23;
      int c;
      switch (led_state[i]) 
	{
	case LED_ON:
	  c = green;
	  break;
	case LED_ERROR_ON:
	  c = red;
	  break;
	default:
	  c = black;
	  break;
	}
      SDL_FillRect(surf, &r, c);
    }

  draw_string(surf, DISPLAY_X * 1/5 + 8, (surf->h - 17) + 4, "D\x12 8", black, fill_gray);
  draw_string(surf, DISPLAY_X * 2/5 + 8, (surf->h - 17) + 4, "D\x12 9", black, fill_gray);
  draw_string(surf, DISPLAY_X * 3/5 + 8, (surf->h - 17) + 4, "D\x12 10", black, fill_gray);
#endif
  if ((joy_emu == 1) && (display_timer > 0)) {
    draw_string(surf, 240, (surf->h - 17) + 4, "JOY 1", black, fill_gray);
    display_timer--;
  }
  else if ((joy_emu == 2) && (display_timer > 0)) {
    display_timer--;
    draw_string(surf, 240, (surf->h - 17) + 4, "JOY 2", black, fill_gray);
  }

  //draw_string(surf, 24, (surf->h - 17) + 4, speedometer_string, black, fill_gray);


  // Update display
  SDL_Flip(surf);


  screenunlock();
}

void C64Display::draw_string(SDL_Surface *s, int x, int y, const char *str, uint8 front_color, uint8 back_color)
{
  uint8 *pb = (uint8 *)s->pixels + s->pitch*y + x;
  char c;
  while ((c = *str++) != 0) 
    {
      uint8 *q = TheC64->Char + c*8 + 0x800;
      uint8 *p = pb;
      for (int y=0; y<8; y++) 
	{
	  uint8 v = *q++;
	  p[0] = (v & 0x80) ? front_color : back_color;
	  p[1] = (v & 0x40) ? front_color : back_color;
	  p[2] = (v & 0x20) ? front_color : back_color;
	  p[3] = (v & 0x10) ? front_color : back_color;
	  p[4] = (v & 0x08) ? front_color : back_color;
	  p[5] = (v & 0x04) ? front_color : back_color;
	  p[6] = (v & 0x02) ? front_color : back_color;
	  p[7] = (v & 0x01) ? front_color : back_color;
	  p += s->pitch;
	}
      pb += 8;
    }
}


/*
 *  LED error blink
 */

void C64Display::pulse_handler(...)
{
  for (int i=0; i<4; i++)
    switch (c64_disp->led_state[i]) 
      {
      case LED_ERROR_ON:
	c64_disp->led_state[i] = LED_ERROR_OFF;
	break;
      case LED_ERROR_OFF:
	c64_disp->led_state[i] = LED_ERROR_ON;
	break;
      }
}


/*
 *  Draw speedometer
 */

void C64Display::Speedometer(int speed)
{
  static int delay = 0;
  
  if (delay >= 20)
    {
      delay = 0;
      sprintf(speedometer_string, "%d%%", speed);
    } 
  else
    delay++;
}


/*
 *  Return pointer to bitmap data
 */
uint8 *C64Display::BitmapBase(void)
{
  return (uint8 *)buffer;
}


/*
 *  Return number of bytes per row
 */
int C64Display::BitmapXMod(void)
{
	//return screen->pitch;
	return DISPLAY_X;
}



/*
 *  Poll the keyboard
 */

static void translate_key(SDLKey key, bool key_up, uint8 *key_matrix, uint8 *rev_matrix, uint8 *joystick)
{
  int c64_key = -1;
  //printf("key = %d\n", (unsigned int) key);
  if (keyboard_enable) {
    switch (key)
      {
      case SDLK_a: c64_key = MATRIX(1,2); break;
      case SDLK_b: c64_key = MATRIX(3,4); break;
      case SDLK_c: c64_key = MATRIX(2,4); break;
      case SDLK_d: c64_key = MATRIX(2,2); break;
      case SDLK_e: c64_key = MATRIX(1,6); break;
      case SDLK_f: c64_key = MATRIX(2,5); break;
      case SDLK_g: c64_key = MATRIX(3,2); break;
      case SDLK_h: c64_key = MATRIX(3,5); break;
      case SDLK_i: c64_key = MATRIX(4,1); break;
      case SDLK_j: c64_key = MATRIX(4,2); break;
      case SDLK_k: c64_key = MATRIX(4,5); break;
      case SDLK_l: c64_key = MATRIX(5,2); break;
      case SDLK_m: c64_key = MATRIX(4,4); break;
      case SDLK_n: c64_key = MATRIX(4,7); break;
      case SDLK_o: c64_key = MATRIX(4,6); break;
      case SDLK_p: c64_key = MATRIX(5,1); break;
      case SDLK_q: c64_key = MATRIX(7,6); break;
      case SDLK_r: c64_key = MATRIX(2,1); break;
      case SDLK_s: c64_key = MATRIX(1,5); break;
      case SDLK_t: c64_key = MATRIX(2,6); break;
      case SDLK_u: c64_key = MATRIX(3,6); break;
      case SDLK_v: c64_key = MATRIX(3,7); break;
      case SDLK_w: c64_key = MATRIX(1,1); break;
      case SDLK_x: c64_key = MATRIX(2,7); break;
      case SDLK_y: c64_key = MATRIX(3,1); break;
      case SDLK_z: c64_key = MATRIX(1,4); break;
	
      case SDLK_1: c64_key = MATRIX(7,0); break;
      case SDLK_2: c64_key = MATRIX(7,3); break;
      case SDLK_3: c64_key = MATRIX(1,0); break;
      case SDLK_4: c64_key = MATRIX(1,3); break;
      case SDLK_5: c64_key = MATRIX(2,0); break;
      case SDLK_6: c64_key = MATRIX(2,3); break;
      case SDLK_7: c64_key = MATRIX(3,0); break;
      case SDLK_8: c64_key = MATRIX(3,3); break;
      case SDLK_9: c64_key = MATRIX(4,0); break;
      case SDLK_0: c64_key = MATRIX(4,3); break;
	
      case SDLK_EXCLAIM:    c64_key = MATRIX(7,0)|0x80; break;
      case SDLK_QUOTEDBL:   c64_key = MATRIX(7,3)|0x80; break;
      case SDLK_HASH:       c64_key = MATRIX(1,0)|0x80; break;
      case SDLK_DOLLAR:     c64_key = MATRIX(1,3)|0x80; break;
      case SDLK_F15:        c64_key = MATRIX(2,0)|0x80; break; //%
      case SDLK_AMPERSAND:  c64_key = MATRIX(2,3); break;
      case SDLK_QUOTE:      c64_key = MATRIX(3,0); break;
      case SDLK_LEFTPAREN:  c64_key = MATRIX(3,3); break;
      case SDLK_RIGHTPAREN: c64_key = MATRIX(4,0); break;
	      
      case SDLK_SPACE:        c64_key = MATRIX(7,4); break;
      case SDLK_BACKQUOTE:    c64_key = MATRIX(7,1); break;
      case SDLK_BACKSLASH:    c64_key = MATRIX(6,6); break;
      case SDLK_COMMA:        c64_key = MATRIX(5,7); break;
      case SDLK_PERIOD:       c64_key = MATRIX(5,4); break;
      case SDLK_MINUS:        c64_key = MATRIX(5,3); break;
      case SDLK_PLUS:         c64_key = MATRIX(5,0); break;
      case SDLK_EQUALS:       c64_key = MATRIX(6,5); break;
      case SDLK_LEFTBRACKET:  c64_key = MATRIX(5,6); break;
      case SDLK_RIGHTBRACKET: c64_key = MATRIX(6,1); break;
      case SDLK_SEMICOLON:    c64_key = MATRIX(6,2); break;
      case SDLK_SLASH:        c64_key = MATRIX(6,7); break;
	
      case SDLK_BREAK:        c64_key = MATRIX(7,7); break;
      case SDLK_RETURN:       c64_key = MATRIX(0,1); break;
      case SDLK_BACKSPACE:    c64_key = MATRIX(0,0); break;
      case SDLK_PRINT:        c64_key = MATRIX(6,3); break;
      case SDLK_END:          c64_key = MATRIX(6,0); break;
      case SDLK_PAGEUP:       c64_key = MATRIX(6,0); break;
      case SDLK_PAGEDOWN:     c64_key = MATRIX(6,5); break;

      case SDLK_F10:    c64_key = MATRIX(7,2); break;
      case SDLK_F14:    c64_key = MATRIX(7,5); break;
      case SDLK_LSHIFT: c64_key = MATRIX(1,7); break;
      case SDLK_RSHIFT: c64_key = MATRIX(6,4); break;
      case SDLK_RALT: case SDLK_RMETA: c64_key = MATRIX(7,5); break;
	      
      case SDLK_UP:    c64_key = MATRIX(0,7)|0x80; break;
      case SDLK_DOWN:  c64_key = MATRIX(0,7); break;
      case SDLK_LEFT:  c64_key = MATRIX(0,2)|0x80; break;
      case SDLK_RIGHT: c64_key = MATRIX(0,2); break;
	      
      case SDLK_F1: c64_key = MATRIX(0,4); break;
      case SDLK_F2: c64_key = MATRIX(0,4)|0x80; break;
      case SDLK_F3: c64_key = MATRIX(0,5); break;
      case SDLK_F4: c64_key = MATRIX(0,5)|0x80; break;
      case SDLK_F5: c64_key = MATRIX(0,6); break;
      case SDLK_F6: c64_key = MATRIX(0,6)|0x80; break;
      case SDLK_F7: c64_key = MATRIX(0,3); break;
      case SDLK_F8: c64_key = MATRIX(0,3)|0x80; break;

      case SDLK_KP_DIVIDE: c64_key = MATRIX(6,7); break;
      case SDLK_KP_ENTER:  c64_key = MATRIX(0,1); break;
      case SDLK_ASTERISK:  c64_key = MATRIX(6,1); break;
      case SDLK_COLON:     c64_key = MATRIX(5,5); break;
      case SDLK_AT:        c64_key = MATRIX(5,6); break;
      }

    if (c64_key < 0) {
      return;
    }
  }

  if (keyboard_enable == false) {
    switch (key)
      {
	
      case SDLK_LALT:  c64_key = 0x10 | 0x40; break; //fire
      case SDLK_LCTRL: c64_key = 0x01 | 0x40; break; //up
	
      case SDLK_UP:    c64_key = 0x01 | 0x40; break;
      case SDLK_DOWN:  c64_key = 0x02 | 0x40; break;
      case SDLK_LEFT:  c64_key = 0x04 | 0x40; break;
      case SDLK_RIGHT: c64_key = 0x08 | 0x40; break;
	
      default: return; //No action on unmapped keys
      }
	
    // Handle joystick emulation
    if (c64_key & 0x40) 
      {
	c64_key &= 0x1f;
	if (key_up)
	  *joystick |= c64_key;
	else
	  *joystick &= ~c64_key;
	return;
      }
  }

  bool shifted = c64_key & 0x80;
  int c64_byte = (c64_key >> 3) & 7;
  int c64_bit = c64_key & 7;
  if (key_up) 
    {
      if (shifted) 
	{
	  key_matrix[6] |= 0x10;
	  rev_matrix[4] |= 0x40;
	}
      key_matrix[c64_byte] |= (1 << c64_bit);
      rev_matrix[c64_bit] |= (1 << c64_byte);
    } 
  else 
    {
      if (shifted)
	{
	  key_matrix[6] &= 0xef;
	  rev_matrix[4] &= 0xbf;
	}
      key_matrix[c64_byte] &= ~(1 << c64_bit);
      rev_matrix[c64_bit] &= ~(1 << c64_byte);
    }
}

void C64Display::PollKeyboard(uint8 *key_matrix, uint8 *rev_matrix, uint8 *joystick)
{

  SDL_Event event;
  int menu_status = 0;
  static bool cmd_key_up = false;
  static unsigned int cmd_pos = 0;
  static unsigned int start_delay = 180;
  char cmd_buffer[] = "load\"*\",8\r:run\r";
  int eventmask = SDL_EVENTMASK(SDL_KEYDOWN)
    | SDL_EVENTMASK(SDL_KEYUP)
    | SDL_EVENTMASK(SDL_MOUSEBUTTONDOWN)
    | SDL_EVENTMASK(SDL_MOUSEBUTTONUP)
    | SDL_EVENTMASK(SDL_MOUSEMOTION)
    | SDL_EVENTMASK(SDL_JOYAXISMOTION)
    | SDL_EVENTMASK(SDL_JOYHATMOTION)
    | SDL_EVENTMASK(SDL_JOYBUTTONDOWN)
    | SDL_EVENTMASK(SDL_JOYBUTTONUP)
    | SDL_EVENTMASK(SDL_ACTIVEEVENT)
    | SDL_EVENTMASK(GUI_RETURN_INFO)
    | SDL_EVENTMASK(SDL_QUIT);
  if (start_delay>0) start_delay--; 


  if((cmd_pos < sizeof(cmd_buffer)) && (start_delay == 0) && (TheApp->AutoRunEnabled()))
    {
      keyboard_enable = true;
      translate_key((SDLKey)cmd_buffer[cmd_pos], cmd_key_up, key_matrix, rev_matrix, joystick);
      keyboard_enable = false;
      if (cmd_key_up)
	{
	  cmd_key_up = false;
	  cmd_pos++;
	}
      else
	{
	  cmd_key_up = true;
	}
      return;
    }
	  
  SDL_PumpEvents();
  while (SDL_PeepEvents(&event, 1, SDL_GETEVENT, eventmask)) {
      
      if (keyboard_enable) {
	handle_keyboard_event(&event);
	SDL_PumpEvents();
      }
      
      switch (event.type) {
	
      case SDL_KEYDOWN:
	switch (event.key.keysym.sym) {
	  
	case SDLK_ESCAPE: //SELECT
	  if (joy_emu < 2)
	    joy_emu++;
	  else
	    joy_emu = 1;
	  break;
	  
	case SDLK_RCTRL: //R
	  // Copy surface before opening menu to make screenshot possible
	  output_surf = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
	  SDL_Rect brect;
	  brect.x = 0;
	  brect.y = 0;
	  brect.w = 320;
	  brect.h = 240;
	  SDL_BlitSurface(screen, &brect, output_surf, &brect);
	    
	  SDL_PauseAudio(1);
	  DialogPrefs = ThePrefs;
	  menu_status = start_menu(m_buffer, screen, DialogPrefs.DrivePath);
	  TheC64->NewPrefs(&DialogPrefs);
	  
	  switch (menu_status) {
	  case MENU_RESET:
	    TheC64->Reset();
	    break;

	    
	    //Toggle speed limiter
	    //ThePrefs.LimitSpeed = !ThePrefs.LimitSpeed;

	    //Toggel true 1541 emulation
	    //ThePrefs.Emul1541Proc = true/false;

	    //Memory Expansion Size
	    //ThePrefs.REUSize  "None"  "128K" "256K" "512K"

	    //ThePrefs.SkipFrames;
	     
	  case MENU_QUIT:
	    quit_requested = true;
	    break;

	  case MENU_SAVE_SNAP1: TheC64->SaveSnapshot((char *)"snapshot1.ss"); sdl_save_png(output_surf, (char *)"snapshot1.png", 2); break;
	  case MENU_LOAD_SNAP1: TheC64->LoadSnapshot((char *)"snapshot1.ss"); break;
	  case MENU_SAVE_SNAP2: TheC64->SaveSnapshot((char *)"snapshot2.ss"); sdl_save_png(output_surf, (char *)"snapshot2.png", 2); break;
	  case MENU_LOAD_SNAP2: TheC64->LoadSnapshot((char *)"snapshot2.ss"); break;
	  case MENU_SAVE_SNAP3: TheC64->SaveSnapshot((char *)"snapshot3.ss"); sdl_save_png(output_surf, (char *)"snapshot3.png", 2); break;
	  case MENU_LOAD_SNAP3: TheC64->LoadSnapshot((char *)"snapshot3.ss"); break;
	  case MENU_SAVE_SNAP4: TheC64->SaveSnapshot((char *)"snapshot4.ss"); sdl_save_png(output_surf, (char *)"snapshot4.png", 2); break;
	  case MENU_LOAD_SNAP4: TheC64->LoadSnapshot((char *)"snapshot4.ss"); break;

	  case MENU_SCREENSHOT:
	    sdl_save_png(output_surf, (char *)"screen.png", 1);
	    break;
	    
	    //case SAVEPREFS:
	    //ThePrefs.Save(Frodo::get_prefs_path());
	    
	  }
	  SDL_PauseAudio(0);
	  break;
	  
	case SDLK_RETURN: //START
	  if (keyboard_enable && keyboard_pos) {
	    keyboard_pos = false;
	  } else {
	    keyboard_enable = !keyboard_enable;
	    keyboard_pos = true;
	  }
	  break;
	  
	case SDLK_F13:
	  TheC64->NMI(); //NMI (Restore Key)
	  break;
	  
	case SDLK_F12:
	  TheC64->Reset();
	  break;
	  
	default:
	  if ((event.key.keysym.mod & KMOD_SYNTHETIC)||!keyboard_enable) {
	    translate_key(event.key.keysym.sym, false, key_matrix, rev_matrix, joystick);
	    return;
	  }
	}
	break;
	      
	      
      case SDL_KEYUP:
	if ((event.key.keysym.mod & KMOD_SYNTHETIC)||!keyboard_enable) {
	  translate_key(event.key.keysym.sym, true, key_matrix, rev_matrix, joystick);
	  return;
	}
	break;
	      
      case SDL_QUIT:
	quit_requested = true;
	break;
      }
  }
}


bool C64Display::JoyStick1(void)
{
  if (joy_emu == 2)
    return true;
  return false;
}


/*
 *  Allocate C64 colors
 */

void C64Display::InitColors(uint8 *colors)
{
  SDL_Color palette[PALETTE_SIZE];
  for (int i=0; i<16; i++)
    {
      palette[i].r = palette_red[i];
      palette[i].g = palette_green[i];
      palette[i].b = palette_blue[i];
    }
  palette[fill_gray].r = palette[fill_gray].g = palette[fill_gray].b = 0xd0;
  palette[shine_gray].r = palette[shine_gray].g = palette[shine_gray].b = 0xf0;
  palette[shadow_gray].r = palette[shadow_gray].g = palette[shadow_gray].b = 0x80;
  palette[red].r = 0xf0;
  palette[red].g = palette[red].b = 0;
  palette[green].g = 0xf0;
  palette[green].r = palette[green].b = 0;
  SDL_SetColors(screen, palette, 0, PALETTE_SIZE);
  
  for (int i=0; i<256; i++)
    colors[i] = i & 0x0f;
}


/*
 *  Show a requester (error message)
 */

long int ShowRequester(const char *a, const char *b, const char *)
{
  printf("%s: %s\n", a, b);
  return 1;
}
 

#define  systemRedShift      (output_surf->format->Rshift)
#define  systemGreenShift    (output_surf->format->Gshift)
#define  systemBlueShift     (output_surf->format->Bshift)
#define  systemRedMask       (output_surf->format->Rmask)
#define  systemGreenMask     (output_surf->format->Gmask)
#define  systemBlueMask      (output_surf->format->Bmask)

int sdl_save_png(SDL_Surface* my_surface, char* filename, int scale)
{
  int w = my_surface->w/scale;
  int h = my_surface->h/scale;
  uint8* pix = (uint8*)my_surface->pixels;
  uint8 writeBuffer[512 * 3];
  
  FILE *fp = fopen(filename,"wb");
  
  if(!fp) return 0;
  
  png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                                NULL,
                                                NULL,
                                                NULL);
  if(!png_ptr) {
    fclose(fp);
    return 0;
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);

  if(!info_ptr) {
    png_destroy_write_struct(&png_ptr,NULL);
    fclose(fp);
    return 0;
  }

  png_init_io(png_ptr,fp);

  png_set_IHDR(png_ptr,
               info_ptr,
               w,
               h,
               8,
               PNG_COLOR_TYPE_RGB,
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

  png_write_info(png_ptr,info_ptr);

  uint8 *b = writeBuffer;

  int sizeX = w;
  int sizeY = h;
  int y;
  int x;

  uint16 *p = (uint16 *)pix;
  for(y = 0; y < sizeY*scale; y+=scale) {
     for(x = 0; x < sizeX*scale; x+=scale) {
       uint16 v = p[x];
      *b++ = ((v & systemRedMask  ) >> systemRedShift  ) << 3; // R
      *b++ = ((v & systemGreenMask) >> systemGreenShift) << 2; // G
      *b++ = ((v & systemBlueMask ) >> systemBlueShift ) << 3; // B
    }
     p += scale*my_surface->pitch / 2;
    png_write_row(png_ptr,writeBuffer);

    b = writeBuffer;
  }

  png_write_end(png_ptr, info_ptr);

  png_destroy_write_struct(&png_ptr, &info_ptr);

  fclose(fp);
  return 1;
}

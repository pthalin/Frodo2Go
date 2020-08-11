#include "sysdeps.h"
#include <SDL/SDL.h>
#include "font.h"
#define CHAR_ROM_SIZE 4096
#include "Char_ROM.h"

void draw_sym(SDL_Surface* surface, unsigned int symbol, int x, int y, unsigned short color) {
  x += (8 - 1) * 1;
  const unsigned char* ptr = builtin_char_rom + symbol * 8;
  int flip = 0;
  for(int i = 0, ys = 0; i < 7; i++, ptr++, ys += 1) //ROW 
    for(int col = 0, xs = x - col -1; col < 8; col++, xs -= 1)
      if((*ptr & 1 << col) && y + ys < surface->h && xs < surface->w ) {
	((unsigned short*)surface->pixels)[(y-1 + flip * 4 + (1 - 2 * flip) * ys) * (surface->pitch >> 1) + xs] = color;
      }
}

void draw_char(SDL_Surface* surface, unsigned char symbol, int x, int y, unsigned short color) {
  x += (8 - 1) * 1;
  int flip = 0;
  unsigned int s;
  int move_up = 0;
  switch(symbol) {
  case ',':
  case '.':
    move_up = 2;
    break;
  case ':':
  case ';':
    move_up = 1;
  }
  if(symbol >= 97) {
    s=symbol-96+(2*8*16); //lowe case
    move_up = 1;
  } else {
    s=symbol+(16*16); //UPPER CASE
  }
  const unsigned char* ptr = builtin_char_rom + s * 8;

  for(int i = 0, ys = 0; i < 8; i++, ptr++, ys += 1) //ROW 
    for(int col = 0, xs = x - col -1; col < 8; col++, xs -= 1)
      if((*ptr & 1 << col) && y + ys < surface->h && xs < surface->w ) {
	((unsigned short*)surface->pixels)[(y-1 - move_up + flip * 4 + (1 - 2 * flip) * ys) * (surface->pitch >> 1) + xs] = color;
      }
}

void draw_string(SDL_Surface* surface, const char* text, int orig_x, int orig_y, unsigned short color) {
  int x = orig_x, y = orig_y;
  while(*text) {
    if(*text == '\n') {
      x = orig_x;
      y += 8;
    } else {
      draw_char(surface, *text, x, y, color);
      x += 6;
    }
    text++;
  }
}

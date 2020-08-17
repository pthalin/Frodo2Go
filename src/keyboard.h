#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__
#include <SDL/SDL.h>
#ifdef __cplusplus
extern "C"
{
#endif

#define KMOD_SYNTHETIC (1 << 13)
  
  void init_keyboard();
  void draw_keyboard(SDL_Surface* surface);
  int handle_keyboard_event(SDL_Event* event);
  
#ifdef __cplusplus
}
#endif
#endif //__KEYBOARD_H__

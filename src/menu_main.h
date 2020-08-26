#ifndef __MENU_MAIN_H__
#define __MENU_MAIN_H__
#include <SDL/SDL.h>
#ifdef __cplusplus
extern "C"
{
#endif
  void init_menu(SDL_Surface* surface);
  void start_menu(SDL_Surface *buffer, SDL_Surface *screen);
  
  
#ifdef __cplusplus
}
#endif
#endif //__MENU_MAIN_H__

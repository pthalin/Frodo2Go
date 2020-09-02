#ifndef __MENU_MAIN_H__
#define __MENU_MAIN_H__
#include <SDL/SDL.h>
#ifdef __cplusplus
extern "C"
{
#endif
  enum {
    MENU_VOID,
    MENU_QUIT,
    MENU_RESET,
    MENU_SAVE_SNAP1,
    MENU_LOAD_SNAP1,
    MENU_SAVE_SNAP2,
    MENU_LOAD_SNAP2,
    MENU_SAVE_SNAP3,
    MENU_LOAD_SNAP3,
    MENU_SAVE_SNAP4,
    MENU_LOAD_SNAP4,
    MENU_SCREENSHOT
 };

  void init_menu(SDL_Surface* surface);
  int start_menu(SDL_Surface *buffer, SDL_Surface *screen, char (*drive_path)[256]);
  
#ifdef __cplusplus
}
#endif
#endif //__MENU_MAIN_H__

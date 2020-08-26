#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <SDL/SDL.h>
#include "font.h"
#include "menu_file.h"

char const  *menu_items[] =
  {
    "M:Main",   "Drives", "Reset", "Exit",  NULL,
    "M:Drives", "Drive 8", "Drive 9", "Drive 10", "Drive 11",NULL,
  };

enum menu_action_t {
  M_MAIN,   DRIVES,  RESET,   EXIT, N_MAIN,
  M_DRIVES, DRIVE8,  DRIVE9,  DRIVE10,  DRIVE11, N_DRIVES,
  FILE_REQUEST,
  NO_ACTION
};

char tmp[512];
int msg_active = 0;

unsigned short text_color;
unsigned short sel_color;
unsigned short warning_color;


int menu_size = sizeof(menu_items)/sizeof(menu_items[0]);

int menu_len[32];

enum menu_action_t menu_action;
int prev_menu = 0;
int current_menu = M_MAIN;
int menu_refresh = 1;

int current_drive = 0;

void init_menu(SDL_Surface* surface) {

  menu_action = M_MAIN;

  text_color = SDL_MapRGB(surface->format, 200, 200, 200);
  sel_color = SDL_MapRGB(surface->format, 128, 255, 128);
  warning_color = SDL_MapRGB(surface->format, 255, 255, 0);
  
  // Pre calculate nuber of items
  for (int k = 0; k < menu_size; k++) {
    if (NULL != menu_items[k])
      if(strncmp(menu_items[k],"M:",2) == 0) {
	//printf("Menu:%s\n", menu_items[k]+2);
	int cnt = 0;
	while (menu_items[k+cnt]) {
	  cnt++;
	}
	//printf("N=%d\n", cnt-1);
	menu_len[k] = cnt-1; //Exclude NULL
      }
  }
  
}



int display_menu(SDL_Surface* surface,  int menu_id, int *selected) {
  SDL_Rect rect;
  rect.x = 0;
  rect.y = 0;
  rect.w = 320;
  if (msg_active == 0)
    rect.h = 240;
  else
    rect.h = 120;
  
  SDL_FillRect(surface, &rect, 0);
  
  draw_string_osd(surface, menu_items[menu_id]+2, 10, 10, text_color);

  while(*selected < 0)
    *selected = *selected + menu_len[menu_id]; //Wrap up if ative
  
  *selected = *selected % menu_len[menu_id]; //Wrap overflow 
  int x = 20;
  int j = 1;
  while (menu_items[menu_id+j]) {
    if ((*selected+1) == j) {
      draw_string_osd(surface, menu_items[menu_id+j], 12, x, sel_color);
    }
    else {
      draw_string_osd(surface, menu_items[menu_id+j], 12, x, text_color);
    }
    x+=10;
    j++;
  }
  return menu_id + *selected + 1;
}


void menu_function(int action) {

  switch(action){
  case EXIT:
    //printf("Exit\n");
    exit(0);
    break;
  case DRIVES:
    //printf("Drives\n");
    prev_menu = current_menu;
    current_menu = M_DRIVES;
    menu_refresh = 1;
    break;
  case DRIVE8:
  case DRIVE9:
  case DRIVE10:
  case DRIVE11:
    //printf("File sel\n");

    current_drive = action-DRIVE8;
    //printf("Drive idx = %d\n", current_drive);
    prev_menu = current_menu;
    current_menu = FILE_REQUEST;
    break;
  }
}
  


void start_menu(SDL_Surface *buffer, SDL_Surface *screen) {
  long keysym = -1;
  SDL_Event event;
  char myfile[512] = "";
  char mypath[512] = "/home/";
  char *filename;
  int action = NO_ACTION;
  int selected = 0;
  
  SDL_LockSurface(buffer);
  menu_refresh = 1;
  
  while(1) {
    int key_press = 0;
    if (menu_refresh) {
      menu_refresh = 0;
    } else {
      while(key_press == 0) {
	while(SDL_PollEvent(&event)) {
	  switch(event.type) {
	  case SDL_KEYDOWN:
	    keysym = event.key.keysym.sym;
	    key_press = 1;
	    break;
	    
	  case SDL_QUIT: 
	    printf("Quit requested, quitting.\n");
	    exit(0);
	    break;
	  }
	}
      }
      
      switch(keysym) {
      case SDLK_UP:
	selected--;
	break;
      case SDLK_DOWN:
	selected++;
	break;
      case SDLK_LCTRL:
      case SDLK_LEFT:
	current_menu = prev_menu;
	selected = 0;
	break;
      case SDLK_LALT:
      case SDLK_RIGHT:
	menu_function(action);
	break;
      }
    }
   
  
    if (current_menu == FILE_REQUEST) {
      int valid = menu_file_request(screen, myfile, mypath);
      //printf("valind = %d\n", valid);
      //printf("file = %s\n", myfile);
      //printf("path = %s\n", mypath);
      filename = basename(myfile);

      SDL_Rect rect;
      rect.x = 0;
      rect.y = 120;
      rect.w = 320;
      rect.h = 120;
  
      SDL_FillRect(buffer, &rect, 0);

      if (valid == 1) { 
	sprintf(tmp, "Success: Drive %d set to %s", current_drive+8, filename);
	draw_string_osd(buffer, tmp, 10, 140, sel_color);
      } else {
	sprintf(tmp, "Warning: Drive %d was not set!", current_drive+8);
	draw_string_osd(buffer, tmp, 10, 140, warning_color);
      }
     
      
      msg_active = 2;
      current_menu = M_DRIVES;
      prev_menu = M_MAIN;
      menu_refresh = 1;
      action = NO_ACTION;
    } else {
      action = display_menu(buffer, current_menu, &selected);
    }
    
    //  printf("Selected=%d\n", selected);
    

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
    SDL_Delay(100);

    //printf("msg_active = %d\n", msg_active);
    if (msg_active > 0)
      msg_active--;
    
  }
  

}

#ifdef TEST_KEYBOARD
int main() {
  SDL_Init( SDL_INIT_EVERYTHING );

  
  SDL_Surface* screen = SDL_SetVideoMode(320, 240, 16, SDL_DOUBLEBUF);
  SDL_Surface* buffer = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);

  init_menu(buffer); 

  start_menu(buffer, screen);
}
#endif



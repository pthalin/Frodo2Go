#include <stdio.h>
#include <string.h>
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
  N_ACTIONS
};


int menu_size = sizeof(menu_items)/sizeof(menu_items[0]);

int menu_len[16];

enum menu_action_t menu_action;

void init_menu() {
  int i = 0;
  menu_action = M_MAIN;

  // Pre calculate nuber of items
  for (int k = 0; k < menu_size; k++) {
    if (NULL != menu_items[k])
      if(strncmp(menu_items[k],"M:",2) == 0) {
	//printf("Menu:%s\n", menu_items[k]+2);
	int cnt = 0;
	while (menu_items[k+cnt]) {
	  cnt++;
	}
	printf("N=%d\n", cnt);
	menu_len[i++] = cnt-1; //Exclude NULL
      }
  }
}



int display_menu(SDL_Surface* surface, const char *menu_name, int *selected) {
  unsigned short text_color = SDL_MapRGB(surface->format, 200, 200, 200);
  unsigned short sel_color = SDL_MapRGB(surface->format, 128, 255, 128);
  int k;
  int menu_id = -1;

  
  for (k = 0; k < menu_size; k++) {
    //printf("k=%d \n", k);
    if (NULL != menu_items[k])
      if(strncmp(menu_items[k],"M:",2) == 0) {
	menu_id++;
	if(strcmp(menu_items[k]+2,menu_name) == 0) {
	  //printf("Menu:%s\n", menu_items[k]+2);
	  draw_string_osd(surface, menu_items[k]+2, 10, 10, text_color);
	  k++;
	  break;
	}
      }
  }
  
  if (k == menu_size)
    return -1; //Not found

  while(*selected < 0)
    *selected = *selected + menu_len[menu_id]; //Wrap up if ative
  
  *selected = *selected % menu_len[menu_id]; //Wrap overflow 
  int x = 20;
  int j = 0;
  while (menu_items[k+j]) {
    if (*selected == j) {
      draw_string_osd(surface, menu_items[k+j], 12, x, sel_color);
    }
    else {
      draw_string_osd(surface, menu_items[k+j], 12, x, text_color);
    }
    x+=10;
    j++;
  }
  return menu_id + *selected + 1;
}


int menu_function(int action) {

  switch(action){
  case EXIT:
    printf("Exit\n");
    exit(0);
    break;
  case DRIVES:
    printf("Drives\n");
  }
}
  


int main() {
  long keysym;
  SDL_Event event;
  char myfile[256];
  SDL_Init( SDL_INIT_EVERYTHING );
  init_menu(); 
  
  SDL_Surface* screen = SDL_SetVideoMode(320, 240, 16, SDL_DOUBLEBUF);
  SDL_Surface* buffer = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);


  //menu_file_request(screen, myfile, ".");
  //printf("File: %s\n", myfile);
  SDL_LockSurface(buffer);

  int selected = 0;
  int first_run = 1;
  while(1) {
    int key_press = 0;
    if (first_run) {
      first_run = 0;
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
      if(keysym == SDLK_UP){
	selected--;
      } else if(keysym == SDLK_DOWN){
	selected++;
      }
    }
    int action = display_menu(buffer, "Main", &selected);
    //if (display_menu(buffer, "Drives", &selected) < 0) printf("Error: Requested menu not found\n");
    if(keysym == SDLK_SPACE){
      menu_function(action);
    }
    printf("Selected=%d\n", selected);
    printf("action=%d\n", action);

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
    SDL_Delay(500);
  }
  

}



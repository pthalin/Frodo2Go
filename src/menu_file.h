#ifndef __MENU_FILE_H__
#define __MENU_FILE_H__

#define MENU_FILE_MAX_ENTRY  2048
#define MENU_FILE_MAX_NAME    256
#define MENU_FILE_MAX_PATH    512

int menu_file_request(SDL_Surface* surface, char *out, char *pszStartPath);

#endif

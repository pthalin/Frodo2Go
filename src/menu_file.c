#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <SDL/SDL.h>
#include "font.h"
#define MENU_FILE_MAX_ENTRY  2048

#define MENU_FILE_MAX_NAME    256
#define MENU_FILE_MAX_PATH    512

static struct dirent files[MENU_FILE_MAX_ENTRY];
static struct dirent *sortfiles[MENU_FILE_MAX_ENTRY];
static int nfiles;

static int user_file_format = 2;
static void 
SJISCopy(struct dirent *a, char *file)
{
  unsigned char ca;
  int i;
  int len = strlen(a->d_name);

  for(i=0;i<=len;i++){
    ca = a->d_name[i];
    if (((0x81 <= ca)&&(ca <= 0x9f))
    || ((0xe0 <= ca)&&(ca <= 0xef))){
      file[i++] = ca;
      file[i] = a->d_name[i];
    }
    else{
      if(ca>='a' && ca<='z') ca-=0x20;
      file[i] = ca;
    }
  }
}


static int cmpFile(struct dirent *a, struct dirent *b)
{
  char file1[0x108];
  char file2[0x108];
  char ca, cb;
  int i, n, ret;
  
  if(a->d_type == b->d_type){
    SJISCopy(a, file1);
    SJISCopy(b, file2);
    n=strlen(file1);
    for(i=0; i<=n; i++){
      ca=file1[i]; cb=file2[i];
      ret = ca-cb;
      if(ret!=0) return ret;
    }
    return 0;
  }
  
  if(a->d_type == DT_DIR)  return -1;
  else                     return 1;
}

static void sort(struct dirent **a, int left, int right) 
{
  struct dirent *tmp, *pivot;
  int i, p;

  if (left < right) {
    pivot = a[left];
    p = left;
    for (i=left+1; i<=right; i++) {
      if (cmpFile(a[i],pivot)<0){
        p=p+1;
        tmp=a[p];
        a[p]=a[i];
        a[i]=tmp;
      }
    }
    a[left] = a[p];
    a[p] = pivot;
    sort(a, left, p-1);
    sort(a, p+1, right);
  }
}

static void my_sort(struct dirent **a, int left, int right) 
{
  struct dirent* swap_elem;
  int length = right - left;
  int index;

  for (index = 0; index < length; index++) {
    int perm = rand() % length;
    swap_elem = a[perm];
    a[perm] = a[index];
    a[index] = swap_elem;
  }
  sort(a, left, right);
}

int getExtId(char *szFilePath) 
{
  char *pszExt;
  if((pszExt = strrchr(szFilePath, '.'))) {
    if (!strcasecmp(pszExt, ".t64")) {
      return 1;
    } else
    if (!strcasecmp(pszExt, ".d64")) {
      return 2;
    }
  }
  return 0;
}



void 
getDir(const char *path) 
{
  DIR *fd;

  int b=0;
  int format = 0;

  nfiles = 0;

  if(strcmp(path,"./")){
    strcpy(files[nfiles].d_name,"..");
    files[nfiles].d_type = DT_DIR;
    sortfiles[nfiles] = files + nfiles;
    nfiles++;
    b=1;
  }
  fd = opendir(path);
  if (! fd) return;
  while(nfiles<MENU_FILE_MAX_ENTRY){
    memset(&files[nfiles], 0x00, sizeof(struct dirent));
    struct dirent *file_entry = readdir(fd);
    if (! file_entry) break;
    memcpy(&files[nfiles], file_entry, sizeof(struct dirent));
    if(files[nfiles].d_name[0] == '.') continue;

    if(files[nfiles].d_type == DT_DIR) {
      strcat(files[nfiles].d_name, "/");
      sortfiles[nfiles] = files + nfiles;
      nfiles++;
      continue;
    }
    sortfiles[nfiles] = files + nfiles;
    format = getExtId(files[nfiles].d_name);
    /*
    if ( (format == user_file_format               ) || 
         ( (format == FMGR_FORMAT_ZIP            ) && 
           (user_file_format != FMGR_FORMAT_KBD  ) &&
           (user_file_format != FMGR_FORMAT_JOY  ) &&
           (user_file_format != FMGR_FORMAT_STATE) ) ) 
    */
    if  (format == user_file_format               )
    {
      nfiles++;
    }
  }
  closedir(fd);
  if(b)
    my_sort(sortfiles+1, 0, nfiles-2);
  else
    my_sort(sortfiles, 0, nfiles-1);
}

void string_fill_with_space(char *buffer, int size)
{
  int length = strlen(buffer);
  int index;

  for (index = length; index < size; index++) {
    buffer[index] = ' ';
  }
  buffer[size] = 0;
}


int menu_file_request(SDL_Surface* surface, char *out, char *pszStartPath)
{
  static  int sel=0;
  SDL_Event event;
 
  int  last_time;
  int  tmp;
  unsigned short color;
  int top, x, y, i, up=0;
  char path[MENU_FILE_MAX_PATH];
  char oldDir[MENU_FILE_MAX_NAME];
  char buffer[MENU_FILE_MAX_NAME];
  char *p;
  long new_pad;
  long old_pad;
  int  file_selected;
  unsigned short text_color = SDL_MapRGB(surface->format, 200, 200, 200);
  unsigned short sel_color = SDL_MapRGB(surface->format, 128, 255, 128);
  const int rows=22;
 
  memset(files, 0x00, sizeof(struct dirent) * MENU_FILE_MAX_ENTRY);
  memset(sortfiles, 0x00, sizeof(struct dirent *) * MENU_FILE_MAX_ENTRY);
  nfiles = 0;

  strcpy(path, pszStartPath);
  getDir(path);

  last_time = 0;
  old_pad = 0;
  top = 0;
  file_selected = 0;

  if (sel >= nfiles) sel = 0;

  for(;;) 
  {
    x = 0; y = 15;
    
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = 270;
    rect.h = 320;
    SDL_FillRect(surface, &rect, 0);
    
    for(i=0; i<rows; i++){
      if(top+i >= nfiles) {
	break;
      }
      if (top+i == sel) {
	color = sel_color;
      }
      else {
	color = text_color;
      }
      strncpy(buffer, sortfiles[top+i]->d_name, 28);
      string_fill_with_space(buffer, 28);
      draw_string_osd(surface, buffer, x, y, color);
      y += 10;
    }

    SDL_Flip(surface);
  
    int key_press = 0;
    while(key_press == 0) {
 
      while(SDL_PollEvent(&event)) {
	switch(event.type) {
	case SDL_KEYDOWN:
	  new_pad = event.key.keysym.sym;
	  key_press = 1;
	  break;
	  
	case SDL_QUIT: 
	  printf("Quit requested, quitting.\n");
	  exit(0);
	  break;
	}
      }
    }
    
    if ((new_pad == SDLK_SPACE)) {
      if (sortfiles[sel]->d_type == DT_DIR) {
        if(!strcmp(sortfiles[sel]->d_name,"..")){
          up=1;
        } else {
          strcat(path,sortfiles[sel]->d_name);
          getDir(path);
          sel=0; 
        }
      }else{
        strcpy(out, path);
        strcat(out, sortfiles[sel]->d_name);
        strcpy(pszStartPath,path);
        file_selected = 1;
        break;
      }
    } else if(new_pad == SDLK_b){
      up=1;
    } else if(new_pad == SDLK_RETURN) {
      /* Cancel */
      file_selected = 0;
      break;
    } else if(new_pad == SDLK_UP){
      sel--;
    } else if(new_pad == SDLK_DOWN){
      sel++;
    } else if(new_pad == SDLK_LEFT){
      sel-=10;
    } else if(new_pad == SDLK_RIGHT){
      sel+=10;
    } 
       
    if(up) {
      if(strcmp(path,"./")){
        p=strrchr(path,'/');
        *p=0;
        p=strrchr(path,'/');
        p++;
        strcpy(oldDir,p);
        strcat(oldDir,"/");
        *p=0;
        getDir(path);
        sel=0;
        for(i=0; i<nfiles; i++) {
          if(!strcmp(oldDir, sortfiles[i]->d_name)) {
            sel=i;
            top=sel-3;
            break;
          }
        }
      }
      up=0;
    }

    if(top > nfiles-rows) top=nfiles-rows;
    if(top < 0)           top=0;
    if(sel >= nfiles)     sel=nfiles-1;
    if(sel < 0)           sel=0;
    if(sel >= top+rows)   top=sel-rows+1;
    if(sel < top)         top=sel;
  }

  return file_selected;
}

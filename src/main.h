/*
 *  main.h - Main program
 *
 *  Frodo Copyright (C) Christian Bauer
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

#ifndef _MAIN_H
#define _MAIN_H


class C64;

// Global variables
extern char AppDirPath[1024];	// Path of application directory


/*
 *  X specific stuff
 */

#ifdef __unix

class Prefs;

class Frodo {
 public:
  Frodo();
  void ArgvReceived(int argc, char **argv);
  void ReadyToRun();
  bool RunPrefsEditor();
  bool AutoRunEnabled();
  static const char *get_prefs_path() { return prefs_path; }

  
 private:
  void load_rom(const char *which, const char *path, uint8 *where, size_t size, const uint8 *builtin);
  void load_rom_files();
  
  static char prefs_path[256];	// Pathname of current preferences file
  static char d8_path[256];
  bool auto_run;
};

extern Frodo *TheApp;  // Pointer to Frodo object

#endif


// Global C64 object
extern C64 *TheC64;


/*
 *  Functions
 */

// Determine whether path name refers to a directory
extern bool IsDirectory(const char *path);

#endif

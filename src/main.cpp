/*
 *  main.cpp - Main program
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

#include "main.h"
#include "C64.h"
#include "Display.h"
#include "Prefs.h"
//#include "SAM.h"
#include "Version.h"

// Global variables
C64 *TheC64 = NULL;		// Global C64 object
char AppDirPath[1024];	// Path of application directory


// ROM file names
#ifndef DATADIR
#define DATADIR
#endif

#define BASIC_ROM_FILE DATADIR "Basic ROM"
#define KERNAL_ROM_FILE DATADIR "Kernal ROM"
#define CHAR_ROM_FILE DATADIR "Char ROM"
#define DRIVE_ROM_FILE DATADIR "1541 ROM"


// Builtin ROMs
#include "Basic_ROM.h"
#include "Kernal_ROM.h"
#include "Char_ROM.h"
#include "1541_ROM.h"


/*
 *  Load C64 ROM files
 */

void Frodo::load_rom(const char *which, const char *path, uint8 *where, size_t size, const uint8 *builtin)
{
#ifdef EXT_ROM_LOAD
  FILE *f = fopen(path, "rb");
	if (f) {
		size_t actual = fread(where, 1, size, f);
		fclose(f);
		if (actual == size)
			return;
	}

	// Use builtin ROM
	printf("%s ROM file (%s) not readable, using builtin.\n", which, path);
#endif
	memcpy(where, builtin, size);
}

void Frodo::load_rom_files()
{
	load_rom("Basic", BASIC_ROM_FILE, TheC64->Basic, BASIC_ROM_SIZE, builtin_basic_rom);
	load_rom("Kernal", KERNAL_ROM_FILE, TheC64->Kernal, KERNAL_ROM_SIZE, builtin_kernal_rom);
	load_rom("Char", CHAR_ROM_FILE, TheC64->Char, CHAR_ROM_SIZE, builtin_char_rom);
	load_rom("1541", DRIVE_ROM_FILE, TheC64->ROM1541, DRIVE_ROM_SIZE, builtin_drive_rom);
}


extern int init_graphics(void);


// Global variables
Frodo *TheApp = NULL;
char Frodo::prefs_path[256] = "";
char Frodo::d8_path[256] = "";


/*
 *  Create application object and start it
 */

int main(int argc, char **argv)
{
	timeval tv;
	gettimeofday(&tv, NULL);
	srand(tv.tv_usec);
	/*
	printf(
		"%s Copyright (C) Christian Bauer\n"
		"This is free software with ABSOLUTELY NO WARRANTY.\n"
		, VERSION_STRING
	);
	*/
	if (!init_graphics())
		return 1;
	fflush(stdout);

	TheApp = new Frodo();
	TheApp->ArgvReceived(argc, argv);
	TheApp->ReadyToRun();
	delete TheApp;

	return 0;
}


/*
 *  Constructor: Initialize member variables
 */

Frodo::Frodo()
{
	TheC64 = NULL;
}


/*
 *  Process command line arguments
 */

void Frodo::ArgvReceived(int argc, char **argv)
{
  if (argc == 2)
    strncpy(d8_path, argv[1], 255);
  //strncpy(prefs_path, argv[1], 255);
  //else if ((argc == 3) && !strcmp(argv[1], "-d8"))
  // strncpy(d8_path, argv[2], 255);
}


/*
 *  Arguments processed, run emulation
 */

void Frodo::ReadyToRun()
{
	getcwd(AppDirPath, 256);

	// Load preferences
	if (!prefs_path[0]) {
		char *home = getenv("HOME");
		if (home != NULL && strlen(home) < 240) {
			strncpy(prefs_path, home, 200);
			strcat(prefs_path, "/");
		}
		strcat(prefs_path, ".frodorc");
	}
	//ThePrefs.Load(prefs_path);
	strncpy(ThePrefs.DrivePath[0], d8_path,255);

	if (d8_path[0] == '\0') {
	  auto_run = false;
	  printf("ERROR: Path is to long or empty\n");
	} else {
	  auto_run = true;
	}
	
	
	// Create and start C64
	TheC64 = new C64;
	load_rom_files();
	TheC64->Run();
	delete TheC64;
}
bool Frodo::AutoRunEnabled()
{
  return auto_run;
}

/*
 *  Run preferences editor
 */

bool Frodo::RunPrefsEditor(void)
{
	Prefs *prefs = new Prefs(ThePrefs);
	bool result = prefs->ShowEditor(false, prefs_path);
	if (result) {
		TheC64->NewPrefs(prefs);
		ThePrefs = *prefs;
	}
	delete prefs;
	return result;
}


/*
 *  Determine whether path name refers to a directory
 */

bool IsDirectory(const char *path)
{
	struct stat st;
	return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}


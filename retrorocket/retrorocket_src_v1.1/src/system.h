/*
  system.h
  Copyright 2008 Asbjørn Djupdal and Morten Hartmann

  This file is part of RetroRocket, a gravity game for Nintendo DS.

  RetroRocket is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  RetroRocket is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with RetroRocket.  If not, see <http://www.gnu.org/licenses/>.
*/

///////////////////////////////////////////////////////////////////////////////
//
// System specific functions
// Implemented in a system specific file (system_ds.cpp for NDS)
// The rest of RetroRocket code is system independent
//
///////////////////////////////////////////////////////////////////////////////

#ifndef SYSTEM_H
#define SYSTEM_H

#include "network.h"

///////////////////////////////////////////////////////////////////////////////

enum Keys {
  NOKEY = 0,
  START = 1,
  SELECT = 2,
  LEFT = 4,
  RIGHT = 8,
  UP = 16,
  DOWN = 32,
  L = 64,
  R = 128,
  A = 256,
  B = 512,
  X = 1024,
  Y = 2048,
};

enum Colors {
  WHITE = 0,
  RED = 1,
  GREEN = 2,
  BLUE = 3,
  MAGENTA = 4,
  CYAN = 5,
  YELLOW = 6
};

// General
int main (int argc, char **argv);
void waitVBL();
void initIntroGfx();
void initGameGfx();
char *getUserName();

// Sprite
void setFocus (Ship *focus);
void createSprite (void *pal, void *gfx, int spritenum);
void createParticleSprite (int spritenum);
void deleteSprite (int spritenum);
void moveSprite (Fix32 x, Fix32 y, int spritenum);
void hideSprite (int spritenum);
void setSpriteRotEnable (int spritenum);
void rotateSprite (int spritenum, Fix32 angle);
void drawSOBsOnMap();
void hideMapSprite (int spritenum);
void showMapSprite (int spritenum);

// Action backgrounds
void loadBg (int *info,
             const unsigned short *map,
             int sizeofMap, const unsigned char *tiles,
             int sizeofTiles, const unsigned short *pal, int bg);
void deleteBg (int bg);
void loadBgPal (int bg, int palSlot, const unsigned short *pal);
void setBgTile (int bg, Fix32 x, Fix32 y, int tile, int palSlot = -1);
void showBG (int bg);
void hideBG (int bg);

// Map
void drawBM (u8 *buffer, int w, int h, int threshold,
             int left, int top, int width, int height, bool bw = true);
void drawMap (u16 *map, u8 *tiles, int *info, int w, int h,
              int left, int top, int width, int height);
void hideMap();
void showMap();

// Text
void clearText();
void clearText (int from, int to);
void setTextColor (int r, int g, int b);
void setTextColor (int c);
void outputText (int x, int y, const char *fmt, ...);

// File I/O
void getFileList (const char *ext, vector<char*>*filenames);

// Sound
void playSound (u8 *data, int size, int volume,  Fix32 x, Fix32 y,
                int rate = DEFAULT_SAMPLERATE);
void playSound (u8 *data, int size, int volume,
                int rate = DEFAULT_SAMPLERATE, int panning = 64);
void playSoundThrust (u8 *data, int size);
void modifySoundThrust (int rate, int volume);

// Music
void loadMod (const char *filename, 
              int randomStartPos, int randomPatterns, int blankPos);
void freeMod();
void playMod (int pos);
void playRandomMod();
void stopMod();

// Network
Network *openNetworkMaster (char *fieldFilename, int min, int max,
                            int aiPlayers, bool autogen);
Network *openNetworkSlave (char *fieldFilename);
void setWirelessChannel (int i);

// Input
unsigned int sysGetInput();
void readLine (void (*cb)(char *s), const char *str);
void getMapClick (int *x, int *y);
bool getPointer (int *x, int *y);
bool getPointer();

// Other
int startCritical();
void endCritical (int ime);

///////////////////////////////////////////////////////////////////////////////

#endif

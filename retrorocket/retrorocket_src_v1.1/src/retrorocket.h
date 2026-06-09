/*
  retrorocket.h
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
// Main RetroRocket header file
//
///////////////////////////////////////////////////////////////////////////////

#ifndef RETROROCKET_H
#define RETROROCKET_H

///////////////////////////////////////////////////////////////////////////////

#define DATADIR             "/retrorocket_data"
#define MAIN_CONF           "retrorocket.conf"
#define GAMEEXT             "game"
#define USERCONFEXT         "userconf"
#define LOCAL_CONFIG_FILE   "local.conf"
#define HIGHSCOREFILE       "highscores"
#define AUTOGEN_GAMEFILE    "autogen"

#define FILENAMESIZE        256  // Warning: Must be at least 256 for libfat
#define LINESIZE            256

#define ACTION_SCREEN       1
#define INFO_SCREEN         0
// action screen bg
#define FRONT_BG            1
#define SOB_BG              2
#define BACK_BG             3
// info screen bg
#define TEXT_BG             1
#define MAP_BG              3
#define KEYBOARD_BG         2

#define MAX_VOLUME           127
#define DEFAULT_SAMPLERATE   12000
#define MAX_AUDIBLE_DISTANCE (SCREEN_WIDTH * 2)

#define PARTICLE_PAL        15
#define PLAYERPAL_GRADIENTS 8
#define SPRITESIZE          16
#define HALF_SPRITESIZE     (SPRITESIZE/2)
#define ANGLES              32
#define MAX_SPRITES         128
#define MAX_SHIPS           PARTICLE_PAL
#define NAME_SIZE           10
#define SCREEN_HEIGHT       192
#define SCREEN_WIDTH        256
#define SCREEN_HALF_HEIGHT  (SCREEN_HEIGHT/2)
#define SCREEN_HALF_WIDTH   (SCREEN_WIDTH/2)
#define OFFSCREEN_X         (SCREEN_WIDTH+1)
#define OFFSCREEN_Y         (SCREEN_HEIGHT+1)
#define SPRITE_PRIO         2
#define SHIPID              ((1 << 31) | (id << 24))
#define COLUMNS             32
#define ROWS                24

#define DEFAULT_RANKING     0.5
#define RANK_IMPACT_FACTOR  10

#define PREDEFINED_TEXTCOLORS 10

// converts from degrees to NDS angles
#define DEG22      32
#define DEG45      64
#define DEG90     128
#define DEG180    256
#define DEG270    384
#define DEG360    512


extern bool bot;

///////////////////////////////////////////////////////////////////////////////

using namespace std;

///////////////////////////////////////////////////////////////////////////////

#ifdef NDS
#include <jtypes.h>
#else
#ifndef TYPES_DEFINED
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
#endif
#endif

#include <stdio.h>
#include <stack>
#include <list>
#include <vector>
#include <set>
#include <string>
#include <map>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <sys/dir.h>
#include <limits.h>
///////////////////////////////////////////////////////////////////////////////

#include "utils.h"

/** A single (collision) point */
struct RRPoint {
  Fix32 x, y;
};

//class Network;//seems to need a protoptype ..

struct ZoneSum {
  int properties;
  Fix32 forceX;
  Fix32 forceY;
};

///////////////////////////////////////////////////////////////////////////////

#ifdef PROF
#include "cyg-profiler/cyg-profile.h"
#endif

#include "menu.h"
#include "mob.h"
#include "ball.h"
#include "sob.h"
#include "weapon.h"
#include "field.h"
#include "system.h"
#include "config.h"
#include "worldgen.h"
#include "player.h"
#include "network.h"
#include "bot.h"

///////////////////////////////////////////////////////////////////////////////
// defines, enums

// Bit values for the input actions
typedef enum ACTION_BITS {
  ACTION_THRUST  = 1,  
  ACTION_LEFT    = 2,  
  ACTION_RIGHT   = 4,  
  ACTION_SHOOT_A = 8, 
  ACTION_SHOOT_B = 16, 
  ACTION_MENU    = 32,
  ACTION_BEAM    = 64

} ACTION_BITS;

///////////////////////////////////////////////////////////////////////////////

/** key codes for all input actions */
struct Controls {
  int left, right, thrust, triggerA, triggerB, menu, beam;
};

///////////////////////////////////////////////////////////////////////////////

/** Main retrorocket class.
    Create one object of this type and then call advance() 60 times each second
*/
class RetroRocket {
protected:
  // number of balls required to save before changing level
  int required_balls;
  // y value when ship reaches outer space (collects ball in thrust)
  int outer_space;

  int minNumberPlayers;
  int maxNumberPlayers;

  // how many balls are currently collected
  int collectedBalls;

  // current level (thrust), index into levelFilenames
  int currentLevel;

  bool finish;
  int finishCountdown;

  virtual void playGame();
  void endGame();
  void advanceGame();

  virtual void playMaster (bool autogen = false);
  void loadGame (char *filename, bool autogen = false);
  void initLevel();
  void cleanupLevel();
  void cleanupGame();
  void showGameIntroText();

public:
  volatile bool gameDone;

  // end code, -2 if game abort, -1 if draw, >= 0 if one winner
  volatile int winner;
  volatile bool timeoutReceived;
  
  Menu *mainMenu;
  Menu *gameMenu;
  Menu *netMenu;
  Menu *gameConfigMenu;
  Menu *autogenMenu;

  // config menu setting variables
  int channel;
  int musicSetting, oldMusicSetting;
  int soundSetting;
  int gameMusic;
  int updatefrequency;
  vector <const char *> musicTypeList;
  vector <const char *> soundSettingList;
  vector <const char *> boolList;
  vector <const char *> gameModeList;

  // game config setting variables
  int computerPlayers;
  int userMinPlayers;
  int requiredKills;
  int gameMode;

  int myCRC;
  int receivedCRC;
  
  // random seed for this level
  int fieldSeed;

  // bit masks of weapons that are allowed for this game
  int allowedWeaponA;
  int allowedWeaponB;

  // game is in strategy mode
  bool rtsMode;

  int vblcount;
  
  // file name of this game
  char fieldFilename[FILENAMESIZE];

  // main objects of this game
  Field *field;
  ParticleManager *manager;
  vector<Ship*> ships;
  vector<Player*> players;
  vector<Ball*> balls;
  vector<Sob*> sobs;
  vector<Mob*> mobs;
  vector<RRPoint> ballpoints;
  vector<unsigned short *>playerPals;
  Network *network;
  Config *config;
  Config *gameFile;
  WorldGen worldgen;

  vector<int> playerDeathOrder;

  char playerNames[MAX_SHIPS][NAME_SIZE+1];

  // id of local player (e.g. index into ships[] and players[])
  unsigned int localShip;

  // id of watched player (e.g. index into ships[] and players[])
  unsigned int watchedShip;

  // current key settings
  Controls localControls;

  // filenames of all levels of this game
  vector<char*> levelFilenames;

  // binary sound data
  int thrustSoundSize;
  u8 *thrustSound;
  int shootASoundSize;
  u8 *shootASound;
  int shootBSoundSize;
  u8 *shootBSound;
  int deadSoundSize;
  u8 *deadSound;
  int particleSoundSize;
  u8 *particleSound;
  int trackingSoundSize;
  u8 *trackingSound;
  
  // binary splash screen data
  int splashMapSize, splashTilesSize;
  unsigned short *splashMap;
  unsigned char *splashTiles;
  unsigned short *splashPal;
  int *splashInfo;

  //For testing the bot

  bool bot;

  // menu callback functions
  static void assignKey (Menu *menu);
  static void newGame (Menu *menu);
  static void playAutogen (Menu *menu);
  static void joinGame (Menu *menu);
  static void quitGame (Menu *menu);
  static void saveSettings (Menu *menu);
  static void musicAction (Menu *menu);
  static void mainMenuCallback (Menu *menu);
  static void changeChannel (Menu *menu);
  static void setupGameMenu (Menu *menu);

  void showSplash();
  void hideSplash();

  RetroRocket();
  virtual ~RetroRocket();

  /** call every VBL (60 times per second) */
  void advance() __attribute__ ((no_instrument_function));

  virtual void start();

  void abortGame();
  void nextLevel();
};

extern RetroRocket *retrorocket;

///////////////////////////////////////////////////////////////////////////////

#endif


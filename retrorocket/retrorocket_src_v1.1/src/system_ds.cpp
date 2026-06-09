/*
  system_ds.cpp
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
// NDS system specific code
//
///////////////////////////////////////////////////////////////////////////////

#include <PA9.h>
#include <nds.h>
#include <nds/arm9/console.h>

#include <MessageQueue.h>

#include <fat.h>
#include <mikmod9.h>

#define TYPES_DEFINED

#include "retrorocket.h"

#define THRUST_CHANNEL 15
#define TILE_PALPOS 12

static Fix32 bg_x;
static Fix32 bg_y;

static Ship* focus;

static inline void translatePos (Fix32 x, Fix32 y, Fix32 *xx, Fix32 *yy);
static inline void translateMapPos (Fix32 x, Fix32 y, Fix32 *xx, Fix32 *yy);
static inline void scrollBG (Ship *f, Fix32 *x, Fix32 *y);

static int numberOfMapShips;
static int mapLeft;
static int mapWidth;
static int mapTop;
static int mapHeight;
static int fieldWidth;
static int fieldHeight;

static int particleGfxNum;
static int mapshipGfxNum;

static int *bgInfo;

static bool mapVisible;

static bool front, back, sob;

static void (*readLineCallback)(char *str);
static bool keyboardActive;
static char readLineString[LINESIZE];
static int readLineCurPos;

static bool visibleMapSprite[MAX_SPRITES];

static bool gamemode;

static MODULE *module;
static bool randomMusicMode = false;
static bool playing = false;

static int musicRandomPos;
static int musicRandomPatterns;
static int musicBlankPattern;

static SoundInfo thrustSound;

static u8 particleSprite[SPRITESIZE * SPRITESIZE] __attribute__ ((aligned (4)));
static u16 particlePal[2];
static u8 *mapShip;

static char userName[NAME_SIZE+1];

Network *openNetwork (bool isMaster, char *fieldFilename, int min, int max,
                      int aiPlayers, bool autogen);
void initNet();

///////////////////////////////////////////////////////////////////////////////

// called by the drivers in mikmod library
// sends the value passed in as a parameter via the FIFO to the arm7
void MikMod9_SendCommand(u32 command) {
  char *data = (char*)(&command);
  IPC_SendMessage (data, sizeof (u32));
}

// reimplementation of Player_SetPosition(), but without clearing channels
// ugly, will probably break if mikmod is upgraded
void setModPosition (UWORD pos) {
  module->forbid=1;
  if (pos>=module->numpos) pos=module->numpos;
  module->posjmp=2;
  module->patbrk=0;
  module->sngpos=pos;
  module->vbtick=module->sngspd;
  module->forbid=0;
}

void timerInterrupt() {
  // change pattern if in random mode and pattern finished
  if (playing && randomMusicMode && module->sngpos == musicBlankPattern) {
    setModPosition (musicRandomPos + (rand() % musicRandomPatterns));
  }
  // player tick
  MikMod_Update();
  // the bpm can change in the middle of the song
  TIMER0_DATA = TIMER_FREQ_256(md_bpm * 50 / 125);
}

///////////////////////////////////////////////////////////////////////////////

char *getUserName() {
  return userName;
}
  
unsigned int sysGetInput() {
  if (keyboardActive) return 0;
  
  unsigned int input = 0;

  if (Pad.Held.Start)  input |= START;
  if (Pad.Held.Select) input |= SELECT;
  if (Pad.Held.Left)   input |= LEFT;
  if (Pad.Held.Right)  input |= RIGHT;
  if (Pad.Held.Up)     input |= UP;
  if (Pad.Held.Down)   input |= DOWN;
  if (Pad.Held.L)      input |= L;
  if (Pad.Held.R)      input |= R;
  if (Pad.Held.A)      input |= A;
  if (Pad.Held.B)      input |= B;
  if (Pad.Held.X)      input |= X;
  if (Pad.Held.Y)      input |= Y;

  return input;
}

void waitVBL() {
  PA_UpdatePad();
  PA_WaitForVBL();
}

///////////////////////////////////////////////////////////////////////////////

// TODO: combine with drawMap
void drawBM (u8 *buffer, int bmWidth, int bmHeight, int threshold,
             int left, int top, int width, int height, bool bw) {
  int screen = ACTION_SCREEN;
  if (gamemode) screen = INFO_SCREEN;

  PA_Clear8bitBg (screen);
  
  Fix32 skipX = (Fix32)bmWidth / (Fix32)width;
  Fix32 skipY = (Fix32)bmHeight / (Fix32)height;
  Fix32 skip = Fix32::max (skipX, skipY);

  fieldWidth = bmWidth;
  fieldHeight = bmHeight;
  mapWidth = ((Fix32)bmWidth / skip).toInt();
  mapLeft = left + (width - mapWidth) / 2;
  mapHeight = ((Fix32)bmHeight / skip).toInt();
  mapTop = top + (height - mapHeight) / 2;

  int yy = mapTop;
  for (Fix32 y = 0; y < bmHeight; y += skip) {
    int xx = mapLeft;
    for (Fix32 x = 0; x < bmWidth; x += skip) {
      if (yy >= SCREEN_HEIGHT) return;
      if (xx >= SCREEN_WIDTH) break;
      int p = buffer[y.toInt()*bmWidth + x.toInt()];
      if (bw) {
        PA_Put8bitPixel (screen, xx, yy, p > threshold ? 28 : 40);
      } else {
        int g = (p >> 3) + 2;
        int r = (p >> 3) + 34;
        PA_Put8bitPixel (screen, xx, yy, p > threshold ? g : r);
      }
      xx++;
    }
    yy++;
  }
}

void drawMap (u16 *map, u8 *tiles, int *info, int bmWidth, int bmHeight,
              int left, int top, int width, int height) {
  int screen = ACTION_SCREEN;
  if (gamemode) screen = INFO_SCREEN;

  PA_Clear8bitBg (screen);
  
  Fix32 skipX = (Fix32)bmWidth / (Fix32)width;
  Fix32 skipY = (Fix32)bmHeight / (Fix32)height;
  Fix32 skip = Fix32::max (skipX, skipY);

  fieldWidth = bmWidth;
  fieldHeight = bmHeight;
  mapWidth = ((Fix32)bmWidth / skip).toInt();
  mapLeft = left + (width - mapWidth) / 2;
  mapHeight = ((Fix32)bmHeight / skip).toInt();
  mapTop = top + (height - mapHeight) / 2;

  int yy = mapTop;
  for (Fix32 y = 0; y < bmHeight; y += skip) {
    int xx = mapLeft;
    for (Fix32 x = 0; x < bmWidth; x += skip) {
      if (yy >= SCREEN_HEIGHT) return;
      if (xx >= SCREEN_WIDTH) break;
      int p = Field::getCPixel (map, tiles, info,
                                x.toInt(), y.toInt(), bmWidth, bmHeight);
      int c = p > 0 ? 28 : 40;
      PA_Put8bitPixel (screen, xx, yy, c);
      xx++;
    }
    yy++;
  }
}

void hideMap() {
  int screen = ACTION_SCREEN;
  if (gamemode) screen = INFO_SCREEN;

  PA_HideBg (screen, MAP_BG);
  mapVisible = false;
  for (int i = 0; i < MAX_SPRITES; i++) {
    PA_SetSpriteXY (screen, i, OFFSCREEN_X, OFFSCREEN_Y);
  }
}

void drawSOBsOnMap() {
  int spritecounter = numberOfMapShips;
  Fix32 xxMap, yyMap;
  int owner = -1;
  int radarlevel;
  int stealthlevel;
  unsigned int i;
  if (mapVisible) {
    //PA_WaitForVBL(); // necessary to avoid corruption
    radarlevel =
      retrorocket->players[retrorocket->localShip]->upgradeLevels.radar;
    for (i=0; i < retrorocket->sobs.size(); i++) {
      Sob *sob = retrorocket->sobs[i];
      if (sob->sobType == Sob::NODE || sob->sobType == Sob::SENTRY) {
        // map sprite
        if (sob->sobType == Sob::NODE) {
          owner = ((ResourceNode*) sob)->owner;
          PA_SetSpritePal (INFO_SCREEN, spritecounter, owner);
        }
        if (sob->sobType == Sob::SENTRY) {
          if (((SentryGun *)sob)->hasOwner) {
            owner = ((SentryGun*) sob)->owner;
            PA_SetSpritePal (INFO_SCREEN, spritecounter, owner);
          } else {
            continue;
          }
        }
        stealthlevel = retrorocket->players[owner]->upgradeLevels.stealth;
        if (radarlevel > stealthlevel) {
          translateMapPos (sob->getCenterX(), sob->getCenterY(), &xxMap, &yyMap);
          PA_SetSpriteXY (INFO_SCREEN, spritecounter, xxMap.toUInt(),
                          yyMap.toUInt());
          showMapSprite (spritecounter);
          spritecounter++;
        }
      }
    }
    for (;spritecounter < MAX_SPRITES; spritecounter++) {
      hideMapSprite(spritecounter);
    }
  }
}

void showMap() {
  int screen = ACTION_SCREEN;
  if (gamemode) screen = INFO_SCREEN;

  PA_ShowBg (screen, MAP_BG);
  mapVisible = true;
  drawSOBsOnMap();
}

void hideMapSprite (int spritenum) {
  visibleMapSprite[spritenum] = false;
  PA_SetSpriteXY (INFO_SCREEN, spritenum, OFFSCREEN_X, OFFSCREEN_Y);
}

void showMapSprite (int spritenum) {
  visibleMapSprite[spritenum] = true;
}

///////////////////////////////////////////////////////////////////////////////

int main (int argc, char **argv) {
  // PAlib init
  PA_Init();
  PA_InitVBL();
  
  // init drawable bg (for maps)
  PA_WaitForVBL();
  PA_SetBgPalCol (INFO_SCREEN, 28, PA_RGB(0, 26, 0));
  PA_SetBgPalCol (INFO_SCREEN, 40, PA_RGB(6, 0, 0));
  PA_Init8bitBg (INFO_SCREEN, MAP_BG);
  hideMap();

  // init keyboard
  PA_InitKeyboard (KEYBOARD_BG);
  PA_ScrollKeyboardY (SCREEN_HEIGHT);
  keyboardActive = false;

  // init text on info screen
  PA_WaitForVBL();
  PA_InitText (INFO_SCREEN, TEXT_BG);

  infoText ("Initializing...");

  bg_x = 0;
  bg_y = 0;

  {  // init user name
    int i;
    for (i=0; i < PersonalData->nameLen; i++) {
      userName[i] = PersonalData->name[i];
    }
    userName[i] = 0;
  }
  
  // init file system
  if (!fatInitDefault()) panic ("Can not init libfat");
  if (chdir (DATADIR)) panic ("Datadir not present");

  focus = NULL;

  PA_WaitForVBL();

  // init BG
  bgInfo = NULL;
  front = false;
  back = false;

  // init mikmod
  MikMod_RegisterDriver(&drv_nds_hw);
  MikMod_RegisterLoader(&load_mod);
  if (MikMod_Init((char*)"")) {
    panic ("MikMod: %s", MikMod_strerror (MikMod_errno));
  }

  // init sound
  PA_InitASLibForSounds (AS_NO_DELAY | AS_MODE_16CH);
  AS_SetDefaultSettings (AS_PCM_8BIT, DEFAULT_SAMPLERATE, 0);
  AS_ReserveChannel (THRUST_CHANNEL);
  
  // create retrorocket object
  retrorocket = new RetroRocket();

  // init net
  initNet(); // must come after "new Retrorocket()"
             // because it needs config object

  // create sprites
  memset (particleSprite, 0, SPRITESIZE * SPRITESIZE);
  particleSprite[0] = 1;
  particlePal[0] = 0;
  particlePal[1] = PA_RGB(31, 31, 31);
  readBinary ((u8**)&mapShip, retrorocket->config->getString ("mapship"));

  // set seed
  int seed =
    PA_RTC.Year<<26 |
    PA_RTC.Month<<22 |
    PA_RTC.Day<<17 |
    PA_RTC.Hour<<12 |
    PA_RTC.Minutes<<6 |
    PA_RTC.Seconds;
  srand (seed);

#ifdef PROF
  // setup profiler
  irqSet(IRQ_HBLANK, cygprofile_hblank);
  irqEnable(IRQ_HBLANK);
  cygprofile_begin();
#endif

  retrorocket->start();

  // main loop
  while (true) {
    if (keyboardActive) {
      // print out current string
      readLineString[readLineCurPos] = '\0';
      for (int i = 0; i < COLUMNS; i++) {
        PA_OutputSimpleText (INFO_SCREEN, i, 9, " ");
      }
      PA_OutputSimpleText (INFO_SCREEN, 1, 9, readLineString);

      char c = PA_CheckKeyboard();
      if (c != 0) {
        if (c > 31) {
          readLineString[readLineCurPos++] = c;
          readLineString[readLineCurPos] = '\0';
        } else if (c == PA_TAB) {
          readLineString[readLineCurPos++] = ' ';
          readLineString[readLineCurPos++] = ' ';
        } else if ((c == PA_BACKSPACE) && (readLineCurPos > 0)) {
          readLineCurPos--;
        } else if (c == '\n') {
          readLineString[readLineCurPos] = '\0';
          keyboardActive = false;
          clearText();
          PA_ScrollKeyboardY (SCREEN_HEIGHT);
          readLineCallback (readLineString);
        }
      }
    } else {
      PA_UpdatePad();
    }
    PA_WaitForVBL();
    if (focus) scrollBG (focus, &bg_x, &bg_y);
    retrorocket->advance();
  }
}

void readLine (void (*cb)(char *s), const char *str) {
  readLineCallback = cb;
  clearText();
  PA_SetTextTileCol (INFO_SCREEN, YELLOW);
  outputText ((COLUMNS - strlen (str)) / 2, 7, str);
  PA_ScrollKeyboardXY (20, 95);
  readLineCurPos = 0;
  keyboardActive = true;
  PA_SetTextTileCol (INFO_SCREEN, WHITE);
}

static inline void translatePos (Fix32 x, Fix32 y, Fix32 *xx, Fix32 *yy) {
  *xx = x - bg_x;
  *yy = y - bg_y;
  if ((*xx > SCREEN_WIDTH) ||
      (*yy > SCREEN_HEIGHT) ||
      (*xx < (0 - SPRITESIZE)) ||
      (*yy < (0 - SPRITESIZE))) {
    *xx = OFFSCREEN_X;
    *yy = OFFSCREEN_Y;
  }
}

static inline void translateMapPos (Fix32 x, Fix32 y, Fix32 *xx, Fix32 *yy) {
  *xx = ((Fix32)mapLeft + ((x / (Fix32)fieldWidth)) * (Fix32)mapWidth)-7;
  *yy = ((Fix32)mapTop + ((y / (Fix32)fieldHeight)) * (Fix32)mapHeight)-7;
}

void scrollBG (Ship *f, Fix32 *x, Fix32 *y) {
  if (bgInfo) {
    if (f->x < SCREEN_HALF_WIDTH) {
      *x = 0;
    } else if (f->x > (retrorocket->field->width - SCREEN_HALF_WIDTH)) {
      *x = retrorocket->field->width - SCREEN_WIDTH;
    } else {
      *x = f->x - SCREEN_HALF_WIDTH;
    }
    if (f->y < SCREEN_HALF_HEIGHT) {
      *y = 0;
    } else if (f->y > (retrorocket->field->height - SCREEN_HALF_HEIGHT)) {
      *y = retrorocket->field->height - SCREEN_HEIGHT;
    } else {
      *y = f->y - SCREEN_HALF_HEIGHT;
    }

    if (front) {
      switch (bgInfo[0]) {
        case BG_TILEDBG:
          PA_BGScrollXY (ACTION_SCREEN, FRONT_BG, x->toUInt(), y->toUInt());
          break;
        case BG_INFINITEMAP:
        case BG_LARGEMAP:
          PA_LargeScrollXY (ACTION_SCREEN, FRONT_BG, x->toUInt(), y->toUInt());
          break;
        default:
          panic ("BG mode %d not supported", bgInfo[0]);
      }
    }

    if (back) {
      switch (bgInfo[0]) {
        case BG_TILEDBG:
          PA_BGScrollXY (ACTION_SCREEN, BACK_BG, x->toUInt(), y->toUInt());
          break;
        case BG_INFINITEMAP:
        case BG_LARGEMAP:
          PA_LargeScrollXY (ACTION_SCREEN, BACK_BG, x->toUInt(), y->toUInt());
          break;
        default:
          panic ("BG mode %d not supported", bgInfo[0]);
      }
    }

    if (sob) {
      switch (bgInfo[0]) {
        case BG_TILEDBG:
          PA_BGScrollXY (ACTION_SCREEN, SOB_BG, x->toUInt(), y->toUInt());
          break;
        case BG_INFINITEMAP:
        case BG_LARGEMAP:
          PA_LargeScrollXY (ACTION_SCREEN, SOB_BG, x->toUInt(), y->toUInt());
          break;
        default:
          panic ("BG mode %d not supported", bgInfo[0]);
      }
    }
  }
}

void setTextColor (int c) {
  if (!keyboardActive) {
    PA_SetTextTileCol (INFO_SCREEN, c);
  }
}

void setTextColor (int r, int g, int b) {
  if (!keyboardActive) {
    PA_SetTextCol (INFO_SCREEN, r, g, b);
  }
}

void setFocus (Ship *foc) {
  focus = foc;
}

void createSprite (void *pal, void *gfx, int spritenum) {
  PA_WaitForVBL(); // necessary to avoid corruption

  // action sprite
  int gfxNum = PA_GetSpriteGfx (ACTION_SCREEN, spritenum);
  if (gfxNum != particleGfxNum) {
    PA_DeleteGfx (ACTION_SCREEN, gfxNum);
  }
  gfxNum = PA_CreateGfx (ACTION_SCREEN, gfx, OBJ_SIZE_16X16, 1);

  PA_LoadSpritePal (ACTION_SCREEN, spritenum, pal);
  PA_SetSpritePal (ACTION_SCREEN, spritenum, spritenum);
  PA_SetSpriteGfx (ACTION_SCREEN, spritenum, gfxNum);

  // map sprite
  PA_LoadSpritePal (INFO_SCREEN, spritenum, pal);
  PA_SetSpritePal (INFO_SCREEN, spritenum, spritenum);
  numberOfMapShips = spritenum + 1;
  showMapSprite (spritenum);
}

void createParticleSprite (int spritenum) {
  int gfxNum = PA_GetSpriteGfx (ACTION_SCREEN, spritenum);
  if (gfxNum != particleGfxNum) {
    PA_DeleteGfx (ACTION_SCREEN, gfxNum);
  }
  PA_SetSpritePal (ACTION_SCREEN, spritenum, PARTICLE_PAL);
  PA_SetSpriteGfx (ACTION_SCREEN, spritenum, particleGfxNum);
}

void deleteSprite (int spritenum) {
  hideSprite (spritenum);
}

void moveSprite (Fix32 x, Fix32 y, int spritenum) {
  Fix32 xx, yy;
  Fix32 xxMap, yyMap;
  translatePos (x, y, &xx, &yy);
  PA_SetSpriteXY (ACTION_SCREEN, spritenum, xx.toUInt(), yy.toUInt());
  if (mapVisible && (spritenum < numberOfMapShips) &&
      visibleMapSprite[spritenum]) {
    translateMapPos (x+SPRITESIZE/2, y+SPRITESIZE/2, &xxMap, &yyMap);
    PA_SetSpriteXY (INFO_SCREEN, spritenum, xxMap.toUInt(), yyMap.toUInt());
  }
}

void hideSprite (int spritenum) {
  PA_SetSpriteXY (ACTION_SCREEN, spritenum, OFFSCREEN_X, OFFSCREEN_Y);
  if (spritenum < numberOfMapShips) {
    PA_SetSpriteXY (INFO_SCREEN, spritenum, OFFSCREEN_X, OFFSCREEN_Y);
  }
}

void setSpriteRotEnable (int spritenum) {
  PA_SetSpriteRotEnable (ACTION_SCREEN, spritenum, spritenum);
}

void rotateSprite (int spritenum, Fix32 angle) {
  int angleInt = DEG270 - angle.toInt();
  PA_SetRotsetNoZoom (ACTION_SCREEN, spritenum, angleInt);
}

void initIntroGfx() {
  gamemode = false;

  PA_WaitForVBL(); // necessary to avoid corruption

  // init action backgrounds
  PA_ResetBgSysScreen (ACTION_SCREEN); // clean up VRAM
  front = back = sob = false;

  // init sprites
  PA_ResetSpriteSys();

  // init drawable bg (for map preview)
  PA_WaitForVBL();
  PA_SetBgPalCol(ACTION_SCREEN, 1, PA_RGB(16, 16, 0));
  for (int i = 2; i <= 33; i++) {
    PA_SetBgPalCol(ACTION_SCREEN, i, PA_RGB(0, i-2, 0));
  }
  for (int i = 34; i <= 65; i++) {
    PA_SetBgPalCol(ACTION_SCREEN, i, PA_RGB(i-34, 0, 0));
  }
  PA_Init8bitBg (ACTION_SCREEN, SOB_BG);
  PA_SetBgPrio (ACTION_SCREEN, SOB_BG, SOB_BG);

  hideMap();
  // User text colours
  for (unsigned int i =0; i < retrorocket->playerPals.size(); i++) {
    int blue = ( retrorocket->playerPals[i][1] >> 10 ) & 0x1F;
    int green = ( retrorocket->playerPals[i][1] >> 5 ) & 0x1F;
    int red = ( retrorocket->playerPals[i][1] ) & 0x1F;
    PA_CreateTextPal(INFO_SCREEN, i+PREDEFINED_TEXTCOLORS, red,green,blue);
    if (i == 6) {
      // Don't overflow the palette
      break; 
    }
  }
}

void initGameGfx() {
  gamemode = true;

  // init backgrounds
  PA_ResetBgSysScreen (ACTION_SCREEN); // clean up VRAM
  front = back = sob = false;

  // init sprites
  PA_ResetSpriteSys();

  // setup game sprites
  PA_WaitForVBL();
  PA_LoadSpritePal (ACTION_SCREEN, PARTICLE_PAL, (void*)particlePal);
  particleGfxNum = PA_CreateGfx (ACTION_SCREEN, (void*)particleSprite,
                                 OBJ_SIZE_16X16, 1);
  mapshipGfxNum = PA_CreateGfx (INFO_SCREEN, (void*)mapShip,
                                OBJ_SIZE_16X16, 1);
  for (int i = 0; i < MAX_SPRITES; i++) {
    PA_CreateSpriteFromGfx (ACTION_SCREEN, i, particleGfxNum,
                            OBJ_SIZE_16X16, 1, PARTICLE_PAL,
                            OFFSCREEN_X, OFFSCREEN_Y);
    PA_SetSpritePrio (ACTION_SCREEN, i, SPRITE_PRIO);
  }
  for (int i = 0; i < MAX_SPRITES; i++) {
    PA_CreateSpriteFromGfx (INFO_SCREEN, i, mapshipGfxNum,
                            OBJ_SIZE_16X16, 1, 0, OFFSCREEN_X, OFFSCREEN_Y);
    PA_SetSpritePrio (INFO_SCREEN, i, MAP_BG); // TODO: why not SPRITE_PRIO?
    hideMapSprite (i);
  }
  hideMap();
}

void showBG (int bg) {
  switch (bg) {
    case FRONT_BG: 
      if (front) PA_ShowBg (ACTION_SCREEN, bg);
      break;
    case BACK_BG:
      if (back) PA_ShowBg (ACTION_SCREEN, bg);
      break;
    case SOB_BG:
      if (sob) PA_ShowBg (ACTION_SCREEN, bg);
      break;
  }
}

void hideBG (int bg) {
  switch (bg) {
    case FRONT_BG: 
      if (front) PA_HideBg (ACTION_SCREEN, bg);
      break;
    case BACK_BG:
      if (back) PA_ShowBg (ACTION_SCREEN, bg);
      break;
    case SOB_BG:
      if (sob) PA_ShowBg (ACTION_SCREEN, bg);
      break;
  }
}

void clearText() {
  if (!keyboardActive) {
    PA_ClearTextBg (INFO_SCREEN);
    setTextColor (0);
  }
}

void clearText (int from, int to) {
  if (!keyboardActive) {
    int chars = ((to-from+1)*COLUMNS);
    char spaces[chars+1];
    memset (spaces, ' ', chars);
    spaces[chars] = 0;
    PA_OutputSimpleText (INFO_SCREEN, 0, from, spaces);
    setTextColor (0);
  }
}

void getFileList (const char *ext, vector<char*>*filenames) {
  DIR_ITER* dir;
  set<string> filenamesSet;
  dir = diropen ("."); 

  if (dir == NULL) {
    panic ("Can't open directory");
  } else {
    char fn[FILENAMESIZE];
    while (dirnext (dir, fn, NULL) == 0) {
      if (matchExt (fn, ext)) {
        filenamesSet.insert(fn);
      }
    }

    set<string>::iterator setIter = filenamesSet.begin();
    while (setIter != filenamesSet.end()) {
      char *filename = (char*)malloc (setIter->size() + 1);
      if (!filename) panic ("Out of memory");
      strcpy (filename, setIter->c_str());
      filenames->push_back (filename);
      setIter++;
    }    
  }
  dirclose (dir);
}

void outputText (int x, int y, const char *fmt, ...) {
  if (!keyboardActive) {
    va_list ap;
    va_start (ap, fmt);

    char line[LINESIZE];
    vsprintf (line, fmt, ap);
    PA_OutputSimpleText (INFO_SCREEN, x, y, line);

    va_end (ap);
  }
}

void loadBg (int *info, 
             const unsigned short *map,
             int sizeofMap, const unsigned char *tiles,
             int sizeofTiles, const unsigned short *pal, int bg) {

  if (bg == FRONT_BG) {
    PA_DeleteBg (ACTION_SCREEN, FRONT_BG); // splash
    front = true;
  }
  if (bg == BACK_BG) back = true;
  if (bg == SOB_BG) sob = true;

  bgInfo = info;
  PA_WaitForVBL(); // necessary to avoid corruption
  PA_EasyBgLoadEx (ACTION_SCREEN, bg, (u32*)info,
                   (unsigned char*)tiles, sizeofTiles,
                   (unsigned short*)map, sizeofMap, 
                   (unsigned short*)pal);
  PA_SetBgPrio (ACTION_SCREEN, bg, bg);
  PA_HideBg (ACTION_SCREEN, bg);
}
  
void deleteBg (int bg) {
  PA_WaitForVBL(); // necessary to avoid corruption
  if (bg == FRONT_BG) front = false;
  if (bg == BACK_BG) back = false;
  if (bg == SOB_BG) sob = false;
  bgInfo = NULL;
  PA_DeleteBg (ACTION_SCREEN, bg);
}

void loadBgPal (int bg, int palSlot, const unsigned short *pal) {
  PA_LoadBgPalN (ACTION_SCREEN, bg, palSlot+1, (unsigned short*)pal);
}

void setBgTile (int bg, Fix32 xp, Fix32 yp, int tile, int palSlot) {
  unsigned short *bgMap = retrorocket->field->mapSOB;
  // FIXME: choose correct bg

  // adjust tile to accomodate palSlot
  tile |= (palSlot+1) << TILE_PALPOS;
  
  int tileToSet = retrorocket->field->getTile (xp.toUInt(), yp.toUInt());

  // set tile in map
  bgMap[tileToSet] = tile;

  int x = xp.toUInt() >> 3;
  int y = yp.toUInt() >> 3;
  int xx = bg_x.toUInt() >> 3;
  int yy = bg_y.toUInt() >> 3;

  // set tile on screen
  switch (bgInfo[0]) {
    case BG_TILEDBG:
      PA_SetLargeMapTile (ACTION_SCREEN, bg, x, y, tile);
      break;
    case BG_INFINITEMAP:
    case BG_LARGEMAP:
      if (((x - xx) >= -2) &&
          ((x - xx) < scrollpos[ACTION_SCREEN][SOB_BG].maxx) &&
          ((y - yy) >= -2) &&
          ((y - yy) < 30)) {
        PA_SetLargeMapTile (ACTION_SCREEN, bg, x&63, y&31, tile);
      }
    break;
    default: 
      panic ("BG mode %d not supported", bgInfo[0]);
  }
}

void playSound (u8 *data, int size, int volume, Fix32 x, Fix32 y, int rate) {
  Fix32 diffX = x - (bg_x + SCREEN_HALF_WIDTH);
  Fix32 diffY = y - (bg_y + SCREEN_HALF_HEIGHT);

  Fix32 distance = vectorLength (diffX, diffY);
  int newVolume = ((Fix32)volume * ((Fix32)MAX_AUDIBLE_DISTANCE - distance) /
                   (Fix32)MAX_AUDIBLE_DISTANCE).toInt();
  if (newVolume < 0) newVolume = 0;

  int panning = diffX.toInt() + 64;
  if (panning < 0) panning = 0;
  if (panning > 127) panning = 127;

  playSound (data, size, newVolume, rate, panning);
}

void playSound (u8 *data, int size, int volume, int rate, int panning) {
  if (retrorocket->soundSetting) {
    int c = AS_SoundDefaultPlay (data, size, volume, panning, false, 0);
    AS_SetSoundRate (c, rate);
    AS_SetSoundPan (c, panning);
  }
}

void playSoundThrust (u8 *data, int size) {
  SoundInfo si = {(u8*)data, size,
                 AS_PCM_8BIT, DEFAULT_SAMPLERATE, 127, 64, true, 0, 0};
  thrustSound = si;
  AS_SoundDirectPlay (THRUST_CHANNEL, thrustSound); // start the thrust sound
  AS_SetSoundVolume (THRUST_CHANNEL, 0); // set thrust volume to 0
}

void modifySoundThrust (int rate, int volume) {
  if (retrorocket->soundSetting) {
    AS_SetSoundRate (THRUST_CHANNEL, rate);
    AS_SetSoundVolume (THRUST_CHANNEL, volume);
  }
}

Network *openNetworkMaster (char *fieldFilename, int min, int max,
                            int aiPlayers, bool autogen) {
  return openNetwork (true, fieldFilename, min, max, aiPlayers, autogen);
}

Network *openNetworkSlave (char *fieldFilename) {
  return openNetwork (false, fieldFilename, 0, 0, 0, false);
}

int startCritical() {
  int ime = REG_IME;
  REG_IME = IME_DISABLE;
  return ime;
}

void endCritical (int ime) {
  REG_IME = ime;
}

void getMapClick (int *x, int *y) {
  bool done = false;
  while (!done) {
    PA_WaitForVBL();
    if (Stylus.Released) {
      done = true;
      *x = (Stylus.X - mapLeft) * fieldWidth / mapWidth;
      *y = (Stylus.Y - mapTop) * fieldHeight / mapHeight;
    }
  }
}

bool getPointer (int *x, int *y) {
  *x = Stylus.X;
  *y = Stylus.Y;
  return getPointer();
}

bool getPointer () {
  return (Stylus.Held || Stylus.Newpress);
}

void loadMod (const char *filename, 
              int randomStartPos, int randomPatterns, int blankPos) {
  module = Player_Load ((char *)filename, 12, 0);
  if (!module) panic ("MikMod: %s", MikMod_strerror (MikMod_errno));
  md_volume = 64;

  musicRandomPos = randomStartPos;
  musicRandomPatterns = randomPatterns;
  musicBlankPattern = blankPos;

  TIMER0_CR = 0;
  irqSet (IRQ_TIMER0, timerInterrupt);
  irqEnable (IRQ_TIMER0);
  TIMER0_DATA = TIMER_FREQ_256(md_bpm * 50 / 125);
  TIMER0_CR = TIMER_DIV_256 | TIMER_IRQ_REQ | TIMER_ENABLE;
}

void freeMod() {
  if (playing) Player_Stop();
  irqDisable (IRQ_TIMER0);
  Player_Free (module);
}

void playMod (int pos) {
  if (!playing) {
    Player_Start (module);
    playing = true;
  }
  Player_SetPosition (pos);
  randomMusicMode = false;
  timerInterrupt();
  AS_SoundDirectPlay (THRUST_CHANNEL, thrustSound); // start the thrust sound
  AS_SetSoundVolume (THRUST_CHANNEL, 0); // set thrust volume to 0
}

void playRandomMod() {
  playMod (musicRandomPos + (rand() % musicRandomPatterns));
  randomMusicMode = true;
}

void stopMod() {
  Player_Stop();
  randomMusicMode = false;
  playing = false;
  timerInterrupt();
  AS_SoundDirectPlay (THRUST_CHANNEL, thrustSound); // start the thrust sound
  AS_SetSoundVolume (THRUST_CHANNEL, 0); // set thrust volume to 0
}

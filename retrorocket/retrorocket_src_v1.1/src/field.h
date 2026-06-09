/*
  field.h
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
// Playing fields (levels)
//
///////////////////////////////////////////////////////////////////////////////

#ifndef FIELD_H
#define FIELD_H

#include "utils.h"
#include "retrorocket.h"

#define TILE_N     1023
#define TILE_HFLIP 1024
#define TILE_VFLIP 2048
#define TRANSPARENT_COLOR 0
#define COLOUR_BITS 5
#define BG_LARGEMAP 4
#define AUTOGENTILES 9
#define TILESETSIZE 64*AUTOGENTILES

///////////////////////////////////////////////////////////////////////////////

/** Information about a landing platform */
struct Platform {
  Fix32 x1;
  Fix32 x2;
  Fix32 y;
  int properties;
  unsigned int homeShip;
};

/** Information about a special zone */
enum ZoneProperties {
  GRAVITYFIELD = 1,
  LIQUID = 2
};

struct RRZone {
  Fix32 x;
  Fix32 y;
  Fix32 w;
  Fix32 h;
  int properties;
  Fix32 forceX;
  Fix32 forceY;
};

enum Direction {
  UNIDIRECTIONAL,
  UPWARDS,
  DOWNWARDS, 
  LEFTWARDS, 
  RIGHTWARDS
};

/** waypoint for laps */
struct WayPoint {
  Fix32 x;
  Fix32 y;
  Fix32 length;
  bool horizontal;
  Direction direction;
};

///////////////////////////////////////////////////////////////////////////////

class Config;
class WorldGen;

/** Playing field */
class Field {

protected:
  Field() {}
  void findBlankTile();

  void createFromBM (Config *c, int bmWidth, int bmHeight,
                     u8 *bmBuffer, int seaLevel);

public:

  enum PlatformProperties {
    LANDED = 1,
    HOME = 2,
    FILLINGSTATION = 4,
    WEAPONA = 8,
    WEAPONB = 16,
    GARAGE = 32
  };

  int score; // score for this level

  int width;
  int height;

  bool foundBM;
  bool foundFG;
  bool foundBG;
  bool foundSOB;
  bool foundCM;

  int numberWayPoints;
  
  u8 *bmBuffer;

  int *info;
  u16 *collisionMap;
  u8 *collisionTiles;
  int collisionTilessize;

  int mapsize;

  u16 *mapFG;
  u8 *tilesFG;
  u16 *paletteFG;
  int tilessizeFG;
  int paletteSizeFG;

  u16 *mapBG;
  u8 *tilesBG;
  u16 *paletteBG;
  int tilessizeBG;
  int paletteSizeBG;

  u16 *mapSOB;
  u8 *tilesSOB;
  u16 *paletteSOB;
  int tilessizeSOB;
  int paletteSizeSOB;

  // tile that is completely blank (nothing ship collides with)
  int blankTile;

  vector<Platform> platforms;
  vector<RRZone> zones;
  vector<WayPoint> wayPoints;
  vector<RRPoint> startingPoints;

  /** load field from filename */
  Field (char *filename);
  /** load field from given config object */
  Field (Config *file);
  /** load autogen field from given config and worldgen objects */
  Field (Config *file, WorldGen *worldgen);

  virtual ~Field();
  void load (Config *file);

  static int getRed (int pixel, u16 *pal) {
    u16 col = pal[pixel];
    return (col & 0x1f);
  }

  static int getGreen (int pixel, u16 *pal) {
    u16 col = pal[pixel];
    return ((col >> 5) & 0x1f);
  }

  static int getBlue (int pixel, u16 *pal) {
    u16 col = pal[pixel];
    return ((col >> 10) & 0x1f);
  }

  int getTile (int x, int y) {
    return getTile (info, x, y);
  }

  static int getTile (int *info, int x, int y) {
    // find tile position in map
    // TODO: this is just guessing, not correct in all cases
    int tilePos = 0;
    switch (info[0]) {
      case 2: { // Normal Map
        int megaTile = ((y >> 8) * (info[1] >> 8)) + (x >> 8);
        int shiftValue = info[1] > 256 ? 4 : 3;
        tilePos =
          megaTile * 1024 + 
          (((y & 0xff) >> 3) * (info[1] >> shiftValue)) +
          ((x & 0xff) >> 3);
        break;
      }
      case 5: // Infinite Map
      case 4: // Large Map
        tilePos = ((y >> 3) * (info[1] >> 3)) + (x >> 3);
        break;
      default:
        panic ("Map mode %d not implemented", info[0]);
    }
    return tilePos;
  }

  static int getPixel (u16 *map, u8 *tiles, int *info, Fix32 x, Fix32 y,
                       Fix32 width, Fix32 height, int blankTile) {
    // define all pixels outside field as 1
    if ((x < 0) || (y < 0) || (x > width) || (y > height)) {
      return 1;
    }
    int xint = x.toUInt();
    int yint = y.toUInt();
    int tilePos = getTile (info, xint, yint);

    // get tile
    int tile = *(map + tilePos);
    if (tile == blankTile) return TRANSPARENT_COLOR; // no need to check further

    int pos = 0;

    // tile is flipped horizontally
    if (tile & TILE_HFLIP) {
      pos += 7 - (xint & 7);
    } else {
      pos += xint & 7;
    }

    // tile is flipped vertically
    if (tile & TILE_VFLIP) {
      pos += (7 - (yint & 7)) * 8;
    } else {
      pos += (yint & 7) * 8;
    }

    return *(tiles + (tile & TILE_N) * 8 * 8 + pos);
  }

  static bool getCPixel (u16 *map, u8 *tiles, int *info, Fix32 x, Fix32 y,
                         Fix32 width, Fix32 height, int blankTile) {
    return getPixel (map, tiles, info, x, y, width, height, blankTile)
      != TRANSPARENT_COLOR;
  }
  static bool getCPixel (u16 *map, u8 *tiles, int *info,
                         Fix32 x, Fix32 y, Fix32 width, Fix32 height) {
    return getCPixel (map, tiles, info, x, y, width, height, -1);
  }
  /** returns true if given coordinates should collide on this field */
  bool getCPixel (Fix32 x, Fix32 y) {
    return getCPixel (collisionMap, collisionTiles, info, x, y,
                      width, height, blankTile);
  }

  /** same as getCPixel, but for SOB layer, and only tile exact, not pixel */
  bool getSOBCPixel (Fix32 x, Fix32 y) {
    // define all pixels outside field as false
    if ((x < 0) || (y < 0) || (x > width) || (y > height)) {
      return false;
    }
    int xint = x.toUInt();
    int yint = y.toUInt();
    int tilePos = getTile (info, xint, yint);
    int tile = *(mapSOB + tilePos);
    return tile != 0;
  }

  int landed (Ship *ship);  // returns bitmask of PlatformProperties
  ZoneSum getZoneSum (Mob *mob); // returns zone force and properties
  int getWayPoint (Ship *ship); // returns waypoint at ship (or 0)
  RRPoint getStart (unsigned int id);
  static bool fieldIsOfGivenType (char *filename,
                                  int gameMode, int userMinPlayers,
                                  int computerPlayers);
  static void printFieldInfo (Menu *menu);
  bool hasLaps() { return wayPoints.size() > 0; }
};

///////////////////////////////////////////////////////////////////////////////

#endif

/*
  sob.h
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
// SOBs (stationary objects)
// SOBs are drawn directly onto a tiled layer and is therefore
// more limited than MOBs wrt placement
//
///////////////////////////////////////////////////////////////////////////////

#ifndef SOB_H
#define SOB_H

#include "retrorocket.h"

///////////////////////////////////////////////////////////////////////////////

/** animated SOBs use this to create the animation sequence */
struct AnimStep {
  int frame;
  int duration;
};

/** base class for all sobs  */
class Sob {

protected:
  // tile data for first animation frame
  const u8 *sobTiles; // the tile data for this sob (first animation frame)
  int sobStartTile;   // the tile number for the first for this sob

  // tile data for current animation frame
  const u8 *tiles; // the tile data for this sob (current animation frame)
  int startTile;   // the tile number of the first tile for this sob

  bool visible;
  int wmask;
  int hmask;
  int animTimer;
  
  int score;

  int currentAnimStep;
  vector<AnimStep> animSteps;

  virtual void showTile (int index, Fix32 x, Fix32 y, int tile);
  void hideTile (int index, Fix32 x, Fix32 y);

  // coordinate of lower right corner
  Fix32 right;
  Fix32 bottom;

public:
  int owner; // owner of this sob (e.g. for resource node)
  Fix32 x;
  Fix32 y;
  int killer; // the player who destroyed this sob
  int width;
  int height;

  static int blankStartTile;

  enum SobType {
    OTHER,
    DOOR,
    RESOURCE,
    BUTTON,
    MARKER,

    NODE,
    SENTRY,
    PLATFORM,
    FUEL,
    POWERPLANT,

    BALLMOUNT,
    STRENGTHBAR,
    GUN
  };

  SobType sobType;

  Fix32 strength;

  Sob (Fix32 x, Fix32 y);
  virtual ~Sob() {}

  // functions for copying sob graphic data to the total field graphics
  static void copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                         int *fieldTilessize, int *fieldPaletteSize,
                         const u8 *tiles, int sizeOfTiles,
                         const u16 *pal, int sizeOfPal,
                         int *startTile, bool copyPal);

  static void copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                         int *fieldTilessize, int *fieldPaletteSize,
                         const u8 *tiles, int sizeOfTiles,
                         const u16 *pal, int sizeOfPal,
                         int *startTile) {
    copyTiles (fieldTiles, fieldPalette,
               fieldTilessize, fieldPaletteSize,
               tiles, sizeOfTiles,
               pal, sizeOfPal,
               startTile, true); }

  static void initSOBs (u8 **fieldTiles, u16 **fieldPalette,
                        int *fieldTilessize, int *fieldPaletteSize);

  virtual int getID();
  virtual void performCollision (int damage, int shooter);
  virtual void show();
  virtual void hide();
  virtual void updateStrength (Fix32 strength, bool sendPacket = true) {
    this->strength = strength;
  }
  virtual void advance() = 0;
  virtual void init();
  virtual bool getCPixel (Fix32 x, Fix32 y, int damage, int shooter = -1);
  virtual Fix32 getCenterX() { return x + ((Fix32)8*width)/2; }
  virtual Fix32 getCenterY() { return y + ((Fix32)8*height)/2; }
  virtual void performBeamAction (Ship *ship) {}
  virtual void deleteSelf (bool sendPacket = true);
};

///////////////////////////////////////////////////////////////////////////////

class BallMount : public Sob {
  int capacity;

public:
  static int ballMountStartTile;

  BallMount (Fix32 x, Fix32 y);
  void advance();
  void performCollision (int damage, int shooter);
  void deleteSelf (bool sendPacket = true);
  static void copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                         int *fieldTilessize, int *fieldPaletteSize);
};

///////////////////////////////////////////////////////////////////////////////

class Fuel : public Sob {
  int capacity;
  int gas_refill_rate;

public:
  static int fuelStartTile;

  Fuel (Fix32 x, Fix32 y);
  void advance();
  void performBeamAction (Ship *ship);
  void deleteSelf (bool sendPacket = true);
  static void copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                         int *fieldTilessize, int *fieldPaletteSize);
};

///////////////////////////////////////////////////////////////////////////////

class PowerPlant : public Sob {
  int inactiveCounter;
  int animTimer;
  int currentFrame;
  int totalFrames;

public:
  bool active;
  static PowerPlant *powerPlant;

  static int powerplantStartTile;

  PowerPlant (Fix32 x, Fix32 y);
  void advance();
  void deleteSelf (bool sendPacket = true);
  void performCollision (int damage, int shooter);
  static void copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                         int *fieldTilessize, int *fieldPaletteSize);
};

///////////////////////////////////////////////////////////////////////////////
class StrengthBar : public Sob {
  Sob *parentSob;

public:
  static int strengthBarStartTile;

  StrengthBar (Fix32 x, Fix32 y, Sob *parent);
  void advance();
  void deleteSelf (bool sendPacket = true);
  void displayStrength (int percent, bool sendPacket = true);
  bool getCPixel (Fix32 x, Fix32 y, int damage, int shooter = -1) {
    return false;
  }
  static void copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                         int *fieldTilessize, int *fieldPaletteSize);
};

///////////////////////////////////////////////////////////////////////////////
class RRMarker : public Sob {
  
 public:
  static int markerStartTile;
  RRMarker (Fix32 x, Fix32 y, int owner);
  void init();
  void advance();
  void deleteSelf (bool sendPacket = true);
  bool getCPixel (Fix32 x, Fix32 y, int damage, int shooter = -1) {
    return false;
  }
  void showTile (int index, Fix32 x, Fix32 y, int tile);
  static void copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                         int *fieldTilessize, int *fieldPaletteSize);
};

///////////////////////////////////////////////////////////////////////////////

class Resource : public Sob {
public:
  static int resourceStartTile;

  Resource (Fix32 x, Fix32 y);
  void advance();
  void performCollision (int damage, int shooter);
  void deleteSelf (bool sendPacket = true);
  bool getCPixel (Fix32 x, Fix32 y, int damage, int shooter = -1);
  static void copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                         int *fieldTilessize, int *fieldPaletteSize);
};

///////////////////////////////////////////////////////////////////////////////

class ResourceNode : public Sob {

public:
  StrengthBar* strengthBar;
  RRMarker* marker;
  static int resourceNodeStartTile;
  int timer;
  Fix32 maxStrength;
  ResourceNode (Fix32 x, Fix32 y, int owner);
  void advance();
  virtual void deleteSelf (bool sendPacket = true);
  void updateStrength (Fix32 strength, bool sendPacket = true) {
    this->strength = strength;
    strengthBar->displayStrength (100*strength.toUInt()/maxStrength.toUInt(),
                                  sendPacket);
  }
  void performCollision (int damage, int shooter);
  static void copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                         int *fieldTilessize, int *fieldPaletteSize);
};

///////////////////////////////////////////////////////////////////////////////

class ButtonRight : public Sob {
public:
  static int buttonrightStartTile;

  ButtonRight (Fix32 x, Fix32 y);
  void advance();
  void performCollision (int damage, int shooter);
  void deleteSelf (bool sendPacket = true);
  static void copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                         int *fieldTilessize, int *fieldPaletteSize);
};

///////////////////////////////////////////////////////////////////////////////

class ButtonLeft : public Sob {
public:
  static int buttonleftStartTile;

  ButtonLeft (Fix32 x, Fix32 y);
  void advance();
  void deleteSelf (bool sendPacket = true);
  void performCollision (int damage, int shooter);
  static void copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                         int *fieldTilessize, int *fieldPaletteSize);
};

///////////////////////////////////////////////////////////////////////////////

class Gun : public Sob {
protected:
  Fix32 angleOffset;
  Fix32 gunX;
  Fix32 gunY;
  int gunstation_rate;
  Fix32 gunstation_reaction_distance;

public:
  Gun (Fix32 x, Fix32 y);
  void deleteSelf (bool sendPacket = true);
  virtual void advance();
};

///////////////////////////////////////////////////////////////////////////////

class GunNW : public Gun {
public:
  static int gunnwStartTile;

  GunNW (Fix32 x, Fix32 y);
  static void copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                         int *fieldTilessize, int *fieldPaletteSize);
};

///////////////////////////////////////////////////////////////////////////////

class GunNE : public Gun {
public:
  static int gunneStartTile;

  GunNE (Fix32 x, Fix32 y);
  static void copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                         int *fieldTilessize, int *fieldPaletteSize);
};

///////////////////////////////////////////////////////////////////////////////

class GunSE : public Gun {
public:
  static int gunseStartTile;

  GunSE (Fix32 x, Fix32 y);
  static void copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                         int *fieldTilessize, int *fieldPaletteSize);
};

///////////////////////////////////////////////////////////////////////////////

class GunSW : public Gun {
public:
  static int gunswStartTile;

  GunSW (Fix32 x, Fix32 y);
  static void copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                         int *fieldTilessize, int *fieldPaletteSize);
};

///////////////////////////////////////////////////////////////////////////////

class SentryGun : public Sob {

public:
  enum SentryType {
    RANDOM,
    DIRECTED,
    PREDICTING,
    HEATSEEKER
  };

protected:
  Fix32 gunX;
  Fix32 gunY;
  RRMarker *marker;
  StrengthBar* strengthBar;
  int advanceCounter;
  Fix32 maxStrength;
  int gunstation_rate;
  int volume_shot;
  int volume_tracking;
  int period_tracking;
  int seek_radius;
  Fix32 bullet_speed;
  bool tracking;
  int trackingPeriodCounter;
  
  void setupObject (Fix32 x, Fix32 y, Fix32 angle, int owner, SentryType type);

public:
  bool hasOwner;
  SentryType sentryType;
  Fix32 angleOffset;
  void performCollision (int damage, int shooter);
  SentryGun (Fix32 x, Fix32 y, Fix32 angle, SentryType type);
  SentryGun (Fix32 x, Fix32 y, Fix32 angle, int owner, SentryType type);
  void fire(Fix32 angle, bool sendPacket = true);
  virtual void deleteSelf (bool sendPacket = true);
  void updateStrength (Fix32 strength, bool sendPacket = true) {
    this->strength = strength;
    strengthBar->displayStrength (100*strength.toUInt()/maxStrength.toUInt(),
                                  sendPacket);
  }
  virtual void advance();
  void init();
  static int sentryGunStartTile;
  static void copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                         int *fieldTilessize, int *fieldPaletteSize);
};

///////////////////////////////////////////////////////////////////////////////

class Door : public Sob {

  int counter;
  bool open;

public:
  static int doorStartTile;

  Door (Fix32 x, Fix32 y, Fix32 x2, Fix32 y2);
  void advance();
  void show();
  void deleteSelf (bool sendPacket = true);
  bool getCPixel (Fix32 xx, Fix32 yy, int damage, int shooter = -1);
  void openDoor();
  static void copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                         int *fieldTilessize, int *fieldPaletteSize);
};

///////////////////////////////////////////////////////////////////////////////

class PlatformSOB : public Sob {
private:
  StrengthBar* strengthBar;
  RRMarker *marker;
  int advanceCounter;
  Fix32 maxStrength;
  Fix32 platform_strength;
  
public:
  static int platformStartTile;

  PlatformSOB (Fix32 x, Fix32 y, int owner);
  void advance();
  void updateStrength (Fix32 strength, bool sendPacket = true) {
    this->strength = strength;
    strengthBar->displayStrength (100*strength.toUInt()/maxStrength.toUInt(),
                                  sendPacket);
  }
  void init();
  virtual void deleteSelf (bool sendPacket = true);
  void performCollision (int damage, int shooter);
  static void copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                         int *fieldTilessize, int *fieldPaletteSize);
  static int platformWidth() { return 48; }
};

///////////////////////////////////////////////////////////////////////////////

#endif

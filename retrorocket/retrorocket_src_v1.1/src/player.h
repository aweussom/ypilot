/*
  player.h
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
// Represents a player
//
///////////////////////////////////////////////////////////////////////////////

#ifndef PLAYER_H
#define PLAYER_H

#include "retrorocket.h"

/** current level for all possible upgrades */
struct UpgradeLevels {
  int shipStrength;
  int stealth;
  int radar;
  int capacity;
  int firingRate;
  int buildingStrength;
  int sentryRate;
  int damage;
  int bulletSpeed;
  int thrust;
  int weight;
  int nodeGatherRate;
};

/** Base class representing a player
    Every ship has a player, the subclass identifies the player as
    remote, local etc. 
 */
class Player {

protected:
  vector <WeaponA*> weaponAAvailable;
  vector <WeaponB*> weaponBAvailable;

  virtual void updateMenus() {}

public:
  Ship *ship;

  unsigned int id;

  UpgradeLevels upgradeLevels;
  
  // player state
  int lives;
  int score;
  int frags;
  int bestLapTime;
  bool dead;
  bool immortal;
  Fix32 credits;

  int weaponAInventory;

  // absolute max ship capacity
  int maxGas;
  int maxWeaponA;
  int maxWeaponB;
  int maxStrength;
  int maxLives;

  // user defined max ship capacity
  int userMaxGas;
  int userMaxWeaponA;
  int userMaxWeaponB;

  int autoFireRate;
  int repetitionRate;
  Fix32 buildingStrengthFactor;
  Fix32 nodeGatherRate;
  int sentryRate;
  Fix32 damageScaler;
  Fix32 bulletSpeed;

  Fix32 weightFactor;
  Fix32 rotSpeed;
  Fix32 thrust;

  int singleShotDamage;
  int sentryDamage;
  int bombDamage;

  int resurrectTime;
  int immortalTime;
  bool shootBKeyPressed;
  int oldActions, newActions;

  Fix32 gas; // here and not in Ship because of levelshift in Thrust

  Player (unsigned int id);
  virtual ~Player();

  virtual void advance() {}
  virtual void setWeaponA (int weapon) = 0;
  virtual void setWeaponB (int weapon) = 0;
  virtual void updateAction();
  virtual void registerShip (Ship *ship);
  virtual void shopUpgrade (Upgrades upgrade, bool ignorePrice = false);
  static int getUpgradePrice (int level, const char* upgradePriceTag);
  virtual void setPermanentlyDead() {}
  virtual bool isLocalPlayer() { return true; }
};

///////////////////////////////////////////////////////////////////////////////

/** represents a remote player (network player) */
class RemotePlayer : public Player {
  vector <WeaponA*> weaponAList;
  vector <WeaponB*> weaponBList;

public:
  RemotePlayer (unsigned int id) : Player (id) {}

  void registerShip (Ship *ship);
  void setWeaponA (int weapon);
  void setWeaponB (int weapon);
  void advance();
  virtual bool isLocalPlayer() { return false; }
};

///////////////////////////////////////////////////////////////////////////////

/** represents a local player */
class LocalPlayer : public Player {

private:
  bool menuRemoved;
  int vblcount;

  void updateMenus();
  void getInput();
  void updateShipConfig();

public:
  int curWeaponA;
  vector <const char *> weaponAList;
  vector <WeaponA*> weaponABought;

  int curWeaponB;
  vector <const char *> weaponBList;
  vector <WeaponB*> weaponBBought;

  Menu *menu;

  Menu *homeMenu;
  Menu *buyAmmoMenu;
  Menu *buyWeaponsMenu;
  Menu *buyUpgradesMenu;
  Menu *shipConfigMenu;
  Menu *loadConfigMenu;

  Menu *playerListMenu;

  LocalPlayer (unsigned int id);
  ~LocalPlayer();
  
  static void saveConfig (char *str);
  vector<WeaponA*>::iterator buyWeaponA (vector<WeaponA*>::iterator i);
  vector<WeaponB*>::iterator buyWeaponB (vector<WeaponB*>::iterator i);
  void updateAction();
  void setWeaponA (int weapon);
  void setWeaponB (int weapon);
  void registerShip (Ship *ship);
  void setPermanentlyDead();

  void advance();

  // menu callback functions
  static void updateShipConfig (Menu *menu);
  static void loadConfig (Menu *menu);
  static void requestConfigFilename (Menu *menu);
  static void shopAmmo (Menu *menu);
  static void shopWeaponA (Menu *menu);
  static void shopWeaponB (Menu *menu);
  static void upgradeStatus (Menu *menu);
  static void displayWeaponPrice (Menu *menu);
  static void shopUpgradeAction (Menu *menu);
  static void displayInventory (Menu *menu);
  static void changeFocus (Menu *menu);
};

#endif

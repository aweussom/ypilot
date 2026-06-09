/*
  weapon.h
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
// Classes implementing weapon logic
//
///////////////////////////////////////////////////////////////////////////////

#ifndef WEAPON_H
#define WEAPON_H

#include "retrorocket.h"

enum Weapons {
  WEAPONA_STREAM = 1,
  WEAPONA_SPRAY = 2, 
  WEAPONA_DOUBLE = 4,
  WEAPONA_TRIPPLE = 8,

  WEAPONB_SINGLE = 1,
  WEAPONB_BUCK = 2,
  WEAPONB_SEEKER = 4,
  WEAPONB_MINE = 8,
  WEAPONB_RESOURCE_NODE = 16,
  WEAPONB_SENTRYGUN = 32,
  WEAPONB_HEATSENTRYGUN = 64,
  WEAPONB_THRUSTSINGLE = 128,
  WEAPONB_BOMB = 256,
  WEAPONB_ASM = 512,
};

///////////////////////////////////////////////////////////////////////////////

/** Base class for all weapons */
class Weapon {
public:
  Player *owner;
  Fix32 price;
  int volume_shot;
  int particle_damage;
  int type;

  int weaponstate;

  Weapon();
  virtual ~Weapon() {}
  virtual const char *name() = 0;
  virtual const char *fullName() = 0;
  virtual void shoot() = 0;
};

///////////////////////////////////////////////////////////////////////////////

/** Base class for all weapons of type A (primary weapon) */
class WeaponA : public Weapon {

protected:
  int reloadtime;
  int weapon_a_refill_rate;

public:
  static Fix32 ammoPrice;
  static int quanta;
  
  WeaponA (Player *owner);
  virtual ~WeaponA() {}
  virtual void reload();
  virtual void init();
  virtual void shoot();
};

///////////////////////////////////////////////////////////////////////////////

/** Weapon shooting single stream of particles */
class SingleStream : public WeaponA {
public:
  SingleStream (Player *owner);
  virtual void shoot();
  const char *name() { return "Stream"; }
  const char *fullName() { return "Weapon A: Stream"; }
};

///////////////////////////////////////////////////////////////////////////////

/** Weapon shooting spraying stream of particles */
class SprayStream : public WeaponA {
  int spraystream_spread;

public:
  SprayStream (Player *owner);
  virtual void shoot();
  const char *name() { return "Spray"; } 
  const char *fullName() { return "Weapon A: Spray"; }
};

///////////////////////////////////////////////////////////////////////////////

/** Weapon shooting double stream of particles */
class DoubleStream : public WeaponA {

public:
  DoubleStream (Player *owner);
  virtual void shoot();
  virtual void init();
  const char *name() { return "Double"; }
  const char *fullName() { return "Weapon A: Double"; }
};

///////////////////////////////////////////////////////////////////////////////

/** Weapon shooting stream of triplets of particles */
class TriStream : public WeaponA {
public:
  TriStream (Player *owner);
  virtual void shoot();
  virtual void init();
  const char *name() { return "Triple"; }
  const char *fullName() { return "Weapon A: Tripple"; }
};

///////////////////////////////////////////////////////////////////////////////

/** Base class for all weapons of type B (single shot weapon) */
class WeaponB : public Weapon {

protected:
  int reloadtime;
  int weapon_b_refill_rate;
  int oneshot_damage;

public:
  int inventory;
  Fix32 ammoPrice;
  int quanta;

  WeaponB (Player *owner);
  virtual ~WeaponB() {}
  void reload();
  void init();
  virtual void shoot();
};

///////////////////////////////////////////////////////////////////////////////

/** Weapon shooting single shots of high damage particles */
class ThrustSingleShot : public WeaponB {
 public:
  ThrustSingleShot (Player *owner);
  virtual void shoot();
  const char *name() { return "Single-MK2"; }
  const char *fullName() { return "Weapon B: Single MK-2"; }
};

///////////////////////////////////////////////////////////////////////////////

/** Weapon shooting single shots of high damage particles */
class SingleShot : public WeaponB {
  int singleshot_armtime;

public:
  SingleShot (Player *owner);
  virtual void shoot();
  const char *name() { return "Single"; }
  const char *fullName() { return "Weapon B: Single"; }
};

///////////////////////////////////////////////////////////////////////////////

/** Weapon plating a mine */
class MineShot : public WeaponB {
 public:
  MineShot (Player *owner);
  virtual void shoot();
  const char *name() { return "Mine"; }
  const char *fullName() { return "Weapon B: Mine"; }
};

///////////////////////////////////////////////////////////////////////////////

/** Weapon shooting a buck shot of particles */
class BuckShot : public WeaponB {
  Fix32 buckshot_spread_speed;
  int buckshot_armtime;

 public:
  BuckShot (Player *owner);
  virtual void shoot();
  const char *name() { return "Buckshot"; }
  const char *fullName() { return "Weapon B: Buckshot"; }
};

///////////////////////////////////////////////////////////////////////////////

/** Weapon shooting single heatseeking shots of high damage particles */
class HeatSeeker : public WeaponB {
  int heatseeker_armtime;

 public:
  HeatSeeker (Player *owner);
  virtual void shoot();
  const char *name() { return "Heatseeker"; }
  const char *fullName() { return "Weapon B: Heatseeker"; }
};

///////////////////////////////////////////////////////////////////////////////

/** Weapon for mounting a sentry gun */
class SentrySeed : public WeaponB {
 public:
  SentrySeed (Player *owner);
  virtual void shoot();
  const char *name() { return "Sentrygun"; }
  const char *fullName() { return "Builder: Sentry Gun"; }
};

///////////////////////////////////////////////////////////////////////////////

/** Weapon for mounting a sentry gun with heat seekers */
class HeatSentrySeed : public WeaponB {
 public:
  HeatSentrySeed (Player *owner);
  virtual void shoot();
  const char *name() { return "Sentrygun-MK2"; }
  const char *fullName() { return "Builder: Sentry Gun MK-2"; }
};

///////////////////////////////////////////////////////////////////////////////

/** Weapon for planting a resource node */
class ResourceNodeSeed : public WeaponB {
 public:
  ResourceNodeSeed (Player *owner);
  virtual void shoot();
  const char *name() { return "ResourceNode"; }
  const char *fullName() { return "Builder: Resource Node"; }
};

///////////////////////////////////////////////////////////////////////////////

/** Very damaging bomb, for taking out platforms in RTS games */
class Bomb : public WeaponB {
  int bomb_damage;

 public:
  Bomb (Player *owner);
  virtual void shoot();
  const char *name() { return "NuclearBomb"; }
  const char *fullName() { return "Weapon B: Nuclear Bomb"; }
};

///////////////////////////////////////////////////////////////////////////////

/** Very damaging missile, for taking out sentries in RTS games */
class ASM : public WeaponB {
  int asm_damage;
  int asm_armtime;

 public:
  ASM (Player *owner);
  virtual void shoot();
  const char *name() { return "ASMissile"; }
  const char *fullName() { return "Weapon B: Air-to-Surface"; }
};

///////////////////////////////////////////////////////////////////////////////



#endif

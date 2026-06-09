/*
  mob.h
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
// Classes for MOBs (moving objects)
// All MOBs except ball are in this file
//
///////////////////////////////////////////////////////////////////////////////

#ifndef MOB_H
#define MOB_H

#include "retrorocket.h"
#include <stdlib.h>

enum Upgrades {
  UPGRADE_WEIGHT,
  UPGRADE_THRUST,
  UPGRADE_SHIP_STRENGTH,
  UPGRADE_SHIP_CAPACITY,
  UPGRADE_BUILDING_STRENGTH,
  UPGRADE_FIRING_RATE,
  UPGRADE_PARTICLE_SPEED,
  UPGRADE_DAMAGE,
  UPGRADE_STEALTH_LEVEL,
  UPGRADE_RADAR_LEVEL,
  UPGRADE_SENTRYGUN_RATE,
  UPGRADE_NODE_GATHER_RATE
};

///////////////////////////////////////////////////////////////////////////////

/** Collection of points that makes up the ship's collision map
 *  Represented in two ways: Array of points and traditional collision map
 */
struct SpriteCollisionPoints {
  RRPoint gunPoint;
  RRPoint gunPointL;
  RRPoint gunPointR;
  RRPoint gunPointB;
  RRPoint *points; // array of non-transparent points
  int collisionPoints;
  bool cmap[SPRITESIZE][SPRITESIZE];  // collision map
};

/** The state of a ship */
struct Shipstate {
  Fix32 x, y, vx, vy, orientation;
  unsigned int particleIDCounter;
  int shootATime;
  int actions;  
  int weaponAState;
  int ammoA;
  int ammoB;
  Fix32 gas;
  int lapTime;
  int bestLapTime;
};

/** The state of a particle */
struct ParticleState {
  Fix32 x, y, vx, vy;
};

///////////////////////////////////////////////////////////////////////////////

class Sob;

/** Base class for all Moving OBjects (MOBs) */
class Mob {

protected:
  SpriteCollisionPoints *spriteCPoints;
  unsigned char *gfx;
  unsigned short *pal;
  int orientationSteps;
  int angles;
  
  Fix32 gravity;
  Fix32 strut_length;
  Fix32 liquid_drag;

  /** Create collision maps and lists of collision points for all
      angles of a rotated mob */
  virtual void initRotation();
  /** rotate mob to given angle */
  virtual void setOrientation (Fix32 o);
  /** find the new position of a sprite pixel after sprite rotation */
  virtual void rotateSpritePixel (Fix32 x, Fix32 y, Fix32 angle,
                                  Fix32 *xx, Fix32 *yy);
  virtual bool getCPixelRel (Fix32 x, Fix32 y);

public:
  SpriteCollisionPoints *scp; // collision points for current angle

  Fix32 orientation; // angle
  Fix32 x;
  Fix32 y;
  Fix32 vx;
  Fix32 vy;

  Fix32 gravityCoeff;
  Fix32 dragCoeff;
  Fix32 drag;
  Fix32 mass;

  bool zoneable; // if zones affect this mob
  
  unsigned int id;
  int spritenum;

  int strength;
  int hitBy;
  int damage;

  Mob *child; // the child (if any) of this mob
              // a child is typically a ball picked up by a ship in thrustgames

  Mob();
  virtual ~Mob() { if (spriteCPoints) free (spriteCPoints); }

  virtual void init (Fix32 x, Fix32 y, Fix32 vx, Fix32 vy,
                     Fix32 drag, Fix32 mass, int damage);
  virtual void init() {}
  virtual int getNumberOfChilds();
  virtual Mob *getLastChild();
  virtual void hide();
  virtual Fix32 getOrientation() { return fromAngleStep (orientationSteps); }
  virtual void advance();
  virtual void recalculateCoefficients(); // call if physics (mass etc) changes
  virtual bool collidesWith (Mob *b);
  virtual bool collidesWith (Sob *sob);
  virtual void advance (int timesteps);
  virtual void plot();

  /** true if given coordinates collides with this mob */
  virtual bool getCPixel (Fix32 xx, Fix32 yy) {
    int xxx = (xx - x).toInt();
    int yyy = (yy - y).toInt();
    if ((xxx < 0) || (xxx >= SPRITESIZE) || (yyy < 0) || (yyy >= SPRITESIZE)) {
      return false;
    } else {
      return scp->cmap[yyy][xxx];
    }
  }

  // position of center
  virtual Fix32 getCenterX() { return x; }
  virtual Fix32 getCenterY() { return y; }

  static int getPixel (u8 *gfx, Fix32 x, Fix32 y);

  Fix32 distanceTo (Mob *mob);
  Fix32 distanceTo (Sob *sob);
};

///////////////////////////////////////////////////////////////////////////////

/** A single particle */
class Particle : public Mob {

private:
  int heatseeker_lifetime;
  int seek_radius;
  Fix32 heatseeker_accel;
  Fix32 asm_accel;
  int mine_lifetime;
  int vbcounter;

  void checkCollision();

public:
  enum ParticleType{
    NORMAL,
    MINE,
    STAR,
    BUCKSHOT,
    HEATSEEKER,
    SENTRYSEED,
    HEATSENTRYSEED,
    RESOURCENODESEED,
    SHIELDSEED,
    ASM
  };
  ParticleType particleType;
  bool active;
  unsigned int id;
  int owner;
  int armTime;
  int lifeTime;
  int mineLastCollisionTime;
  
  Particle();
  void init (Fix32 x, Fix32 y, Fix32 vx, Fix32 vy,
             Fix32 drag, Fix32 mass, int damage, unsigned int id);
  bool getCPixel (Fix32 x, Fix32 y) {
    return (x.toInt() == this->x.toInt()) && (y.toInt() == this->y.toInt());
  }
  void plot() { if (active) Mob::plot(); }
  virtual void advance();

  // set state of this particle (pos etc.)
  void newState(ParticleState particleState);
  void newState(ParticleState particleState, int timesteps); // not used

  ParticleState getState();

  static bool sobsInVicinity (Fix32 x, Fix32 y, int range);
};

///////////////////////////////////////////////////////////////////////////////

class Player;
class WeaponA;
class WeaponB;

/** One player's ship */
class Ship : public Mob {

protected:
  Fix32 rotValue; // how much to rotate each VBL
  Fix32 thrustValue; // how much to thrust each VBL

  int thrustSoundRate;

  int lastWayPoint;
  int lastBShotTime;

  int shipDamage;
  Fix32 gasUsageRate;

  int vblCounter;
  
  // cached values from config file
  Fix32 ship_mass;
  Fix32 gas_mass;
  Fix32 weapon_a_mass;
  Fix32 weapon_b_mass;
  Fix32 platform_friction;
  int rate_speed_factor;
  int rate_delta;
  int rate_idle;
  int volume_idle;
  int volume_speed_factor;
  bool beam_range_exists;
  Fix32 beam_range;
  int gas_refill_rate;
  Fix32 gas_price;
  int strength_refill_rate;
  int base_income_rate;
  int empty_gas_die_time;

  void checkCollision();
  void checkLanded();
  void updateThrustSound();
  void checkLaptime();

  void refill();
  void refillGas();
  void refillStrength();
  void refillWeaponA();
  void refillWeaponB();
  void recalculateCoefficients();
  

public:
  void setOrientation (Fix32 o);
  Player *player; // this ships player
  Fix32 getThrustValue(){return thrustValue;}
  WeaponA *weaponA;
  WeaponB *weaponB;
  int roundsA;
  int roundsB;

  Fix32 price;
  int emptyGasCounter;
  Fix32 gunPointX, gunPointY;
  int lapTime;
  bool bestLapMade;
  Fix32 gunPointBX, gunPointBY, gunPointRX, gunPointRY, gunPointLX, gunPointLY;
  unsigned int particleIDCounter;
  Fix32 prevX, prevY;

  int shootATime;

  // set of all ships this ship should be listed as "draw" with
  // (lost their last life at the same time)
  // used for ranking calculations
  set<int> drawWith;

  Ship (Player *player);
  virtual ~Ship();

  void init();
  void resurrect();
  void rotateLeft();
  void rotateRight();
  void stopRotate() { rotValue = 0; }
  void giveThrust();
  void stopThrust() { thrustValue = 0; }
  void shootA();
  void shootB();
  void beam();
  static void getClosestShip (Ship **ship, Fix32 *distance, Fix32 x, Fix32 y);
  static void getClosestEnemyShip (Ship **ship, int friendID,
                                   Fix32 *distance, Fix32 x, Fix32 y);

  /** creates new particles with this ship as owner */
  Particle *newParticle (Fix32 x, Fix32 y, Fix32 vx, Fix32 vy, int damage);

  virtual void advance();
  /** Ship update from action flags */
  void newState (Shipstate shipstate); 
  /** timesteps is how far in future to predict position */
  void newState (Shipstate shipstate, int timesteps); 
  Shipstate getState ();
  Fix32 getCenterX() { return x + HALF_SPRITESIZE; }
  Fix32 getCenterY() { return y + HALF_SPRITESIZE; }

  /** collect all balls this ship is carrying */
  void collectBalls();
};

///////////////////////////////////////////////////////////////////////////////

/** Class that manages all the particles in game */
class ParticleManager {

  stack<int> unusedParticles;
  list<int> usedParticles;

  list<int>::iterator deleteParticle (list<int>::iterator i);
  unsigned int particleIDCounter;


public:
  vector<Particle*> particles;
  Fix32 particle_drag;
  Fix32 particle_mass;

  ParticleManager();
  void deleteParticle (unsigned int id);
  void newParticleState (unsigned int id, ParticleState particleState);
  Particle *newParticle (Fix32 x, Fix32 y, Fix32 vx, Fix32 vy, int damage,
                         int owner = -1);
  Particle *newParticle (Fix32 x, Fix32 y, Fix32 vx, Fix32 vy, int damage,
                         unsigned int id, int owner);
  void deleteAllParticles();
  /** Deletes particles in pos x,y and returns sum of damage from all
   * these particles
   */
  bool getCollisionDamage (Mob *mob);

  void particleCircle (int radius,
                       int particles, Fix32 x, Fix32 y, Fix32 vx, Fix32 vy,
                       int damage, Fix32 bulletSpeed, int owner = -1);
};

///////////////////////////////////////////////////////////////////////////////

#endif

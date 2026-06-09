/*
  mob.cpp
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

#include "retrorocket.h"

///////////////////////////////////////////////////////////////////////////////

Mob::Mob() {
  id = -1;
  gravity = retrorocket->gameFile->getFloat ("gravity");
  if (retrorocket->gameFile->exists ("beam_range")) {
    strut_length = retrorocket->gameFile->getInt ("strut_length");
  }
  liquid_drag = retrorocket->gameFile->getFloat ("liquid_drag");

  this->spritenum = retrorocket->mobs.size();
  retrorocket->mobs.push_back (this);
  child = NULL;
  hide();
  spriteCPoints = NULL;
  strength = 1;
}

void Mob::recalculateCoefficients() {
  dragCoeff = (Fix32)1 - ((Fix32)1 / ((Fix32)1 + mass * drag));
  gravityCoeff = gravity;
}

void Mob::setOrientation (Fix32 o) {
  if(o>0)
    orientation = (o < DEG360) ? o : (o - DEG360);
  else
    orientation = (o + DEG360);
  orientationSteps = toAngleStep (orientation);
  rotateSprite (spritenum, fromAngleStep (orientationSteps));
  // set current rotated pixels to precalculated values
  scp = &(spriteCPoints[orientationSteps]);
}

void Mob::rotateSpritePixel (Fix32 x, Fix32 y, Fix32 angle,
                             Fix32 *xx, Fix32 *yy) {
  Fix32 xi = x - HALF_SPRITESIZE;
  Fix32 yi = y - HALF_SPRITESIZE;
  rotatePoint (xi, yi, angle, xx, yy);
  *xx += HALF_SPRITESIZE;
  *yy += HALF_SPRITESIZE;
}

void Mob::initRotation() {
  int collisionPoints = 0;

  spriteCPoints = (SpriteCollisionPoints*)malloc (sizeof (SpriteCollisionPoints)
                                                  * angles);
  if (!spriteCPoints) panic ("Out of memory");
  // count number of non-transparent pixels
  for (Fix32 yy = 0; yy < SPRITESIZE; yy++) {
    for (Fix32 xx = 0; xx < SPRITESIZE; xx++) {
      if (getCPixelRel (xx, yy)) collisionPoints++;
    }
  }

  // make collision map for all angles
  for (int i = 0; i < angles; i++) {
    Fix32 angle = i * (DEG360/angles);

    SpriteCollisionPoints *scp = &(spriteCPoints[i]);

    // clear collision map
    for ( int x = 0; x < SPRITESIZE; x++ ) {
      for ( int y = 0; y < SPRITESIZE; y++ ) {
        scp->cmap[y][x] = false;
      }
    }

    scp->collisionPoints = collisionPoints;
    scp->points = (RRPoint*)malloc (sizeof (RRPoint) * collisionPoints);
    if (!scp->points) panic ( "Out of memory" );

    // rotate gunpoints
    rotateSpritePixel (17, 7, angle, &scp->gunPoint.x, &scp->gunPoint.y);
    rotateSpritePixel (0, 7, angle, &scp->gunPointB.x, &scp->gunPointB.y);
    rotateSpritePixel (12, 3, angle, &scp->gunPointL.x, &scp->gunPointL.y);
    rotateSpritePixel (12, 11, angle, &scp->gunPointR.x, &scp->gunPointR.y);

    // rotate all pixels in sprite
    int pcnt = 0;
    for (Fix32 yy = 0; yy < SPRITESIZE; yy++) {
      for (Fix32 xx = 0; xx < SPRITESIZE; xx++) {
        // save in collision point list if pixel is non-transparent
        if (getCPixelRel (xx, yy)) {
          RRPoint p;
          rotateSpritePixel (xx, yy, angle, &p.x, &p.y);

          // make sure no pixel is rotated out of sprite boundaries
          if (p.x < 0) p.x = 0;
          if (p.y < 0) p.y = 0;
          if (p.x >= SPRITESIZE) p.x = SPRITESIZE - 1;
          if (p.y >= SPRITESIZE) p.y = SPRITESIZE - 1;

          scp->points[pcnt] = p;
          scp->cmap[p.y.toUInt()][p.x.toUInt()] = true; // update collision map
          pcnt++;
        }
      }
    }
  }
}

int Mob::getPixel (u8 *gfx, Fix32 x, Fix32 y) {
  Fix32 xi = y;
  Fix32 yi = (Fix32)SPRITESIZE - x - 1;
  int tile;
  if ((xi<0) || (yi<0) || (xi>=SPRITESIZE) || (yi>=SPRITESIZE)) return false;
  if (xi < 8) tile = (yi < 8) ? 0 : 2;
  else tile = ( yi < 8 ) ? 1 : 3;
  return gfx[(tile << 6) + ((yi.toUInt() & 7) << 3)  + (xi.toUInt() & 7)];
}

bool Mob::getCPixelRel (Fix32 x, Fix32 y) {
  return getPixel (gfx, x, y) != 0;
}

bool Mob::collidesWith (Mob *b) {
  Fix32 diffX = x - b->x;
  Fix32 diffY = y - b->y;

  // Test bounding boxes, return immediately if there is no collision
  if (diffX.abs() > SPRITESIZE || diffY.abs() > SPRITESIZE) {
    return false;
  }

  // Bounding boxes collide - check hot spots
  for (int i = 0; i < scp->collisionPoints; i++) {
    if (b->getCPixel (x + scp->points[i].x,
                      y + scp->points[i].y)) {
      return true;
    }
  }
  return false;
}

bool Mob::collidesWith (Sob *sob) {
  // bounding box
  if (((x + SPRITESIZE) < sob->x) ||
      ((x - SPRITESIZE) > (sob->x + (Fix32)8*sob->width)) ||
      ((y + SPRITESIZE) < sob->y) ||
      ((y - SPRITESIZE) > (sob->y + (Fix32)8*sob->height))) {
    return false;
  }

  // bounding boxes collide - check hot spots
  for (int i = 0; i < scp->collisionPoints; i++) {
    if (sob->getCPixel (x + scp->points[i].x,
                        y + scp->points[i].y,
                        damage, id)) {
      return true;
    }
  }
  return false;
}

Fix32 Mob::distanceTo (Mob *mob) {
  Fix32 x = mob->getCenterX() - getCenterX();
  Fix32 y = mob->getCenterY() - getCenterY();
  return vectorLength (x, y);
}

Fix32 Mob::distanceTo (Sob *sob) {
  Fix32 x = (Fix32)sob->getCenterX() - getCenterX();
  Fix32 y = (Fix32)sob->getCenterY() - getCenterY();
  return vectorLength (x, y);
}

void Mob::advance() {
  if (child) {
    // find strut vector
    Fix32 strutX = child->getCenterX() - getCenterX();
    Fix32 strutY = child->getCenterY() - getCenterY();
    // find strut angle
    Fix32 strutAngle = vectorAngleAccurate (strutX, strutY);
    // force strut length by adjusting child pos
    child->x += vectorXCompAccurate (strutAngle, strut_length) - strutX;
    child->y += vectorYCompAccurate (strutAngle, strut_length) - strutY;
    // find velocity in strut direction
    Fix32 vStrutThis = vectorAngleComp (strutAngle, vx, vy);
    Fix32 vStrutChild = vectorAngleComp (strutAngle,
                                                 child->vx, child->vy);
    Fix32 vStrut =
      (vStrutThis * mass + vStrutChild * child->mass) /
      (mass + child->mass);
    // find angle to strut normal
    Fix32 normalAngle = strutAngle + DEG90;
    // find velocity in strut normal direction
    Fix32 vStrutNormalThis = vectorAngleComp (normalAngle, vx, vy);
    Fix32 vStrutNormalChild = vectorAngleComp (normalAngle,
                                                       child->vx, child->vy);
    // update vx and vy
    vx =
      vectorXComp (strutAngle, vStrut) +
      vectorXComp (normalAngle, vStrutNormalThis);
    vy =
      vectorYComp (strutAngle, vStrut) +
      vectorYComp (normalAngle, vStrutNormalThis);
    child->vx = 
      vectorXComp (strutAngle, vStrut) +
      vectorXComp (normalAngle, vStrutNormalChild);
    child->vy =
      vectorYComp (strutAngle, vStrut) +
      vectorYComp (normalAngle, vStrutNormalChild);
  }

  // adjust for zone
  if (zoneable) {
    ZoneSum sum = retrorocket->field->getZoneSum( this );
    vx += sum.forceX;
    vy += sum.forceY;
    if (sum.properties & LIQUID) {
      vx *= liquid_drag;
      vy *= liquid_drag;
    }
  }

  // update pos and speed
  x += vx;
  y += vy;
  vx *= dragCoeff;
  vy *= dragCoeff;
  vy += gravityCoeff;
}

void Mob::init (Fix32 x, Fix32 y, Fix32 vx, Fix32 vy,
                Fix32 drag, Fix32 mass, int damage) {
  this->x = x;
  this->y = y;
  this->vx = vx;
  this->vy = vy;
  this->drag = drag;
  this->mass = mass;
  this->zoneable = false;
  this->damage = damage;
  recalculateCoefficients();
}

void Mob::hide() {
  hideSprite (spritenum);
  if (child) child->hide();
}

void Mob::plot() {
  moveSprite (x, y, spritenum);
}

void Mob::advance (int timesteps) {
  for (int i = 0; i < timesteps; i++) {
    advance();
  }
}

///////////////////////////////////////////////////////////////////////////////

Particle::Particle() : Mob() {
  heatseeker_lifetime = retrorocket->gameFile->getInt("heatseeker_lifetime");
  seek_radius = retrorocket->gameFile->getInt ("seek_radius");
  heatseeker_accel = retrorocket->gameFile->getFloat ("heatseeker_accel");
  if (retrorocket->allowedWeaponB & WEAPONB_ASM) {
    asm_accel = retrorocket->gameFile->getFloat ("asm_accel");
  }
  mine_lifetime = retrorocket->gameFile->getInt ("mine_lifetime");
  active = false;
  createParticleSprite (spritenum);
  vbcounter = 0;
}

void Particle::init (Fix32 x, Fix32 y, Fix32 vx, Fix32 vy,
                     Fix32 drag, Fix32 mass, int damage,
                     unsigned int id) {
  this->id = id;
  this->owner = -1;
  this->particleType = NORMAL;
  this->armTime = 0;
  this->lifeTime = 0;
  this->mineLastCollisionTime = -1;

  Mob::init (x, y, vx, vy, drag, mass, damage);
  active = true;
}

void Particle::checkCollision() {
  Particle *p = this;
  bool deleted = false;

  // collision with sobs
  if (retrorocket->sobs.size()) { // there are sobs
    if (retrorocket->field->getSOBCPixel (p->x, p->y)) { // coll. in sob layer
      for (unsigned int s = 0; s < retrorocket->sobs.size(); s++) {
        Sob* sob = retrorocket->sobs[s];

        // do no damage if not master
        int damage = retrorocket->network->master ? p->damage : 0;

        if (sob->getCPixel (p->x, p->y, damage, owner)) {
          if ((p->particleType == Particle::RESOURCENODESEED) &&
              (sob->sobType == Sob::RESOURCE) && retrorocket->network->master) {
            ResourceNode *r = new ResourceNode (sob->x, sob->y-16, p->owner);
            r->init();
            retrorocket->network->sendSobCreatePacket (r);
          } else {
            if ((sob->sobType == Sob::BUTTON) && (owner != -1)) {
              // open all doors
              for (unsigned int s = 0; s < retrorocket->sobs.size(); s++) {
                Sob *sob = retrorocket->sobs[s];
                if (sob->sobType == Sob::DOOR) {
                  ((Door*)sob)->openDoor();
                }
              }
            }
            playSound (retrorocket->particleSound,
                       retrorocket->particleSoundSize,
                       retrorocket->config->getInt("volume_particle_sob"),
                       x, y, retrorocket->config->getInt("rate_particle_sob"));
          }
          retrorocket->manager->deleteParticle (id);
          deleted = true;
          break;
        }
      }
    }
  }

  if (!deleted) {
    // collision with background
    if (retrorocket->field->getCPixel (p->x, p->y)) {
      if (retrorocket->network->master) {
        if (((p->particleType == Particle::SENTRYSEED) ||
             (p->particleType == Particle::HEATSENTRYSEED))) {
            
          Fix32 angle = vectorAngle (p->vx, p->vy);
          angle = angle - DEG180;       // vri 180 grader
          if (angle < 0)
            angle += DEG360;
          Fix32 xx, yy;
          // align to tile grid in inverse direction of shot
          xx = (p->vx >= 0) ?
            p->x - (p->x % 8) :
            p->x + 8 - (p->x % 8);
          yy = (p->vy >= 0) ?
            p->y - (p->y % 8) :
            p->y + 8 - (p->y % 8);

          if (!sobsInVicinity(xx, yy,
                              retrorocket->gameFile->getInt
                              ("sob_vicinity_range"))){
            SentryGun *s;
            if (p->particleType == Particle::SENTRYSEED) {
              s = new SentryGun (xx-8, yy-8, angle, owner, SentryGun::RANDOM);
            } else {
              s = new SentryGun (xx-8, yy-8, angle, owner,
                                 SentryGun::HEATSEEKER);
            }
            s->init();
            retrorocket->network->sendSobCreatePacket (s);
          }
        }
      }
      if (p->particleType == Particle::MINE) {

        if (mineLastCollisionTime == (lifeTime -1)) {
          // stuck in vertical structure
          retrorocket->manager->deleteParticle (id);
        } else if (p->vy > 0) { // hitting ground
          p->vx = 0;
          p->vy = -retrorocket->gameFile->getFloat ("mine_bounce_speed");
        } else { // hitting ceiling
          p->vy = -p->vy;
          p->vx = -p->vx;
        }
        mineLastCollisionTime = lifeTime;

      } else { // standard particle
        playSound(retrorocket->particleSound, retrorocket->particleSoundSize,
                  retrorocket->config->getInt("volume_particle_bg"), x, y);
        retrorocket->manager->deleteParticle (id);
      }
    }
  }
}

void Particle::advance() {
  if (active) {
    lifeTime++;

    checkCollision();

    // do specific actions based on type
    switch (particleType) {

      case HEATSEEKER: {
        if (lifeTime >= heatseeker_lifetime) {
          int d = retrorocket->gameFile->getInt ("particle_damage");
          int s = retrorocket->gameFile->getInt ("star_spread_speed");
          retrorocket->manager->particleCircle (0, 12, x, y, 0, 0, d, s, owner);
          retrorocket->manager->newParticle (x, y, 0, 0, d, owner);

          playSound (retrorocket->deadSound, retrorocket->deadSoundSize,
                     retrorocket->config->getInt("volume_dead"), x, y);    
          retrorocket->manager->deleteParticle(id);
        } else if (!armTime) {
          // blink particle
          if (!(lifeTime | 8)) {
            // TODO: visible
          } else {
            // TODO: invisible
          }
          // find closest ship
          Fix32 minDistance = Fix32::HUGEVAL;
          Ship *target = NULL;
          for (unsigned int i = 0; i < retrorocket->ships.size(); i++) {
            Ship *ship = retrorocket->ships[i];
            if (!ship->player->dead) {
              Fix32 distance = distanceTo (ship);
              if (distance < minDistance) {
                minDistance = distance;
                target = ship;
              }
            }
          }
          // attract to closest ship
          if (minDistance < seek_radius) {
            // find target vector
            Fix32 tx = target->getCenterX() - getCenterX();
            Fix32 ty = target->getCenterY() - getCenterY();
            // find angle of target vector
            Fix32 targetAngle = vectorAngle (tx, ty);
            // find acceleration vector towards target
            Fix32 accX = vectorXComp (targetAngle, heatseeker_accel);
            Fix32 accY = vectorYComp (targetAngle, heatseeker_accel);
            // adjust speed towards target
            vx += accX;
            vy += accY;
            // send network packet
            vbcounter++;
            if (target->player->isLocalPlayer() &&
                (target->player->newActions != target->player->oldActions)) {
              retrorocket->network->sendHeatSeekerPacket(this);
              vbcounter = 0;
            }
            if (target->player->isLocalPlayer() &&
                (vbcounter > retrorocket->updatefrequency )) {
              retrorocket->network->sendHeatSeekerPacket(this);
              vbcounter = 0;
            }
          }
        } else {
          armTime--;
        }
        break;
      }

      case ASM: {
        if (lifeTime >= heatseeker_lifetime) {
          int d = retrorocket->gameFile->getInt ("particle_damage");
          int s = retrorocket->gameFile->getInt ("star_spread_speed");
          retrorocket->manager->particleCircle (0, 12, x, y, 0, 0, d, s, owner);
          retrorocket->manager->newParticle (x, y, 0, 0, d, owner);

          playSound (retrorocket->deadSound, retrorocket->deadSoundSize,
                     retrorocket->config->getInt("volume_dead"), x, y);    
          retrorocket->manager->deleteParticle(id);
          // find closest SOB 
        } else if (!armTime) {
          Fix32 minDistance = Fix32::HUGEVAL;
          Sob *target = NULL;
          for (unsigned int i = 0; i < retrorocket->sobs.size(); i++) {
            Sob *sob = retrorocket->sobs[i];
            if (sob->sobType == Sob::NODE || sob->sobType == Sob::SENTRY ||
                sob->sobType==Sob::PLATFORM) {
              Fix32 distance = distanceTo (sob);
              if (distance < minDistance) {
                minDistance = distance;
                target = sob;
              }
            }
          }
          // attract to closest ship
          if (minDistance < seek_radius) {
            // find target vector
            Fix32 tx = target->getCenterX() - getCenterX();
            Fix32 ty = target->getCenterY() - getCenterY();
            // find angle of target vector
            Fix32 targetAngle = vectorAngle (tx, ty);
            // find acceleration vector towards target
            Fix32 accX = vectorXComp (targetAngle, asm_accel);
            Fix32 accY = vectorYComp (targetAngle, asm_accel);
            // adjust speed towards target
            vx += accX;
            vy += accY;
            // send network packet
            vbcounter++;
          }
        } else {
          armTime--;
        }
        break;
      }

      case STAR: {
        if (!armTime--) {
          int d = retrorocket->gameFile->getInt ("particle_damage");
          int s = retrorocket->gameFile->getInt ("star_spread_speed");
          retrorocket->manager->particleCircle (0, 12, x, y, 0, 0, d, s, owner);
          retrorocket->manager->newParticle (x, y, 0, 0, d, owner);

          playSound (retrorocket->deadSound, retrorocket->deadSoundSize,
                     retrorocket->config->getInt("volume_dead"), x, y);    
          retrorocket->manager->deleteParticle(id);
        }
        break;
      }

      case MINE: {
        if (lifeTime >= mine_lifetime) {
          retrorocket->manager->deleteParticle(id);
        }
        break;
      }

      case BUCKSHOT: {
        if (!armTime--) {
          int d = ((Fix32)(retrorocket->gameFile->getInt ("particle_damage")) *
                   retrorocket->players[owner]->damageScaler).toInt();
          Fix32 s = retrorocket->gameFile->getFloat ("buckshot_spread_speed");
          retrorocket->manager->particleCircle (0, 4, x, y, vx, vy, d, s,
                                                owner);
          retrorocket->manager->particleCircle (0, 16, x, y, vx, vy, d,
                                                (Fix32)2*s, owner);
          retrorocket->manager->deleteParticle(id);
        }
        break;
      }

      default:
        break;
    }

    // update pos and speed
    Mob::advance();
  }
}

void Particle::newState (ParticleState particleState) {
  newState (particleState, 0);
}

void Particle::newState (ParticleState particleState, int timesteps) {
  this->x = particleState.x;
  this->y = particleState.y;
  this->vx = particleState.vx;
  this->vy = particleState.vy;
  if (timesteps) Mob::advance (timesteps);
}

ParticleState Particle::getState() {
  ParticleState particleState;
  particleState.x = x;
  particleState.y = y;
  particleState.vx = vx;
  particleState.vy = vy;
  return(particleState);
}

bool Particle::sobsInVicinity (Fix32 x, Fix32 y, int range) {

  
  for (unsigned int s = 0; s < retrorocket->sobs.size(); s++) {
    Sob* sob = retrorocket->sobs[s];
    if (((sob->getCenterX() - x).abs() < range) &&
        ((sob->getCenterY() - y).abs() < range)) {
      return true;
    }
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////

ParticleManager::ParticleManager() {
  particle_drag = retrorocket->gameFile->getFloat ("particle_drag");
  particle_mass = retrorocket->gameFile->getFloat ("particle_mass");

  // create all particle objects
  // particles are never deleted during play, just hidden when not in use

  int numberOfParticles = MAX_SPRITES - retrorocket->mobs.size();

  for (int i = 0; i < numberOfParticles; i++) {
    Particle *p = new Particle();
    particles.push_back (p);
    unusedParticles.push (i);
    p->hide();
  }
  particleIDCounter = 0;
}

Particle *ParticleManager::newParticle (Fix32 x, Fix32 y, Fix32 vx,
                                        Fix32 vy, int damage, int owner) {
  return newParticle (x, y, vx, vy, damage, particleIDCounter++, owner);
}

Particle *ParticleManager::newParticle (Fix32 x, Fix32 y, Fix32 vx,
                                        Fix32 vy, int damage,
                                        unsigned int id, int owner) {
  int particleNum;

  if (unusedParticles.size()) {
    particleNum = unusedParticles.top();
    unusedParticles.pop();
    usedParticles.push_back (particleNum);
  } else {
    // no unused particles left, reuse the oldest one instead
    particleNum = *usedParticles.begin();
    usedParticles.erase (usedParticles.begin());
    usedParticles.push_back (particleNum);
  }

  particles[particleNum]->init (x, y, vx, vy,
                                particle_drag, particle_mass, damage, id);
  particles[particleNum]->owner = owner;

  return particles[particleNum];
}

void ParticleManager::deleteParticle (unsigned int id) {
  list<int>::iterator i = usedParticles.begin();
  while (i != usedParticles.end()) {
    Particle *p = particles[*i];
    if (p->id == id) {
      deleteParticle (i);
      return;
    }
    i++;
  }  
}

void ParticleManager::newParticleState (unsigned int id,
                                        ParticleState particleState) {
  list<int>::iterator i = usedParticles.begin();
  while (i != usedParticles.end()) {
    Particle *p = particles[*i];
    if (p->id == id) {
      p->newState (particleState);
      return;
    }
    i++;
  }  
}

list<int>::iterator ParticleManager::deleteParticle (list<int>::iterator i) {
  Particle *p = particles[*i];
  unusedParticles.push (*i);
  p->hide();
  p->active = false;
  return usedParticles.erase (i);
}

bool ParticleManager::getCollisionDamage (Mob *mob) {
  Fix32 vvxx = mob->vx;
  Fix32 vvyy = mob->vy;
  int damage = 0;

  int killerID = -1;

  list<int>::iterator i = usedParticles.begin();
  while (i != usedParticles.end()) {
    Particle *p = particles[*i];
    if (mob->collidesWith (p)) {
      damage += p->damage;
      if (p->owner >= 0) killerID = p->owner;
      i = deleteParticle (i);
      retrorocket->network->sendDeleteParticlePacket (p->id);
      mob->vx += (p->vx - vvxx) / (mob->mass / p->mass).sqroot();
      mob->vy += (p->vy - vvyy) / (mob->mass / p->mass).sqroot();
    } else {
      i++;
    }
  }
  mob->hitBy = killerID;
  mob->strength -= damage;

  return damage > 0;
}

void ParticleManager::particleCircle (int radius, int particles, 
                                      Fix32 x, Fix32 y, Fix32 vx, Fix32 vy,
                                      int damage, Fix32 bulletSpeed,
                                      int owner) {
  for (int i = 0; i < particles; i++) {
    Fix32 alfa = ((Fix32)i / (Fix32)particles) * DEG360;
    Fix32 a = (Fix32)DEG90 - alfa;
    if (a < 0) a += DEG360;
    if (a >= DEG360) a -= DEG360;
    Fix32 vvxx = vx + bulletSpeed * a.cosinus_ds();
    Fix32 vvyy = vy - bulletSpeed * a.sinus_ds();

    Fix32 x_i, y_i;
    rotatePoint (0, -radius, alfa, &x_i, &y_i);

    retrorocket->manager->newParticle (x + x_i, y + y_i, vvxx, vvyy, damage,
                                       owner);
  }
}

void ParticleManager::deleteAllParticles() {
  list<int>::iterator i = usedParticles.begin();
  while (i != usedParticles.end()) {
    i = deleteParticle (i);
  }
}

///////////////////////////////////////////////////////////////////////////////

void Ship::getClosestShip(Ship **ship, Fix32 *distance, Fix32 x, Fix32 y) {
  *distance = Fix32::HUGEVAL;
  *ship = NULL;
  for (unsigned int i = 0; i < retrorocket->ships.size(); i++) {
    Ship *currentShip = retrorocket->ships[i];
    if (!currentShip->player->dead) {
      Fix32 xx = x - currentShip->getCenterX();
      Fix32 yy = y - currentShip->getCenterY();
      Fix32 currentDistance = vectorLength (xx, yy);
      if (currentDistance < *distance) {
        *distance = currentDistance;
        *ship = currentShip;
      }
    }
  }
}

void Ship::getClosestEnemyShip(Ship **ship, int friendID, Fix32 *distance, Fix32 x, Fix32 y) {
  *distance = Fix32::HUGEVAL;
  *ship = NULL;
  for (unsigned int i = 0; i < retrorocket->ships.size(); i++) {
    if (i != (unsigned int) friendID) {
      Ship *currentShip = retrorocket->ships[i];
      if (!currentShip->player->dead) {
        Fix32 xx = x - currentShip->getCenterX();
        Fix32 yy = y - currentShip->getCenterY();
        Fix32 currentDistance = vectorLength (xx, yy);
        if (currentDistance < *distance) {
          *distance = currentDistance;
          *ship = currentShip;
        }
      }
    }
  }
}

Ship::Ship (Player *player) : Mob() {
  ship_mass = retrorocket->gameFile->getFloat ("ship_mass");
  gas_mass = retrorocket->gameFile->getFloat ("gas_mass");
  weapon_a_mass = retrorocket->gameFile->getFloat ("weapon_a_mass");
  weapon_b_mass = retrorocket->gameFile->getFloat ("weapon_b_mass");
  if (retrorocket->gameFile->exists ("platform")) {
    platform_friction = retrorocket->gameFile->getFloat ("platform_friction");
  }
  rate_speed_factor = retrorocket->config->getInt("rate_speed_factor");
  rate_delta = retrorocket->config->getInt("rate_delta");
  rate_idle = retrorocket->config->getInt("rate_idle");

  volume_idle = retrorocket->config->getInt("volume_idle");

  volume_speed_factor = retrorocket->config->getInt("volume_speed_factor");
  beam_range_exists = retrorocket->gameFile->exists ("beam_range");
  gas_refill_rate = retrorocket->gameFile->getInt ("gas_refill_rate");
  strength_refill_rate = retrorocket->gameFile->getInt ("strength_refill_rate");
  empty_gas_die_time = retrorocket->gameFile->getInt ("empty_gas_die_time");
  if (beam_range_exists) {
    beam_range = retrorocket->gameFile->getFloat ("beam_range");
  }
  if (retrorocket->rtsMode) {
    gas_price = retrorocket->gameFile->getFloat ("gas_price");
    base_income_rate = retrorocket->gameFile->getInt ("base_income_rate");
  }

  this->player = player;

  angles = ANGLES;

  this->id = retrorocket->ships.size();
  retrorocket->ships.push_back (this);

  drag = retrorocket->gameFile->getFloat ("ship_drag");
  gasUsageRate = retrorocket->gameFile->getFloat ("ship_gas_usage_rate");

  // read bitmap for ship
  u8 *tmp;
  readBinary ((u8**)&tmp, retrorocket->gameFile->getString ("ship_bitmap"));
  gfx = (u8*)malloc (SPRITESIZE * SPRITESIZE);
  if (!gfx) panic ("Out of memory");
  // convert bitmap to DS sprite gfx data
  for (int y = 0; y < SPRITESIZE; y++) {
    for (int x = 0; x < SPRITESIZE; x++) {
      int tile;
      if (x < 8) tile = (y < 8) ? 0 : 2;
      else tile = (y < 8) ? 1 : 3;
      gfx[(tile << 6) + ((y & 7) << 3)  + (x & 7)] = tmp[y*SPRITESIZE+x];
    }
  }
  free (tmp);

  ConfigContext configContext;
  if (retrorocket->gameFile->findID ("ship_price", &configContext)) {
    price = retrorocket->gameFile->getInt (&configContext);
  } else {
    price = 0;
  }

  shipDamage = retrorocket->gameFile->getInt ("ship_damage");

  createSprite ((void*)(retrorocket->playerPals[id]), (void*)gfx, spritenum);

  initRotation();
  setSpriteRotEnable (spritenum);
  particleIDCounter = 0;

  player->registerShip (this);
}

Ship::~Ship() {
  if (player->isLocalPlayer()) {
    modifySoundThrust(0,0);
  }

  for (int i = 0; i < angles; i++) {
    SpriteCollisionPoints *scp = &(spriteCPoints[i]);
    free (scp->points);
  }

  free (gfx);
}

void Ship::rotateLeft() {
  rotValue = (Fix32)DEG360 - player->rotSpeed;
}

void Ship::rotateRight() {
  rotValue = player->rotSpeed;
}

void Ship::init() {
  roundsA = 0;
  roundsB = 0;
  recalculateCoefficients();
  bestLapMade = false;

  Fix32 tmpGas = player->gas; // to make sure gas is kept between thrust levels
  resurrect();
  player->gas = tmpGas;
}

void Ship::resurrect() {
  thrustSoundRate = retrorocket->config->getInt("rate_idle");
  RRPoint start = retrorocket->field->getStart (id);
  setOrientation (3*DEG90);
  rotValue = 0.0f;
  thrustValue = 0.0f;
  player->oldActions = player->newActions = 0;
  player->dead = false;
  Mob::init (start.x, start.y, 0, 0, drag, mass, shipDamage);
  prevX = getCenterX();
  prevY = getCenterY();

  strength = player->maxStrength;

  if (!retrorocket->rtsMode) {
    player->gas = player->userMaxGas;
    roundsA = player->userMaxWeaponA;
    roundsB = player->userMaxWeaponB;
    recalculateCoefficients();
  }

  zoneable = true;
  lastWayPoint = 0;
  lapTime = 0;
  lastBShotTime = 0;
  
  if (child) {
    child->init();
    //    if (id == retrorocket->localShip) ((Ball*)child)->sendBallPacket();
  }
  child = NULL;
  // Clear "killed by" line
  if (id == retrorocket->localShip) clearText(ROWS/2, ROWS/2);
}

void Ship::newState (Shipstate shipstate) {
  newState (shipstate, 0);
}

void Ship::newState (Shipstate shipstate, int timesteps) {
  this->x = shipstate.x;
  this->y = shipstate.y;
  this->vx = shipstate.vx;
  this->vy = shipstate.vy;
  if (this->weaponA) {
    this->weaponA->weaponstate = shipstate.weaponAState;
  }
  if (this->particleIDCounter > shipstate.particleIDCounter) {
    for (unsigned int pid = shipstate.particleIDCounter;
         pid < this->particleIDCounter;
         pid++) {
      retrorocket->manager->deleteParticle(SHIPID | pid);
      // Must have been weapon A, weapon B is acked.
      player->weaponAInventory++;
    }
  }
  this->particleIDCounter = shipstate.particleIDCounter;
  setOrientation (shipstate.orientation);
  if (timesteps) Mob::advance (timesteps);
  player->newActions = shipstate.actions;
  this->shootATime = shipstate.shootATime;
  player->updateAction();
  this->roundsA = shipstate.ammoA;
  this->roundsB = shipstate.ammoB;
  this->lapTime = shipstate.lapTime;
  player->gas = shipstate.gas;
  player->bestLapTime = shipstate.bestLapTime;
}

Shipstate Ship::getState () {
  Shipstate shipstate;
  shipstate.x = x, shipstate.y = y;
  shipstate.vx = vx, shipstate.vy = vy;
  shipstate.orientation = orientation;
  shipstate.actions = player->newActions;
  shipstate.particleIDCounter = particleIDCounter;
  shipstate.shootATime = shootATime;
  if (weaponA != NULL) {
    shipstate.weaponAState = weaponA->weaponstate;
  }
  shipstate.ammoA = roundsA;
  shipstate.ammoB = roundsB;
  shipstate.lapTime = lapTime;
  shipstate.gas = player->gas;
  shipstate.bestLapTime = player->bestLapTime;
  return shipstate;
}

void Ship::giveThrust() {
  if ((player->gas > 0) && !player->dead) {
    thrustValue = player->thrust / mass;
    player->gas -= gasUsageRate;
    if (player->gas < 0) player->gas = 0;
  }
  else thrustValue = 0;
}

Mob *Mob::getLastChild() {
  if (child) {
    return child->getLastChild();
  }
  return this;
}

void Ship::collectBalls() {
  if (child) {
    ((Ball*)child)->collectBalls();
    child = NULL;
  }
}

int Mob::getNumberOfChilds() {
  if (child) {
    return child->getNumberOfChilds() + 1;
  }
  return 0;
}

void Ship::beam() {
  if (beam_range_exists) {
    for (unsigned int i = 0; i < retrorocket->balls.size(); i++) {
      Ball *ball = retrorocket->balls[i];
      if (!ball->active) {
        Mob *lastChild = getLastChild();
        Fix32 distanceToBall = lastChild->distanceTo (ball);

        if ((distanceToBall < beam_range) && (distanceToBall >= strut_length)) {
          lastChild->child = ball;
          ball->release (id);
          retrorocket->network->sendPickUpBallPacket(this, ball);
        }
      }
    }

    for (unsigned int i = 0; i < retrorocket->sobs.size(); i++) {
      Sob *sob = retrorocket->sobs[i];
      Fix32 distanceToSOB = distanceTo (sob);

      if (distanceToSOB < beam_range) {
        sob->performBeamAction (this);
        break;
      }
    }
  }
}

void Ship::shootA() {
  if ((weaponA) && !(shootATime % (int)player->autoFireRate)) {
    weaponA->shoot();
  }
  shootATime++;
}

void Ship::shootB() {
  if (weaponB && (lastBShotTime >= player->repetitionRate)) {
    weaponB->shoot();
    lastBShotTime = 0;
  }
}

void Ship::refill() {
  refillGas();
  refillStrength();
  refillWeaponA();
  refillWeaponB();
}

void Ship::refillGas() {
  emptyGasCounter = 0;

  if (player->gas < player->userMaxGas) {
    if (retrorocket->rtsMode) {
      if (player->credits >= gas_price) {
        player->gas += gas_refill_rate;
        player->credits -= gas_price;
      }
    } else {
      player->gas += gas_refill_rate;
    }
  }
  if (player->gas > player->userMaxGas) player->gas = player->userMaxGas;
}

void Ship::refillStrength() {
  if (strength < player->maxStrength) {
    strength += strength_refill_rate;
    if (strength > player->maxStrength) strength = player->maxStrength;
  }
}

void Ship::refillWeaponA() {
  if (weaponA) weaponA->reload();
}
void Ship::refillWeaponB() {
  if (weaponB) weaponB->reload();
}

Particle *Ship::newParticle (Fix32 x, Fix32 y, Fix32 vx, Fix32 vy, int damage) {
  Particle *p = retrorocket->manager->newParticle (x, y, vx, vy, damage,
                                            SHIPID | particleIDCounter++);
  p->owner = id;
  return p;
}

void Ship::recalculateCoefficients() {
  mass = ship_mass + player->gas * gas_mass;

  if (weaponA) mass += (Fix32)roundsA * weapon_a_mass;
  if (weaponB) mass += (Fix32)roundsB * weapon_b_mass;

  mass *= player->weightFactor;

  Mob::recalculateCoefficients();
}

void Ship::checkCollision() {
  if (player->isLocalPlayer()) {
    bool collision = false;

    if (!player->dead) {

      // check child collisions
      if (child) {
        if (child->strength <= 0) {
          collision = true;
          hitBy = child->hitBy;
          strength = 0;
        }
      }

      // particles
      if (!player->immortal) {
        if (retrorocket->manager->getCollisionDamage (this)) {
          collision = true;
          playSound(retrorocket->particleSound, retrorocket->particleSoundSize,
                    retrorocket->config->getInt("volume_particle_ship"),
                    x, y, retrorocket->config->getInt("rate_particle_ship"));
        }
      }

      // sobs
      for (unsigned int s = 0; s < retrorocket->sobs.size(); s++) {
        if (collidesWith (retrorocket->sobs[s])) {
          strength = 0;
          hitBy = -1;
          collision = true;
          break;
        }
      }

      // Background
      for (int i = 0; i < scp->collisionPoints; i++) {
        Fix32 x_i = scp->points[i].x;
        Fix32 y_i = scp->points[i].y;

        if (retrorocket->field->getCPixel ( x + x_i, y + y_i )) {
          strength = 0;
          hitBy = -1;
          collision = true;
        }
      }
    
      // check ball collision
      for (unsigned int j = 0; j < retrorocket->balls.size(); j++) {
        if (collidesWith (retrorocket->balls[j])) {
          strength = 0;
          hitBy = retrorocket->balls[j]->owner;
          if (hitBy == (int)id) hitBy = -1;
          collision = true;
        }
      }
    }

    if (collision) retrorocket->network->sendStrengthUpdatePacket (this);
  }
}

void Ship::checkLanded() {
  // check if landed on platform, refill if landed
  if (int flags = retrorocket->field->landed (this)) {
    setOrientation (3*DEG90);
    if (vx.abs() > (Fix32)0.01) {
      vx *= (Fix32)1 - ((Fix32)1 / ((Fix32)1 + mass * platform_friction));
    } else {
      vx = 0;
    }
    vy = 0;
    if (!player->dead) {
      if (flags & Field::HOME) {
        refill();
      } else {
        if (flags & Field::FILLINGSTATION) refillGas();
        if (flags & Field::WEAPONA)        refillWeaponA();
        if (flags & Field::WEAPONB)        refillWeaponB();
        if (flags & Field::GARAGE)         refillStrength();
      }
      recalculateCoefficients();
    }
    stopRotate();
  }
}

void Ship::updateThrustSound() {
  // calculate and play thrust sound
  if (retrorocket->ships[retrorocket->localShip] == this) {
    if (player->dead) {
      modifySoundThrust(0,0);
    } else {
      int volume;
      if (thrustValue != 0) { // if thrusting
        Fix32 v = vectorLength (vx, vy);
        int maxRate = (v.abs() * rate_speed_factor + rate_idle).toUInt();
        thrustSoundRate += rate_delta;
        thrustSoundRate = (thrustSoundRate > maxRate) ?
          maxRate : thrustSoundRate;
        volume = volume_idle +
          (thrustSoundRate - rate_idle)/volume_speed_factor;
      } else { // if idle engines
        thrustSoundRate -= rate_delta;
        thrustSoundRate = (thrustSoundRate < rate_idle) ?
          rate_idle : thrustSoundRate;
        int idleVol = 0;
        if (retrorocket->soundSetting == 2) {
          idleVol = volume_idle;
        }
        volume = idleVol + (thrustSoundRate - rate_idle)/volume_speed_factor;
      }
      // play current thrust sound
      volume = (volume > 127) ? 127 : volume;

      modifySoundThrust(thrustSoundRate, volume);
    }
  }
}

void Ship::checkLaptime() {
  // check laptime and waypoints

  int newWayPoint = retrorocket->field->getWayPoint ( this );

  prevX = getCenterX();
  prevY = getCenterY();
      
  if ( lapTime >= player->bestLapTime ) { // reset if not potential best
    lapTime = 0;
    lastWayPoint = 0;
  }
  if (lastWayPoint)
    lapTime++;
      
  if (newWayPoint == 1) { // if it is start
    lapTime = 0;
    lastWayPoint = 1;
  } else if (newWayPoint == (lastWayPoint+1)) { // else if valid next wp
    if (newWayPoint == retrorocket->field->numberWayPoints) { // if goal
      if ((lapTime < player->bestLapTime) && lapTime) { // if new record
        player->bestLapTime = lapTime;
        bestLapMade = true;
        //player->credits += retrorocket->gameFile->getInt ("best_lap_credits");
      }
      // give credits for new lap
      if (retrorocket->rtsMode) {
        player->credits += retrorocket->gameFile->getInt ("lap_credits");
      }
      
      lastWayPoint = 0;
      lapTime = 0;
    } else { // just a new waypoint on the way
      lastWayPoint = newWayPoint;
    }
  }
}

void Ship::advance() {
  updateThrustSound();
  lastBShotTime++;

  // base income
  if (retrorocket->rtsMode) {
    vblCounter++;
    if ((vblCounter % base_income_rate) == 0) {
      player->credits += 1;
    }
  }

  // dies if no gas for long time
  if (!retrorocket->field->landed (this) && (player->gas <= 0)) {
    emptyGasCounter++;
    if (emptyGasCounter > empty_gas_die_time) {
      strength = 0;
    }
  }

  if (player->lives <= 0) {
    hide();  // no more lives, keep sprite hidden

  } else if (player->dead) {
    hide();  // player just died, wait before resurrecting
    player->resurrectTime--;
    if ((player->resurrectTime <= 0) &&
        ((player->credits >= price) || !retrorocket->rtsMode)) {
      resurrect();
      player->immortal = true;
      player->immortalTime = retrorocket->gameFile->getInt ("immortal_time");
      if (retrorocket->rtsMode) player->credits -= price;
    }

  } else {

    if (strength <= 0) {
      // killed
      playSound (retrorocket->deadSound, retrorocket->deadSoundSize,
                 retrorocket->config->getInt("volume_dead"), x, y);    
      player->dead = true;
      strength = 0; // to avoid displaying negative numbers 
      if (!retrorocket->rtsMode) {
        if (retrorocket->gameMode != 1) player->lives--;
        if (hitBy >= 0 && id == retrorocket->localShip) {
          char line[LINESIZE];
          snprintf (line, LINESIZE,
                    "Killed by %s", retrorocket->playerNames[hitBy]);
          setTextColor (RED);
          outputText((COLUMNS - strlen (line)) / 2, ROWS / 2, line);
        }
        if (player->lives <= 0) {
          retrorocket->playerDeathOrder.push_back (id);
          player->setPermanentlyDead();
        }
      }
      player->resurrectTime = retrorocket->gameFile->getInt ("resurrect_time");
      int d = retrorocket->gameFile->getInt ("particle_damage");
      int s = retrorocket->gameFile->getInt ("star_spread_speed");
      retrorocket->manager->particleCircle (4, 16, getCenterX(), getCenterY(),
                                     vx, vy, d, s);
      retrorocket->manager->particleCircle (8, 16, getCenterX(), getCenterY(),
                                     vx, vy, d, 2*s);
      hide();

    } else {

      if (player->immortal && (--player->immortalTime <= 0)) {
        player->immortal = false;
      }

      checkLanded();
      checkCollision();
      checkLaptime();

      // rotate
      setOrientation (orientation + rotValue);  

      // thrust
      if (thrustValue != 0) {
        vx += vectorXComp (getOrientation(), thrustValue);
        vy += vectorYComp (getOrientation(), thrustValue);
      }

      // update speed and position
      Mob::advance();
    }
  }
}

void Ship::setOrientation (Fix32 o) {
  Mob::setOrientation (o);
  gunPointX = scp->gunPoint.x;
  gunPointY = scp->gunPoint.y;
  gunPointBX = scp->gunPointB.x;
  gunPointBY = scp->gunPointB.y;
  gunPointLX = scp->gunPointL.x;
  gunPointLY = scp->gunPointL.y;
  gunPointRX = scp->gunPointR.x;
  gunPointRY = scp->gunPointR.y;
}

///////////////////////////////////////////////////////////////////////////////


/*
  int bomb_damage;
  weapon.cpp
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

Fix32 WeaponA::ammoPrice;
int WeaponA::quanta;

///////////////////////////////////////////////////////////////////////////////

SingleStream::SingleStream (Player *owner) : WeaponA (owner) {
  if (retrorocket->rtsMode) {
    price = retrorocket->gameFile->getInt ("weapon_a_weapon_price");
  } else {
    price = 0;
  }
  type = WEAPONA_STREAM;
}

SprayStream::SprayStream (Player *owner) : WeaponA (owner) {
  spraystream_spread = retrorocket->gameFile->getInt("spraystream_spread"); 
  if (retrorocket->rtsMode) {
    price = retrorocket->gameFile->getInt ("weapon_a_weapon_price");
  } else {
    price = 0;
  }
  type = WEAPONA_SPRAY;
}

DoubleStream::DoubleStream (Player *owner) : WeaponA (owner){
  if (retrorocket->rtsMode) {
    price = retrorocket->gameFile->getInt ("weapon_a_weapon_price");
  } else {
    price = 0;
  }
  type = WEAPONA_DOUBLE;
  init();
}

TriStream::TriStream (Player *owner) : WeaponA (owner){
  if (retrorocket->rtsMode) {
    price = retrorocket->gameFile->getInt ("weapon_a_weapon_price");
  } else {
    price = 0;
  }
  type = WEAPONA_TRIPPLE;
  init();
}

ThrustSingleShot::ThrustSingleShot (Player *owner) : WeaponB (owner) {
  if (retrorocket->rtsMode) {
    ammoPrice = retrorocket->gameFile->getInt ("mono_ammo_price");
    price = retrorocket->gameFile->getInt ("mono_weapon_price");
    quanta = retrorocket->gameFile->getInt ("mono_ammo_quantum");
  } else {
    ammoPrice = 0;
    price = 0;
    quanta = 0;
  }
  type = WEAPONB_THRUSTSINGLE;
}

SingleShot::SingleShot (Player *owner) : WeaponB (owner) {
  singleshot_armtime = retrorocket->gameFile->getInt ("singleshot_armtime");
  if (retrorocket->rtsMode) {
    ammoPrice = retrorocket->gameFile->getInt ("single_ammo_price");
    price = retrorocket->gameFile->getInt ("single_weapon_price");
    quanta = retrorocket->gameFile->getInt ("single_ammo_quantum");
  } else {
    ammoPrice = 0;
    price = 0;
    quanta = 0;
  }
  type = WEAPONB_SINGLE;
}

MineShot::MineShot (Player *owner) : WeaponB (owner) {
  if (retrorocket->rtsMode) {
    ammoPrice = retrorocket->gameFile->getInt ("mine_ammo_price");
    price = retrorocket->gameFile->getInt ("mine_weapon_price");
    quanta = retrorocket->gameFile->getInt ("mine_ammo_quantum");
  } else {
    ammoPrice = 0;
    price = 0;
    quanta = 0;
  }
  type = WEAPONB_MINE;
}

BuckShot::BuckShot (Player *owner) : WeaponB (owner) {
  buckshot_spread_speed =
    retrorocket->gameFile->getFloat("buckshot_spread_speed");
  buckshot_armtime = retrorocket->gameFile->getInt ("buckshot_armtime");
  if (retrorocket->rtsMode) {
    ammoPrice = retrorocket->gameFile->getInt ("buck_ammo_price");
    price = retrorocket->gameFile->getInt ("buck_weapon_price");
    quanta = retrorocket->gameFile->getInt ("buck_ammo_quantum");
  } else {
    ammoPrice = 0;
    price = 0;
    quanta = 0;
  }
  type = WEAPONB_BUCK;
}

HeatSeeker::HeatSeeker (Player *owner) : WeaponB (owner) {
  heatseeker_armtime = retrorocket->gameFile->getInt ("heatseeker_armtime");
  if (retrorocket->rtsMode) {
    ammoPrice = retrorocket->gameFile->getInt ("heat_ammo_price");
    price = retrorocket->gameFile->getInt ("heat_weapon_price");
    quanta = retrorocket->gameFile->getInt ("heat_ammo_quantum");
  } else {
    ammoPrice = 0;
    price = 0;
    quanta = 0;
  }
  type = WEAPONB_SEEKER;
}

SentrySeed::SentrySeed (Player *owner) : WeaponB (owner) {
  if (retrorocket->rtsMode) {
    ammoPrice = retrorocket->gameFile->getInt ("sentry_ammo_price");
    price = retrorocket->gameFile->getInt ("sentry_weapon_price");
    quanta = retrorocket->gameFile->getInt ("sentry_ammo_quantum");
  } else {
    ammoPrice = 0;
    price = 0;
    quanta = 0;
  }
  type = WEAPONB_SENTRYGUN;
}

HeatSentrySeed::HeatSentrySeed (Player *owner) : WeaponB (owner) {
  if (retrorocket->rtsMode) {
    ammoPrice = retrorocket->gameFile->getInt ("heatsentry_ammo_price");
    price = retrorocket->gameFile->getInt ("heatsentry_weapon_price");
    quanta = retrorocket->gameFile->getInt ("heatsentry_ammo_quantum");
  } else {
    ammoPrice = 0;
    price = 0;
    quanta = 0;
  }
  type = WEAPONB_HEATSENTRYGUN;
}

ResourceNodeSeed::ResourceNodeSeed (Player *owner) : WeaponB (owner) {
  if (retrorocket->rtsMode) {
    ammoPrice = retrorocket->gameFile->getInt ("node_ammo_price");
    price = retrorocket->gameFile->getInt ("node_weapon_price");
    quanta = retrorocket->gameFile->getInt ("node_ammo_quantum");
  } else {
    ammoPrice = 0;
    price = 0;
    quanta = 0;
  }
  type = WEAPONB_RESOURCE_NODE;
}

ASM::ASM (Player *owner) : WeaponB (owner) {
  asm_damage = retrorocket->gameFile->getInt ("asm_damage");
  asm_armtime = retrorocket->gameFile->getInt ("asm_armtime");
  if (retrorocket->rtsMode) {
    ammoPrice = retrorocket->gameFile->getInt ("asm_ammo_price");
    price = retrorocket->gameFile->getInt ("asm_weapon_price");
    quanta = retrorocket->gameFile->getInt ("asm_ammo_quantum");
  } else {
    ammoPrice = 0;
    price = 0;
    quanta = 0;
  }
  type = WEAPONB_ASM;
}

///////////////////////////////////////////////////////////////////////////////

Weapon::Weapon() {
  particle_damage = retrorocket->gameFile->getInt ("particle_damage");
  volume_shot = retrorocket->config->getInt("volume_shot");
  weaponstate = 0;
}

void WeaponA::shoot() {
  playSound (retrorocket->shootASound,
             retrorocket->shootASoundSize, volume_shot,
             owner->ship->x, owner->ship->y);
  owner->ship->roundsA--;
  if (retrorocket->rtsMode) {
    owner->weaponAInventory--;
  }
}

void WeaponB::shoot() {
  playSound (retrorocket->shootBSound,
             retrorocket->shootBSoundSize, volume_shot,
             owner->ship->x, owner->ship->y);
  owner->ship->roundsB--;
  if (retrorocket->rtsMode) {
    inventory--;
  }
}

///////////////////////////////////////////////////////////////////////////////

WeaponA::WeaponA (Player *owner) : Weapon() {
  this->owner = owner;
  weapon_a_refill_rate = retrorocket->gameFile->getInt ("weapon_a_refill_rate");
  init();
}

void WeaponA::reload() {
  if (retrorocket->rtsMode &&
      (owner->ship->roundsA >= owner->weaponAInventory) &&
      (owner->isLocalPlayer())) {
    owner->ship->roundsA = owner->weaponAInventory;
    if (owner->ship->roundsA > owner->userMaxWeaponA) {
      owner->ship->roundsA = owner->userMaxWeaponA;
    }
  } else {
    reloadtime++;
    if ((reloadtime % weapon_a_refill_rate) == 0) {
      reloadtime = 0;
      if (owner->ship->roundsA < owner->userMaxWeaponA) owner->ship->roundsA++;
    }
    if (owner->ship->roundsA > owner->userMaxWeaponA) {
      owner->ship->roundsA = owner->userMaxWeaponA;
    }
  }
}

void WeaponA::init() {
  reloadtime = 0;
}


///////////////////////////////////////////////////////////////////////////////

WeaponB::WeaponB (Player *owner) : Weapon() {
  this->owner = owner;
  inventory = 0;
  weapon_b_refill_rate = retrorocket->gameFile->getInt ("weapon_b_refill_rate");
  oneshot_damage = retrorocket->gameFile->getInt ("oneshot_damage");
  init();
}

void WeaponB::reload() {
  if (retrorocket->rtsMode && (owner->ship->roundsB >= inventory) &&
      (owner->isLocalPlayer())) {
    owner->ship->roundsB = inventory;
    if (owner->ship->roundsB > owner->userMaxWeaponB) {
      owner->ship->roundsB = owner->userMaxWeaponB;
    }
  } else {
    reloadtime++;
    if ((reloadtime % weapon_b_refill_rate) == 0) {
      reloadtime = 0;
      if (owner->ship->roundsB < owner->maxWeaponB) owner->ship->roundsB++;
    }
    if (owner->ship->roundsB > owner->userMaxWeaponB) {
      owner->ship->roundsB = owner->userMaxWeaponB;
    }
  }
}

void WeaponB::init() {
  reloadtime = 0;
}

///////////////////////////////////////////////////////////////////////////////

void SingleStream::shoot() {
  if (owner->ship->roundsA) {
    WeaponA::shoot();

    Fix32 bulletSpeed = owner->bulletSpeed;
    Fix32 vx = owner->ship->vx +
      vectorXComp (owner->ship->getOrientation(), bulletSpeed);
    Fix32 vy = owner->ship->vy +
      vectorYComp (owner->ship->getOrientation(), bulletSpeed);

    owner->ship->newParticle (owner->ship->x + owner->ship->gunPointX,
                       owner->ship->y + owner->ship->gunPointY,
                              vx, vy,
                              ((Fix32)particle_damage *
                               owner->damageScaler).toInt());
  }
}

///////////////////////////////////////////////////////////////////////////////

void SprayStream::shoot() {
  if (owner->ship->roundsA) {
    WeaponA::shoot();

    weaponstate = weaponstate + 191;

    Fix32 spread = ((Fix32)((int)weaponstate % spraystream_spread))
      - spraystream_spread/2;

    Fix32 bulletSpeed = owner->bulletSpeed;
    Fix32 vx = owner->ship->vx +
      vectorXComp (owner->ship->getOrientation() + spread, bulletSpeed);
    Fix32 vy = owner->ship->vy +
      vectorYComp (owner->ship->getOrientation() + spread, bulletSpeed);

    owner->ship->newParticle (owner->ship->x + owner->ship->gunPointX,
                              owner->ship->y + owner->ship->gunPointY,
                              vx, vy,
                              ((Fix32)particle_damage *
                               owner->damageScaler).toInt());
  }
}

///////////////////////////////////////////////////////////////////////////////

void DoubleStream::init() {
}

void DoubleStream::shoot() {
  if (owner->ship->roundsA) {
    WeaponA::shoot();

    Fix32 bulletSpeed = owner->bulletSpeed;
    Fix32 vx = owner->ship->vx +
      vectorXComp (owner->ship->getOrientation(), bulletSpeed);
    Fix32 vy = owner->ship->vy +
      vectorYComp (owner->ship->getOrientation(), bulletSpeed);

    Fix32 gunPointX, gunPointY;
    switch (weaponstate) {
      case 0: {
        gunPointX = owner->ship->gunPointLX;
        gunPointY = owner->ship->gunPointLY;
        weaponstate = 1;
        break;
      }
      default:
      case 1: {
        gunPointX = owner->ship->gunPointRX;
        gunPointY = owner->ship->gunPointRY;
        weaponstate = 0;
        break;
      }
    }
    owner->ship->newParticle (owner->ship->x + gunPointX,
                       owner->ship->y + gunPointY,
                              vx, vy, ((Fix32)particle_damage *
                                       owner->damageScaler).toInt());
  }
}

///////////////////////////////////////////////////////////////////////////////

void TriStream::init() {
}

void TriStream::shoot() {
  if ((owner->ship->roundsA >= 3) && !(weaponstate++ % 3)) {
    WeaponA::shoot();

    Fix32 bulletSpeed = owner->bulletSpeed;
    Fix32 vx = owner->ship->vx +
      vectorXComp (owner->ship->getOrientation(), bulletSpeed);
    Fix32 vy = owner->ship->vy +
      vectorYComp (owner->ship->getOrientation(), bulletSpeed);

    owner->ship->newParticle (owner->ship->x + owner->ship->gunPointX,
                       owner->ship->y + owner->ship->gunPointY,
                              vx, vy, ((Fix32)particle_damage *
                                       owner->damageScaler).toInt());
    owner->ship->newParticle (owner->ship->x + owner->ship->gunPointLX,
                       owner->ship->y + owner->ship->gunPointLY,
                              vx, vy, ((Fix32)particle_damage *
                                       owner->damageScaler).toInt());
    owner->ship->newParticle (owner->ship->x + owner->ship->gunPointRX,
                       owner->ship->y + owner->ship->gunPointRY,
                              vx, vy, ((Fix32)particle_damage *
                                       owner->damageScaler).toInt());
    owner->ship->roundsA -= 2;
  }
}

///////////////////////////////////////////////////////////////////////////////

void ThrustSingleShot:: shoot() {
  if (owner->ship->roundsB) {
    WeaponB::shoot();

    Fix32 bulletSpeed = owner->bulletSpeed;
    Fix32 vx = owner->ship->vx +
      vectorXComp (owner->ship->getOrientation(), bulletSpeed);
    Fix32 vy = owner->ship->vy +
      vectorYComp (owner->ship->getOrientation(), bulletSpeed);
    
    owner->ship->newParticle (owner->ship->x + owner->ship->gunPointX,
                       owner->ship->y + owner->ship->gunPointY,
                              vx, vy, ((Fix32)oneshot_damage *
                                       owner->damageScaler).toInt());
  }
}

///////////////////////////////////////////////////////////////////////////////

void SingleShot:: shoot() {
  if (owner->ship->roundsB) {
    WeaponB::shoot();

    Fix32 bulletSpeed = owner->bulletSpeed;
    Fix32 vx = owner->ship->vx +
      vectorXComp (owner->ship->getOrientation(), bulletSpeed);
    Fix32 vy = owner->ship->vy +
      vectorYComp (owner->ship->getOrientation(), bulletSpeed);
    
    Particle  *p = owner->ship->
      newParticle (owner->ship->x + owner->ship->gunPointX,
                   owner->ship->y + owner->ship->gunPointY,
                   vx, vy, ((Fix32)oneshot_damage *
                            owner->damageScaler).toInt());
    p->particleType = Particle::STAR;
    p->armTime = singleshot_armtime;
  }
}

///////////////////////////////////////////////////////////////////////////////

void MineShot:: shoot() {
  if (owner->ship->roundsB) {
    WeaponB::shoot();

    Fix32 bulletSpeed = retrorocket->gameFile->getFloat ("mine_shot_speed");
    Fix32 vx = owner->ship->vx -
      vectorXComp (owner->ship->getOrientation(), bulletSpeed);
    Fix32 vy = owner->ship->vy -
      vectorYComp (owner->ship->getOrientation(), bulletSpeed);

    Particle  *p = owner->ship->newParticle (owner->ship->x +
                                             owner->ship->gunPointBX,
                                             owner->ship->y +
                                             owner->ship->gunPointBY,
                                             vx, vy,
                                             ((Fix32)oneshot_damage
                                              * owner->damageScaler).toInt());
    p->particleType = Particle::MINE;
    p->drag = retrorocket->gameFile->getInt ("mine_drag");
    p->recalculateCoefficients();

    
  }
}

///////////////////////////////////////////////////////////////////////////////

void BuckShot:: shoot() {
  if (owner->ship->roundsB) {
    WeaponB::shoot();

    Fix32 bulletSpeed = owner->bulletSpeed;
    Fix32 vx = owner->ship->vx +
      vectorXComp (owner->ship->getOrientation(), bulletSpeed);
    Fix32 vy = owner->ship->vy +
      vectorYComp (owner->ship->getOrientation(), bulletSpeed);

    Particle *p = owner->ship->newParticle (owner->ship->x +
                                            owner->ship->gunPointX,
                                            owner->ship->y +
                                            owner->ship->gunPointY,
                                            vx, vy,
                                            ((Fix32)particle_damage *
                                             owner->damageScaler).toInt());

    p->particleType = Particle::BUCKSHOT;
    p->armTime = buckshot_armtime;
  }
}

///////////////////////////////////////////////////////////////////////////////

void HeatSeeker:: shoot() {
  if (owner->ship->roundsB) {
    WeaponB::shoot();

    Fix32 bulletSpeed = owner->bulletSpeed;
    Fix32 vx = owner->ship->vx +
      vectorXComp (owner->ship->getOrientation(), bulletSpeed);
    Fix32 vy = owner->ship->vy +
      vectorYComp (owner->ship->getOrientation(), bulletSpeed);

    Particle *p = owner->ship->newParticle (owner->ship->x +
                                            owner->ship->gunPointX,
                                            owner->ship->y +
                                            owner->ship->gunPointY,
                                            vx, vy,
                                            ((Fix32)oneshot_damage
                                             * owner->damageScaler).toInt());
    p->particleType = Particle::HEATSEEKER;
    p->armTime = heatseeker_armtime;
  }
}

///////////////////////////////////////////////////////////////////////////////

void ASM:: shoot() {
  if (owner->ship->roundsB) {
    WeaponB::shoot();

    Fix32 bulletSpeed = owner->bulletSpeed;
    Fix32 vx = owner->ship->vx +
      vectorXComp (owner->ship->getOrientation(), bulletSpeed);
    Fix32 vy = owner->ship->vy +
      vectorYComp (owner->ship->getOrientation(), bulletSpeed);

    Particle *p = owner->ship->newParticle (owner->ship->x +
                                            owner->ship->gunPointX,
                                            owner->ship->y +
                                            owner->ship->gunPointY,
                                            vx, vy,
                                            ((Fix32)asm_damage
                                             * owner->damageScaler).toInt());
    p->particleType = Particle::ASM;
    p->armTime = asm_armtime;
    p->recalculateCoefficients();
  }
}



///////////////////////////////////////////////////////////////////////////////

void SentrySeed:: shoot() {
  if (owner->ship->roundsB) {
    WeaponB::shoot();

    Fix32 bulletSpeed = owner->bulletSpeed;
    Fix32 vx = owner->ship->vx + vectorXComp (owner->ship->getOrientation(),
                                              bulletSpeed);
    Fix32 vy = owner->ship->vy + vectorYComp (owner->ship->getOrientation(),
                                              bulletSpeed);

    Particle *p = owner->ship->newParticle (owner->ship->x +
                                            owner->ship->gunPointX,
				     owner->ship->y + owner->ship->gunPointY,
				     vx, vy,
                                            ((Fix32)oneshot_damage
                                             * owner->damageScaler).toInt());
    p->particleType = Particle::SENTRYSEED;
  }
}

///////////////////////////////////////////////////////////////////////////////

void HeatSentrySeed:: shoot() {
  if (owner->ship->roundsB) {
    WeaponB::shoot();

    Fix32 bulletSpeed = owner->bulletSpeed;
    Fix32 vx = owner->ship->vx +
      vectorXComp (owner->ship->getOrientation(), bulletSpeed);
    Fix32 vy = owner->ship->vy +
      vectorYComp (owner->ship->getOrientation(), bulletSpeed);

    Particle *p = owner->ship->newParticle (owner->ship->x +
                                            owner->ship->gunPointX,
                                            owner->ship->y +
                                            owner->ship->gunPointY,
                                            vx, vy,
                                            ((Fix32)oneshot_damage
                                             * owner->damageScaler).toInt());
    p->particleType = Particle::HEATSENTRYSEED;
  }
}

///////////////////////////////////////////////////////////////////////////////

void ResourceNodeSeed:: shoot() {
  if (owner->ship->roundsB) {
    WeaponB::shoot();

    Fix32 bulletSpeed = owner->bulletSpeed;
    Fix32 vx = owner->ship->vx -
      vectorXComp (owner->ship->getOrientation(), bulletSpeed);
    Fix32 vy = owner->ship->vy -
      vectorYComp (owner->ship->getOrientation(), bulletSpeed);

    Particle *p = owner->ship->newParticle (owner->ship->x +
                                            owner->ship->gunPointBX,
                                            owner->ship->y +
                                            owner->ship->gunPointBY,
                                            vx, vy,
                                            ((Fix32)oneshot_damage
                                             * owner->damageScaler).toInt());
    p->particleType = Particle::RESOURCENODESEED;
  }
}

///////////////////////////////////////////////////////////////////////////////

Bomb::Bomb (Player *owner) : WeaponB (owner) {
  bomb_damage = retrorocket->gameFile->getInt ("bomb_damage");
  if (retrorocket->rtsMode) {
    ammoPrice = retrorocket->gameFile->getInt ("bomb_ammo_price");
    price = retrorocket->gameFile->getInt ("bomb_weapon_price");
    quanta = retrorocket->gameFile->getInt ("bomb_ammo_quantum");
  } else {
    ammoPrice = 0;
    price = 0;
    quanta = 0;
  }
  type = WEAPONB_BOMB;
}

void Bomb::shoot() {
  if (owner->ship->roundsB) {
    WeaponB::shoot();

    Fix32 bulletSpeed = owner->bulletSpeed;
    Fix32 vx = owner->ship->vx -
      vectorXComp (owner->ship->getOrientation(), bulletSpeed);
    Fix32 vy = owner->ship->vy -
      vectorYComp (owner->ship->getOrientation(), bulletSpeed);

    owner->ship->newParticle (owner->ship->x + owner->ship->gunPointBX,
                              owner->ship->y + owner->ship->gunPointBY,
                              vx, vy, ((Fix32)bomb_damage
                                       * owner->damageScaler).toInt());
  }
}

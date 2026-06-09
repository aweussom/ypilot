/*
  ball.cpp
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

void Ball::load (const char *filename) {
  Config c (filename);

  drag = c.getFloat ("drag");
  mass = c.getFloat ("mass");
  maxStrength = c.getInt ("strength");
  readBinary ((u8**)&gfx, c.getString ("sprite"));
  readBinary ((u8**)&pal, c.getString ("palette"));
  ballDamage = c.getInt ("damage");
 
  createSprite ((void*)pal, (void*)gfx, spritenum);
}

Ball::~Ball() {
  free (gfx);
  free (pal);
}

Ball::Ball (int x, int y, int id, const char *filename) : Mob() {
  load (filename);
  angles = 1;
  initRotation();
  setOrientation (0);
  startX = x;
  startY = y;
  owner = -1;
  this->id = id;
  inPlay = false;
}

void Ball::init() {
  active = false;
  strength = maxStrength;
  Mob::init (startX, startY, 0, 0, drag, mass, ballDamage);
  if (!retrorocket->rtsMode) {
    ballMount = new BallMount (x, y + 16);
    ballMount->init();
  }
  if (child) child->init();
  child = NULL;
  owner = -1;
  inPlay = true;
  showMapSprite(spritenum);
}

void Ball::newBallState (int own, BallState state) {
  if (active) {
    x = state.x;
    y = state.y;
    vx = state.vx;
    vy = state.vy;
  }
}

BallState Ball::getBallState()  {
  BallState state;
  state.x = x;
  state.y = y;
  state.vx = vx;
  state.vy = vy;
  return state;
}

void Ball::release (int owner) {
  active = true;
  if (retrorocket->network->master && !retrorocket->rtsMode) {
    ballMount->deleteSelf();
  }
  ballMount = NULL;
  this->owner = owner;
}

void Ball::advance() {
  if (active) {
    // check child collisions
    if (child) {
      if (child->strength <= 0) {
        hitBy = child->hitBy;
        strength = 0;
      }
    }

    // Particles
    if (retrorocket->manager->getCollisionDamage (this)) {
      playSound (retrorocket->particleSound, retrorocket->particleSoundSize,
                 retrorocket->config->getInt("volume_particle_ship"),
                 x, y, retrorocket->config->getInt("rate_particle_ship"));
    }

    // sobs
    for (unsigned int s = 0; s < retrorocket->sobs.size(); s++) {
      if (collidesWith (retrorocket->sobs[s])) {
        if (retrorocket->rtsMode &&
            retrorocket->sobs[s]->sobType == Sob::PLATFORM) {
          retrorocket->players[owner]->credits +=250;
          retrorocket->ships[owner]->collectBalls();
        } else {
          hitBy = -1;
          strength = 0;
        }
        break;
      }
    }

    // Background
    if (y >= 0) {
      for (int i = 0; i < scp->collisionPoints; i++) {
        Fix32 x_i = scp->points[i].x, y_i = scp->points[i].y;

        if (retrorocket->field->getCPixel ( x + x_i, y + y_i )) {
          strength = 0;
          hitBy = -1;
        }
      }
    }
    
    // check ball collision
    for (unsigned int j = 0; j < retrorocket->balls.size(); j++) {
      if (retrorocket->balls[j] != this) {
        if (collidesWith (retrorocket->balls[j])) {
          strength = 0;
          hitBy = retrorocket->balls[j]->owner;
          if (hitBy == owner) hitBy = -1;
        }
      }
    }

    Mob::advance();
  } else {
    if (retrorocket->rtsMode && retrorocket->network->master && !inPlay) {
      int index = rand() % retrorocket->ballpoints.size();
      retrorocket->network->sendBallCreatePacket(index);
      startX = retrorocket->ballpoints[index].x.toInt();;
      startY = retrorocket->ballpoints[index].y.toInt();;
      init();
    }
  }
}

void Ball::collectBalls() {
  if (child) {
    ((Ball*)child)->collectBalls();
    child = NULL;
  }
  active = false;
  x = retrorocket->field->width + 1;
  y = retrorocket->field->height + 1;
  inPlay = false;
  hideMapSprite (id);
}

void Ball::sendBallPacket() {
  if (child) ((Ball*)child)->sendBallPacket();
  retrorocket->network->sendBallPacket (this);
}

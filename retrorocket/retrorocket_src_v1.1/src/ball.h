/*
  ball.h
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
// Represents a ball in thrust-type games
//
///////////////////////////////////////////////////////////////////////////////

#ifndef BALL_H
#define BALL_H

#include "retrorocket.h"

/** The state of a ball */
struct BallState {
  Fix32 x, y, vx, vy;
};

class BallMount;

/** Represents a ball in thrust-type games */
class Ball : public Mob {

  int maxStrength;
  int ballDamage;
  BallMount *ballMount;

  void load (const char *filename);

public:  
  int startX;
  int startY;
  bool active;
  int id;
  int owner;
  bool inPlay;
  
  Ball (int x, int y, int id, const char *filename);
  ~Ball();
  void init();
  void newBallState (int owner, BallState state);
  BallState getBallState();
  void release (int owner);
  Fix32 getCenterX() { return x + HALF_SPRITESIZE; }
  Fix32 getCenterY() { return y + HALF_SPRITESIZE; }
  void advance();
  void collectBalls();
  void sendBallPacket();
};

#endif

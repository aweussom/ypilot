/*
  sob.cpp
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

const u8 strengthBarTiles[] __attribute__ ((aligned (4))) = {
  0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 5, 5, 5, 5, 5, 5, 5,
  1, 5, 5, 5, 5, 5, 5, 5,
  1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,

  0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 0, 0, 0, 0,
  5, 5, 5, 1, 0, 0, 0, 0,
  5, 5, 5, 1, 0, 0, 0, 0,
  1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,

  0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 4, 4, 5, 5, 5, 5, 5,
  1, 4, 4, 5, 5, 5, 5, 5,
  1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,

  0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 0, 0, 0, 0,
  5, 5, 5, 1, 0, 0, 0, 0,
  5, 5, 5, 1, 0, 0, 0, 0,
  1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,

  0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 4, 4, 4, 4, 5, 5, 5,
  1, 4, 4, 4, 4, 5, 5, 5,
  1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,

  0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 0, 0, 0, 0,
  5, 5, 5, 1, 0, 0, 0, 0,
  5, 5, 5, 1, 0, 0, 0, 0,
  1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,

  0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 3, 3, 3, 3, 3, 3, 5,
  1, 3, 3, 3, 3, 3, 3, 5,
  1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,

  0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 0, 0, 0, 0,
  5, 5, 5, 1, 0, 0, 0, 0,
  5, 5, 5, 1, 0, 0, 0, 0,
  1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,

  0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 3, 3, 3, 3, 3, 3, 3,
  1, 3, 3, 3, 3, 3, 3, 3,
  1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,

  0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 0, 0, 0, 0,
  3, 5, 5, 1, 0, 0, 0, 0,
  3, 5, 5, 1, 0, 0, 0, 0,
  1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,

  0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 2, 2, 2, 2, 2, 2, 2,
  1, 2, 2, 2, 2, 2, 2, 2,
  1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,

  0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 0, 0, 0, 0,
  2, 2, 2, 1, 0, 0, 0, 0,
  2, 2, 2, 1, 0, 0, 0, 0,
  1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
};

const unsigned short strengthBarPal[] __attribute__ ((aligned (4))) = {
  63421, 33760, 33791, 32799, 49680 // grey(frame), green, yellow, red, grey (background)
};

const unsigned char markerTiles[] __attribute__ ((aligned (4))) = {
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 8, 8, 8, 8, 0, 0, 
  0, 8, 1, 2, 2, 1, 8, 0,
  0, 8, 2, 5, 7, 3, 8, 0, 
  0, 8, 2, 4, 5, 3, 8, 0,
  0, 8, 1, 2, 2, 1, 8, 0, 
  0, 0, 8, 8, 8, 8, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 
};

const u8 fuelTiles[] __attribute__ ((aligned (4))) = {
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 1, 2, 2, 3,
  0, 0, 4, 1, 1, 2, 2, 3,
  0, 4, 4, 1, 2, 2, 3, 3,
  4, 4, 1, 1, 2, 2, 3, 3,
  4, 4, 5, 5, 2, 6, 3, 6,

  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  3, 5, 5, 6, 0, 0, 0, 0,
  3, 5, 5, 6, 6, 7, 0, 0,
  3, 5, 5, 5, 6, 7, 7, 0,
  3, 5, 5, 5, 6, 6, 7, 7,
  3, 7, 7, 5, 8, 6, 7, 7,

  4, 4, 5, 1, 2, 6, 3, 6,
  4, 4, 5, 5, 2, 6, 3, 6,
  4, 4, 5, 1, 2, 6, 3, 6,
  4, 4, 5, 1, 2, 6, 6, 6,
  4, 4, 1, 1, 2, 2, 3, 3,
  4, 4, 1, 1, 2, 2, 3, 3,
  4, 4, 1, 1, 2, 2, 3, 3,
  4, 4, 1, 1, 2, 2, 3, 3,

  3, 7, 5, 5, 8, 6, 7, 7,
  3, 7, 7, 5, 8, 6, 7, 7,
  3, 7, 5, 5, 8, 6, 7, 7,
  3, 7, 7, 5, 8, 8, 7, 7,
  3, 5, 5, 5, 6, 6, 7, 7,
  3, 5, 5, 5, 6, 6, 7, 7,
  3, 5, 5, 5, 6, 6, 7, 7,
  3, 5, 5, 5, 6, 6, 7, 7,
};

const unsigned short fuelPal[] __attribute__ ((aligned (4))) = {
  60416, 64512, 64578, 54272, 64677, 64743, 64908, 65007
};

const u8 solidTile[] __attribute__ ((aligned (4))) = {
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
};

const unsigned short solidPal[] __attribute__ ((aligned (4))) = {
  43551
};

const u8 blankTile[] __attribute__ ((aligned (4))) = {
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
};

const unsigned short blankPal[] __attribute__ ((aligned (4))) = {
  43551 // TODO: need no pal for blank tile
};

const u8 powerplantTiles[] __attribute__ ((aligned (4))) = {
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 1, 1, 1,

  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 0, 0, 0,
  4, 4, 4, 4, 4, 1, 1, 1,

  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,

  0, 0, 0, 1, 1, 4, 4, 4,
  0, 0, 1, 4, 4, 4, 4, 4,
  0, 1, 4, 4, 4, 4, 4, 4,
  0, 1, 4, 4, 4, 4, 4, 4,
  1, 4, 4, 4, 4, 4, 4, 4,
  1, 4, 4, 4, 4, 4, 4, 4,
  1, 4, 4, 4, 4, 4, 4, 4,
  1, 4, 4, 4, 4, 4, 4, 4,

  4, 4, 4, 4, 4, 4, 4, 4,
  4, 4, 4, 4, 4, 4, 4, 4,
  4, 4, 4, 4, 4, 4, 4, 4,
  4, 4, 4, 4, 4, 4, 4, 4,
  4, 4, 4, 4, 4, 4, 4, 4,
  4, 4, 4, 4, 4, 4, 4, 4,
  4, 4, 4, 4, 4, 4, 4, 4,
  4, 4, 4, 4, 4, 4, 4, 4,

  1, 1, 0, 3, 3, 0, 0, 0,
  4, 4, 1, 0, 3, 0, 0, 0,
  4, 4, 1, 0, 3, 0, 0, 0,
  4, 4, 1, 0, 3, 0, 0, 0,
  4, 4, 1, 0, 3, 0, 0, 0,
  4, 4, 1, 0, 3, 0, 0, 0,
  4, 4, 1, 0, 3, 0, 0, 0,
  4, 4, 1, 0, 3, 0, 0, 0,

  0, 1, 4, 4, 4, 4, 4, 4,
  0, 1, 4, 4, 4, 4, 4, 4,
  0, 0, 1, 4, 4, 4, 4, 4,
  3, 3, 3, 3, 3, 3, 3, 3,
  3, 0, 0, 0, 0, 0, 0, 0,
  3, 0, 3, 0, 0, 0, 0, 0,
  3, 0, 3, 0, 0, 0, 0, 0,
  3, 0, 3, 0, 0, 0, 0, 0,

  4, 4, 4, 4, 4, 4, 4, 4,
  4, 4, 4, 4, 4, 4, 4, 4,
  4, 4, 4, 4, 4, 4, 4, 4,
  3, 3, 3, 3, 3, 3, 3, 3,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,

  4, 4, 4, 3, 3, 0, 0, 0,
  4, 4, 4, 3, 3, 0, 0, 0,
  4, 4, 4, 3, 3, 0, 0, 0,
  3, 3, 3, 3, 3, 0, 0, 0,
  0, 0, 0, 0, 0, 3, 0, 0,
  0, 0, 0, 0, 0, 3, 0, 0,
  0, 0, 0, 0, 0, 3, 0, 0,
  0, 0, 0, 0, 0, 3, 0, 0,

  // smoke (anim)

  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 2, 2, 0, 0,
  0, 0, 0, 0, 2, 2, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,

  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 2, 2, 0, 0,
  0, 0, 0, 0, 2, 2, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,

  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 2, 2, 0, 0,
  0, 0, 0, 0, 2, 2, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,

  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 2, 2, 0, 0,
  0, 0, 0, 0, 2, 2, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,

  0, 0, 0, 0, 2, 2, 0, 0,
  0, 0, 0, 0, 2, 2, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
};

const unsigned short powerplantPal[] __attribute__ ((aligned (4))) = {
  65205, 65535, 43007, 32768
};

const u8 resourceTiles[] __attribute__ ((aligned (4))) = {
  0, 0, 0, 0, 0, 0, 1, 2,
  0, 0, 0, 0, 0, 0, 1, 2,
  0, 0, 0, 0, 0, 0, 1, 2,
  0, 0, 0, 0, 0, 0, 1, 2, 
  0, 0, 0, 0, 0, 0, 1, 2,
  0, 0, 0, 0, 0, 0, 1, 2, 
  0, 0, 0, 0, 0, 0, 1, 2,
  0, 0, 0, 0, 0, 0, 1, 2, 

  3, 1, 0, 0, 0, 0, 0, 0,
  3, 1, 0, 0, 0, 0, 0, 0, 
  3, 1, 0, 0, 0, 0, 0, 0,
  3, 1, 0, 0, 0, 0, 0, 0, 
  3, 1, 0, 0, 0, 0, 0, 0,
  3, 1, 0, 0, 0, 0, 0, 0, 
  3, 1, 0, 0, 0, 0, 0, 0,
  3, 1, 0, 0, 0, 0, 0, 0, 

  0, 0, 0, 0, 0, 0, 1, 2,
  0, 0, 0, 0, 0, 0, 1, 2,
  0, 0, 0, 0, 0, 0, 1, 2,
  0, 0, 0, 0, 0, 0, 1, 2, 
  0, 0, 0, 0, 0, 0, 1, 2,
  0, 0, 0, 0, 0, 0, 1, 2, 
  0, 0, 0, 0, 0, 0, 1, 2,
  0, 0, 0, 0, 0, 0, 1, 2, 

  3, 1, 0, 0, 0, 0, 0, 0,
  3, 1, 0, 0, 0, 0, 0, 0, 
  3, 1, 0, 0, 0, 0, 0, 0,
  3, 1, 0, 0, 0, 0, 0, 0, 
  3, 1, 0, 0, 0, 0, 0, 0,
  3, 1, 0, 0, 0, 0, 0, 0, 
  3, 1, 0, 0, 0, 0, 0, 0,
  3, 1, 0, 0, 0, 0, 0, 0, 

  0, 0, 1, 1, 1, 1, 1, 2,
  0, 1, 1, 4, 4, 4, 4, 2, 
  0, 1, 4, 4, 2, 2, 2, 3,
  0, 1, 4, 4, 2, 3, 3, 3, 
  0, 1, 4, 4, 2, 3, 3, 5,
  0, 1, 1, 4, 4, 4, 3, 3, 
  0, 0, 1, 1, 1, 4, 4, 4,
  0, 0, 0, 0, 1, 1, 1, 1, 

  3, 1, 1, 1, 1, 1, 0, 0,
  2, 3, 3, 4, 4, 1, 1, 0, 
  3, 3, 3, 2, 2, 4, 1, 0,
  3, 3, 3, 3, 3, 4, 1, 0, 
  5, 5, 5, 3, 4, 4, 1, 0,
  3, 3, 3, 4, 4, 1, 1, 0, 
  4, 4, 4, 1, 1, 1, 0, 0,
  1, 1, 1, 1, 0, 0, 0, 0,
};

const unsigned short resourcePal[] __attribute__ ((aligned (4))) = {
  38254, 33534, 33663, 33468, 46079
};

const u8 resourceNodeTiles[] __attribute__ ((aligned (4))) = {
  0, 0, 0, 0, 0, 0, 1, 1,
  0, 0, 0, 0, 0, 0, 1, 2, 
  0, 0, 0, 0, 0, 0, 1, 2,
  0, 0, 0, 0, 0, 0, 1, 1, 
  0, 0, 0, 0, 0, 0, 0, 1,
  0, 0, 0, 0, 0, 2, 0, 1, 
  0, 0, 2, 2, 2, 2, 2, 1,
  0, 2, 2, 0, 0, 2, 0, 1,

  1, 1, 0, 0, 0, 0, 0, 0,
  2, 3, 0, 0, 0, 0, 0, 0, 
  2, 3, 0, 0, 0, 0, 0, 0,
  2, 3, 0, 0, 0, 0, 0, 0, 
  3, 0, 0, 0, 0, 0, 0, 0,
  3, 0, 2, 0, 0, 0, 0, 0, 
  3, 2, 2, 2, 2, 2, 0, 0,
  3, 0, 2, 0, 0, 2, 2, 0, 

  2, 4, 4, 2, 0, 2, 2, 1,
  2, 4, 5, 2, 0, 2, 0, 1, 
  2, 4, 5, 2, 0, 2, 2, 1,
  2, 5, 5, 2, 0, 2, 0, 1,
  2, 2, 2, 2, 0, 2, 2, 1,
  0, 0, 0, 0, 2, 2, 2, 1, 
  0, 0, 0, 0, 2, 2, 0, 1,
  0, 0, 0, 0, 2, 2, 0, 1,
  
  3, 2, 2, 0, 2, 4, 4, 2,
  3, 0, 2, 0, 2, 4, 5, 2, 
  3, 2, 2, 0, 2, 4, 5, 2,
  3, 0, 2, 0, 2, 5, 5, 2,
  3, 2, 2, 0, 2, 2, 2, 2,
  3, 2, 2, 2, 0, 0, 0, 0, 
  3, 0, 2, 2, 0, 0, 0, 0,
  3, 0, 2, 2, 0, 0, 0, 0,

  // Second frame of ResourceNodeTiles
  
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 1, 1,
  0, 0, 0, 0, 0, 2, 1, 2, 
  0, 0, 2, 2, 2, 2, 2, 1,
  0, 2, 2, 0, 0, 2, 0, 1,

  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 
  1, 1, 0, 0, 0, 0, 0, 0,
  3, 1, 2, 0, 0, 0, 0, 0, 
  3, 2, 2, 2, 2, 2, 0, 0,
  3, 0, 2, 0, 0, 2, 2, 0, 

  2, 4, 4, 2, 0, 2, 2, 1,
  2, 4, 5, 2, 0, 2, 0, 1, 
  2, 4, 5, 2, 0, 2, 2, 1,
  2, 5, 5, 2, 0, 2, 0, 1,
  2, 2, 2, 2, 0, 2, 2, 1,
  0, 0, 0, 0, 2, 2, 2, 1, 
  0, 0, 0, 0, 2, 2, 0, 1,
  0, 0, 0, 0, 2, 2, 0, 1,
  
  3, 2, 2, 0, 2, 4, 4, 2,
  3, 0, 2, 0, 2, 4, 5, 2, 
  3, 2, 2, 0, 2, 4, 5, 2,
  3, 0, 2, 0, 2, 5, 5, 2,
  3, 2, 2, 0, 2, 2, 2, 2,
  3, 2, 2, 2, 0, 0, 0, 0, 
  3, 0, 2, 2, 0, 0, 0, 0,
  3, 0, 2, 2, 0, 0, 0, 0,
};

const unsigned short resourceNodePal[] __attribute__ ((aligned (4))) = {
  58136, 53942, 49680, 33534, 33663
};

const u8 buttonrightTiles[] __attribute__ ((aligned (4))) = {
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 0, 0, 0, 0, 0, 0,
  0, 1, 2, 2, 2, 0, 0, 0,
  0, 1, 3, 3, 3, 2, 0, 0,
  0, 1, 3, 3, 3, 3, 2, 0,
  0, 1, 3, 3, 3, 3, 2, 0,
  0, 1, 3, 3, 3, 3, 2, 0,

  0, 1, 3, 3, 3, 3, 2, 0,
  0, 1, 3, 3, 3, 3, 2, 0,
  0, 1, 3, 3, 3, 3, 2, 0,
  0, 1, 3, 3, 3, 2, 0, 0,
  0, 1, 2, 2, 2, 0, 0, 0,
  1, 1, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
};

const unsigned short buttonrightPal[] __attribute__ ((aligned (4))) = {
  65205, 43007, 32768
};

const u8 buttonleftTiles[] __attribute__ ((aligned (4))) = {
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 1,
  0, 0, 0, 2, 2, 2, 1, 0,
  0, 0, 2, 3, 3, 3, 1, 0,
  0, 2, 3, 3, 3, 3, 1, 0,
  0, 2, 3, 3, 3, 3, 1, 0,
  0, 2, 3, 3, 3, 3, 1, 0,

  0, 2, 3, 3, 3, 3, 1, 0,
  0, 2, 3, 3, 3, 3, 1, 0,
  0, 2, 3, 3, 3, 3, 1, 0,
  0, 0, 2, 3, 3, 3, 1, 0,
  0, 0, 0, 2, 2, 2, 1, 0,
  0, 0, 0, 0, 0, 0, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
};

const unsigned short buttonleftPal[] __attribute__ ((aligned (4))) = {
  65205, 43007, 32768
};

const u8 gunneTiles[] __attribute__ ((aligned (4))) = {
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 1,
  0, 0, 0, 0, 1, 1, 2, 2,
  0, 0, 1, 1, 2, 2, 2, 2,
  0, 1, 2, 1, 1, 1, 1, 1,
  0, 1, 1, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,

  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 1, 1, 1, 1, 0, 0,
  1, 1, 2, 2, 2, 2, 1, 1,
  2, 2, 1, 1, 2, 2, 2, 2,
  1, 1, 1, 1, 1, 1, 2, 2,
  2, 2, 2, 2, 1, 1, 1, 1,
  2, 2, 2, 2, 2, 2, 2, 1,
  2, 2, 2, 2, 2, 2, 2, 2,

  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  1, 0, 0, 0, 0, 0, 0, 0,
  2, 1, 0, 0, 0, 0, 0, 0,
  2, 1, 0, 0, 0, 0, 0, 0,
  1, 1, 0, 0, 0, 0, 0, 0,
  1, 2, 1, 1, 0, 0, 0, 0,

  0, 0, 0, 0, 2, 2, 2, 2,
  0, 0, 0, 0, 2, 2, 2, 2,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,

  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,
  0, 0, 0, 0, 2, 2, 2, 2,
  0, 0, 0, 0, 2, 2, 2, 2,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,

  2, 1, 2, 2, 1, 0, 0, 0,
  2, 1, 2, 2, 1, 0, 0, 0,
  2, 2, 1, 2, 1, 0, 0, 0,
  2, 2, 1, 2, 1, 0, 0, 0,
  2, 2, 1, 2, 1, 0, 0, 0,
  2, 2, 1, 2, 1, 0, 0, 0,
  2, 2, 2, 1, 1, 0, 0, 0,
  2, 2, 2, 2, 2, 0, 0, 0,
};

const unsigned short gunnePal[] __attribute__ ((aligned (4))) = {
  54623, 32768
};

const u8 gunnwTiles[] __attribute__ ((aligned (4))) = {
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 1,
  0, 0, 0, 0, 0, 0, 1, 2,
  0, 0, 0, 0, 0, 0, 1, 2,
  0, 0, 0, 0, 0, 0, 1, 1,
  0, 0, 0, 0, 1, 1, 2, 1,

  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 1, 1, 1, 1, 0, 0,
  1, 1, 2, 2, 2, 2, 1, 1,
  2, 2, 2, 2, 1, 1, 2, 2,
  2, 2, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 2, 2, 2, 2,
  1, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,

  0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 0, 0, 0, 0, 0, 0,
  2, 2, 1, 1, 0, 0, 0, 0,
  2, 2, 2, 2, 1, 1, 0, 0,
  1, 1, 1, 1, 1, 2, 1, 0,
  2, 2, 2, 2, 2, 1, 1, 0,
  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,

  0, 0, 0, 1, 2, 2, 1, 2,
  0, 0, 0, 1, 2, 2, 1, 2,
  0, 0, 0, 1, 2, 1, 2, 2,
  0, 0, 0, 1, 2, 1, 2, 2,
  0, 0, 0, 1, 2, 1, 2, 2,
  0, 0, 0, 1, 2, 1, 2, 2,
  0, 0, 0, 1, 1, 2, 2, 2,
  0, 0, 0, 2, 2, 2, 2, 2,

  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 0, 0, 0, 0,
  2, 2, 2, 2, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,

  2, 2, 2, 2, 0, 0, 0, 0,
  2, 2, 2, 2, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
};

const unsigned short gunnwPal[] __attribute__ ((aligned (4))) = {
  54623, 32768
};

const u8 gunseTiles[] __attribute__ ((aligned (4))) = {
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 2, 2, 2, 2,
  0, 0, 0, 0, 2, 2, 2, 2,

  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 2, 2, 2, 2,
  0, 0, 0, 0, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,

  2, 2, 2, 1, 1, 0, 0, 0,
  2, 2, 1, 2, 1, 0, 0, 0,
  2, 2, 1, 2, 1, 0, 0, 0,
  2, 2, 1, 2, 1, 0, 0, 0,
  2, 2, 1, 2, 1, 0, 0, 0,
  2, 1, 2, 2, 1, 0, 0, 0,
  2, 1, 2, 2, 1, 0, 0, 0,
  1, 2, 1, 1, 0, 0, 0, 0,

  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,
  0, 1, 1, 2, 2, 2, 2, 2,
  0, 1, 2, 1, 1, 1, 1, 1,
  0, 0, 1, 1, 2, 2, 2, 2,
  0, 0, 0, 0, 1, 1, 2, 2,
  0, 0, 0, 0, 0, 0, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0,

  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 1,
  2, 2, 2, 2, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 2, 2,
  2, 2, 1, 1, 2, 2, 2, 2,
  1, 1, 2, 2, 2, 2, 1, 1,
  0, 0, 1, 1, 1, 1, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,

  1, 2, 1, 1, 0, 0, 0, 0,
  1, 1, 0, 0, 0, 0, 0, 0,
  2, 1, 0, 0, 0, 0, 0, 0,
  2, 1, 0, 0, 0, 0, 0, 0,
  1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
};

const unsigned short gunsePal[] __attribute__ ((aligned (4))) = {
  54623, 32768
};

const u8 gunswTiles[] __attribute__ ((aligned (4))) = {
  0, 0, 0, 1, 1, 2, 2, 2,
  0, 0, 0, 1, 2, 1, 2, 2,
  0, 0, 0, 1, 2, 1, 2, 2,
  0, 0, 0, 1, 2, 1, 2, 2,
  0, 0, 0, 1, 2, 1, 2, 2,
  0, 0, 0, 1, 2, 2, 1, 2,
  0, 0, 0, 1, 2, 2, 1, 2,
  0, 0, 0, 0, 1, 1, 2, 1,

  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  2, 2, 2, 2, 0, 0, 0, 0, 
  2, 2, 2, 2, 0, 0, 0, 0, 
  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,

  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  2, 2, 2, 2, 0, 0, 0, 0,
  2, 2, 2, 2, 0, 0, 0, 0,

  0, 0, 0, 0, 0, 0, 1, 1,
  0, 0, 0, 0, 0, 0, 1, 2,
  0, 0, 0, 0, 0, 0, 1, 2,
  0, 0, 0, 0, 0, 0, 0, 1,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,

  1, 2, 2, 2, 2, 2, 2, 2,
  1, 1, 1, 1, 2, 2, 2, 2,
  2, 2, 1, 1, 1, 1, 1, 1,
  2, 2, 2, 2, 1, 1, 2, 2,
  1, 1, 2, 2, 2, 2, 1, 1,
  0, 0, 1, 1, 1, 1, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,

  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 1, 1, 0,
  1, 1, 1, 1, 1, 2, 1, 0,
  2, 2, 2, 2, 1, 1, 0, 0,
  2, 2, 1, 1, 0, 0, 0, 0,
  1, 1, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
};

const unsigned short gunswPal[] __attribute__ ((aligned (4))) = {
  54623, 32768
};

const u8 sentryGunTiles[] __attribute__ ((aligned (4))) = {
  //sentry off
  0, 0, 0, 0, 0, 1, 2, 3,
  0, 0, 0, 4, 5, 5, 6, 1, 
  0, 0, 8, 7, 9, 10, 10, 10,
  0, 8, 7, 13, 10, 14, 14, 14, 
  0, 3, 15, 10, 13, 9, 15, 16,
  7, 18, 14, 13, 4, 17, 19, 2, 
  3, 5, 10, 20, 12, 21, 22, 23,
  19, 1, 10, 24, 5, 25, 22, 21, 

  3, 2, 1, 0, 0, 0, 0, 0,
  1, 7, 2, 2, 4, 0, 0, 0, 
  10, 10, 10, 11, 12, 1, 0, 0,
  10, 10, 10, 10, 13, 6, 8, 0, 
  13, 10, 10, 10, 10, 16, 17, 0,
  15, 14, 10, 10, 10, 10, 3, 7, 
  7, 13, 10, 10, 10, 10, 12, 3,
  12, 20, 10, 10, 10, 10, 8, 19, 

  3, 1, 10, 13, 8, 18, 26, 3,
  2, 5, 10, 10, 20, 4, 1, 15, 
  4, 18, 10, 10, 10, 14, 13, 14,
  0, 3, 15, 10, 10, 10, 10, 10, 
  0, 9, 7, 13, 10, 10, 10, 10,
  0, 0, 15, 7, 9, 10, 10, 10, 
  0, 0, 0, 9, 5, 5, 6, 1,
  0, 0, 0, 0, 0, 9, 7, 2, 

  4, 14, 10, 10, 10, 10, 8, 18,
  20, 10, 10, 10, 10, 10, 12, 2, 
  10, 10, 10, 10, 10, 10, 3, 4,
  10, 10, 10, 10, 10, 16, 17, 0, 
  10, 10, 10, 10, 13, 6, 9, 0,
  10, 10, 10, 11, 12, 4, 0, 0, 
  1, 7, 2, 2, 9, 0, 0, 0,
  17, 7, 9, 0, 0, 0, 0, 0, 

  // sentry on
  0, 0, 0, 0, 0, 1, 2, 3,
  0, 0, 0, 4, 5, 5, 6, 1, 
  0, 0, 8, 7, 27, 10, 10, 10,
  0, 8, 7, 13, 10, 14, 28, 29, 
  0, 3, 9, 10, 33, 34, 35, 36,
  7, 18, 10, 13, 40, 41, 42, 43, 
  3, 5, 10, 48, 49, 50, 51, 52,
  19, 1, 10, 58, 59, 60, 51, 61, 

  3, 2, 1, 0, 0, 0, 0, 0,
  1, 7, 2, 2, 4, 0, 0, 0, 
  10, 10, 10, 9, 12, 1, 0, 0,
  30, 31, 32, 10, 13, 6, 8, 0, 
  37, 38, 39, 31, 10, 16, 17, 0,
  44, 45, 46, 47, 31, 10, 3, 7, 
  53, 54, 55, 56, 39, 57, 12, 3,
  62, 63, 64, 65, 66, 31, 8, 19, 

  3, 1, 10, 67, 68, 69, 70, 71,
  2, 5, 10, 30, 76, 77, 78, 79, 
  4, 18, 10, 32, 47, 83, 84, 85,
  0, 3, 9, 10, 31, 66, 56, 88, 
  0, 9, 7, 13, 10, 31, 39, 66,
  0, 0, 15, 7, 27, 10, 57, 31, 
  0, 0, 0, 9, 5, 5, 6, 1,
  0, 0, 0, 0, 0, 9, 7, 2, 

  72, 73, 74, 75, 38, 31, 8, 18,
  80, 81, 82, 65, 66, 32, 12, 2, 
  86, 87, 55, 46, 39, 57, 3, 4,
  88, 65, 56, 66, 32, 16, 17, 0, 
  38, 66, 39, 32, 13, 6, 9, 0,
  31, 32, 57, 9, 12, 4, 0, 0, 
  1, 7, 2, 2, 9, 0, 0, 0,
  17, 7, 9, 0, 0, 0, 0, 0
};

const unsigned short sentryGunPal[] __attribute__ ((aligned (4))) = {
  43338, 48623, 51794, 41224, 47566, 44395, 45452, 42281, 39110,
  32768, 39111, 46509, 34882, 33825, 40167, 38053, 49680, 52851, 53908,
  35939, 60250, 64478, 59193, 36996, 61307, 57079, 38054, 33827, 33828,
  32772, 32771, 32770, 34883, 39113, 41196, 38060, 34891, 32777, 32774,
  41226, 49684, 53912, 48631, 40179, 33839, 32779, 32775, 35941, 46513,
  60252, 64479, 59197, 45466, 34934, 32785, 32780, 32769, 36999, 47571,
  61309, 60287, 46719, 36189, 32854, 32782, 32776, 34886, 42288, 52857,
  57085, 51967, 41631, 34335, 32986, 32784, 35947, 41235, 43352, 40349,
  36383, 33182, 32887, 33837, 34898, 33878, 32920, 32886, 32783
};

const u8 ballMountTiles[] __attribute__ ((aligned (4))) = {
  0, 0, 0, 0, 1, 0, 0, 0,
  0, 0, 0, 1, 0, 1, 1, 1,
  0, 0, 0, 1, 0, 0, 0, 0,
  0, 0, 0, 0, 1, 1, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 0,
  0, 0, 0, 0, 0, 0, 1, 0,
  0, 0, 0, 0, 0, 0, 1, 0,
  0, 0, 0, 0, 0, 1, 0, 0,

  0, 0, 1, 0, 0, 0, 0, 0,
  1, 1, 0, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 0, 0, 0, 0,
  0, 1, 1, 0, 0, 0, 0, 0,
  1, 0, 0, 0, 0, 0, 0, 0,
  1, 0, 0, 0, 0, 0, 0, 0,
  1, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 0, 0, 0, 0, 0, 0,
};

const unsigned short ballMountPal[] __attribute__ ((aligned (4))) = {
  41645, 
};

const u8 platformTiles[] __attribute__ ((aligned (4))) = {
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 2, 3, 2, 0, 0, 0, 2, 
  1, 0, 2, 3, 2, 0, 2, 4,
  0, 0, 0, 2, 3, 2, 4, 2, 
  0, 0, 0, 0, 2, 4, 2, 0,
  0, 0, 0, 2, 4, 2, 3, 2, 
  0, 0, 2, 4, 2, 0, 2, 3,
  0, 2, 4, 2, 0, 0, 0, 2, 

  1, 1, 1, 1, 1, 1, 1, 1,
  4, 2, 2, 3, 2, 0, 0, 0, 
  2, 0, 0, 2, 3, 2, 0, 2,
  0, 0, 0, 0, 2, 3, 2, 4, 
  0, 0, 0, 0, 0, 2, 4, 2,
  0, 0, 0, 0, 2, 4, 2, 3, 
  2, 0, 0, 2, 4, 2, 0, 2,
  3, 2, 2, 4, 2, 0, 0, 0, 

  1, 1, 1, 1, 1, 1, 1, 1,
  2, 4, 2, 2, 3, 2, 0, 0, 
  4, 2, 0, 0, 2, 3, 2, 0,
  2, 0, 0, 0, 0, 2, 3, 2, 
  0, 0, 0, 0, 0, 0, 2, 4,
  2, 0, 0, 0, 0, 2, 4, 2, 
  3, 2, 0, 0, 2, 4, 2, 0,
  2, 3, 2, 2, 4, 2, 0, 0, 

  1, 1, 1, 1, 1, 1, 1, 1,
  0, 2, 4, 2, 2, 3, 2, 0, 
  2, 4, 2, 0, 0, 2, 3, 2,
  4, 2, 0, 0, 0, 0, 2, 3, 
  2, 0, 0, 0, 0, 0, 0, 2,
  3, 2, 0, 0, 0, 0, 2, 4, 
  2, 3, 2, 0, 0, 2, 4, 2,
  0, 2, 3, 2, 2, 4, 2, 0, 

  1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 2, 4, 2, 2, 3, 2, 
  0, 2, 4, 2, 0, 0, 2, 3,
  2, 4, 2, 0, 0, 0, 0, 2, 
  4, 2, 0, 0, 0, 0, 0, 0,
  2, 3, 2, 0, 0, 0, 0, 2, 
  0, 2, 3, 2, 0, 0, 2, 4,
  0, 0, 2, 3, 2, 2, 4, 2, 

  1, 1, 1, 1, 1, 1, 1, 0,
  0, 0, 0, 2, 4, 2, 1, 0, 
  2, 0, 2, 4, 2, 0, 1, 0,
  3, 2, 4, 0, 0, 0, 0, 0, 
  2, 4, 2, 0, 0, 0, 0, 0,
  4, 2, 3, 2, 0, 0, 0, 0, 
  2, 0, 2, 3, 2, 0, 0, 0,
  0, 0, 0, 2, 3, 2, 0, 0, 
};

const unsigned short platformPal[] __attribute__ ((aligned (4))) = {
  43336, 46509, 57936, 57079
};

///////////////////////////////////////////////////////////////////////////////

void findMask (int *wmask, int *hmask, Fix32 width, Fix32 height) {
  *wmask = (int)pow (2, ceil (log2 (width.toFloat()))) - 1;
  *hmask = (int)pow (2, ceil (log2 (height.toFloat()))) - 1;
}

int Sob::blankStartTile;

Sob::Sob (Fix32 x, Fix32 y) {
  // can only place sob on 8-pixel boundaries
  this->x = x & ~7; 
  this->y = y & ~7;

  score = 0;
  visible = false;
  strength = 0;
  sobType = OTHER;
  retrorocket->sobs.push_back (this);
  killer = -1;

#ifndef NDS
  // DEBUG
  printf ("---add---\n");
  for (unsigned int i = 0; i < retrorocket->sobs.size(); i++) {
    printf ("DB: %d %d\n", i, retrorocket->sobs[i]->sobType);
  }
#endif
}

void Sob::showTile (int index, Fix32 x, Fix32 y, int tile) {
  // set tile
  setBgTile (SOB_BG, x, y, tile);
}

void Sob::hideTile (int index, Fix32 x, Fix32 y) {
  // write blank tile
  setBgTile (SOB_BG, x, y, blankStartTile);
}

void Sob::copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                     int *fieldTilessize, int *fieldPaletteSize,
                     const u8 *tiles, int sizeOfTiles,
                     const u16 *pal, int sizeOfPal,
                     int *startTile, bool copyPal) {
  // TODO: check for tiles/palette overflow

  // realloc fieldTiles
  if (*fieldTilessize) {
    *fieldTiles = (u8*)realloc (*fieldTiles, *fieldTilessize + sizeOfTiles);
  } else {
    *fieldTiles = (u8*)malloc (sizeOfTiles);
  }
  if (!*fieldTiles) panic ("Can't realloc");

    // save start of this SOBs tiles
  *startTile = *fieldTilessize / (8 * 8);

  // append tiles
  memcpy (*fieldTiles + *fieldTilessize, tiles, sizeOfTiles);

  // update sizes
  *fieldTilessize += sizeOfTiles;
  
  if (copyPal) {
  
    // realloc fieldPalette
    if (*fieldPaletteSize) {
      *fieldPalette = (u16*)realloc (*fieldPalette,
                                     *fieldPaletteSize + sizeOfPal);
    } else {
      *fieldPalette = (u16*)malloc (sizeOfPal);
    }
    if (!*fieldPalette) panic ("Can't realloc");

    // append palette
    memcpy ((u8*)(*fieldPalette) + *fieldPaletteSize, pal, sizeOfPal);
    
    // convert color values
    for (int i = 0; i < sizeOfTiles; i++) {
      u8 *tilePtr = *fieldTiles + *startTile * 8 * 8 + i;
      
      if (*(tilePtr) != 0) {
      *(tilePtr) += (*fieldPaletteSize / sizeof (u16)) - 1;
      }
    }
    *fieldPaletteSize += sizeOfPal;
  }
}

void Sob::initSOBs (u8 **fieldTiles, u16 **fieldPalette,
                    int *fieldTilessize, int *fieldPaletteSize) {
  Sob::copyTiles (fieldTiles, fieldPalette, fieldTilessize, fieldPaletteSize,
                  blankTile, sizeof (blankTile), blankPal, sizeof (blankPal),
                  &blankStartTile);
}

void Sob::init() {
  sobTiles = tiles;
  sobStartTile = startTile;

  currentAnimStep = 0;
  animTimer = 0;

  show();
}

void Sob::advance() {
  if (animSteps.size() > 0) {
    animTimer++;
    AnimStep *as = &animSteps[currentAnimStep];

    if (animTimer >= as->duration) {
      animTimer = 0;
      if (++currentAnimStep >= (int)animSteps.size()) currentAnimStep = 0;

      as = &animSteps[currentAnimStep];
      tiles = sobTiles + (as->frame * width * height * 8 * 8);

      startTile = sobStartTile + as->frame * width * height;
      show();
    }
  }
}

void Sob::show() {
  int i = 0;
  for (int h = 0; h < height; h++) {
    for (int w = 0; w < width; w++) {
      showTile (i, x + w * 8, y + h * 8, startTile + i);
      i++;
    }
  }
  visible = true;
}

void Sob::hide() {
  int i = 0;
  for (int h = 0; h < height; h++) {
    for (int w = 0; w < width; w++) {
      hideTile (i++, x + w * 8, y + h * 8);
    }
  }
  visible = false;
}

void Sob::deleteSelf (bool sendPacket) {
  if (killer != -1) {
    retrorocket->ships[killer]->player->score += score;
  }

  Sob::hide();

  if (sendPacket) retrorocket->network->sendSobDeletePacket (this);

  // remove from sobs[]
  vector<Sob*>::iterator i = retrorocket->sobs.begin();
  while (i != retrorocket->sobs.end()) {
    if (*i == this) {
      retrorocket->sobs.erase (i);
      break;
    }
    i++;
  }

#ifndef NDS
  // DEBUG
  printf ("---del---\n");
  for (unsigned int i = 0; i < retrorocket->sobs.size(); i++) {
    printf ("DB: %d %d\n", i, retrorocket->sobs[i]->sobType);
  }
#endif

  // free memory
  delete this;
}

int Sob::getID() {
  for (unsigned int i = 0; i < retrorocket->sobs.size(); i++) {
    if (retrorocket->sobs[i] == this) return i;
  }
  panic ("SOB not in vector");
  return -1;
}

void Sob::performCollision (int damage, int shooter) {
  strength -= damage;
  if (strength < 0) {
    this->killer = shooter;
    deleteSelf();
  }
}

bool Sob::getCPixel (Fix32 xx, Fix32 yy, int damage, int shooter) {
  if (!visible) {
    return false;
  }
  // bounding box
  if ((xx < x) || (xx >= right) ||
      (yy < y) || (yy >= bottom)) {
    return false;
  }
  // cmap
  int diffX = (xx-x).toUInt();
  int diffY = (yy-y).toInt();

  int tile = ((diffX >> 3) & wmask) + (((diffY >> 3) & hmask) * width);
  int pos = (diffX & 7) + ((diffY & 7) * 8);
  bool collision = *(tiles + tile * 8 * 8 + pos);
  if (collision) {
    performCollision (damage, shooter);
  }
  return collision;
}

///////////////////////////////////////////////////////////////////////////////

int BallMount::ballMountStartTile;

BallMount::BallMount (Fix32 x, Fix32 y) : Sob (x, y) {
  tiles = ballMountTiles;
  width = 2;
  height = 1;
  findMask (&wmask, &hmask, width, height);
  startTile = ballMountStartTile;
  sobType = BALLMOUNT;
  right = x + (Fix32)8 * width;
  bottom = y + (Fix32)8 * height;
}

void BallMount::advance() {
}

void BallMount::performCollision (int damage, int shooter) {}

void BallMount::copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                      int *fieldTilessize, int *fieldPaletteSize) {
  Sob::copyTiles (fieldTiles, fieldPalette, fieldTilessize, fieldPaletteSize,
                  ballMountTiles, sizeof (ballMountTiles),
                  ballMountPal, sizeof (ballMountPal),
                  &ballMountStartTile);
}

///////////////////////////////////////////////////////////////////////////////

int Fuel::fuelStartTile;

Fuel::Fuel (Fix32 x, Fix32 y) : Sob (x, y) {
  tiles = fuelTiles;
  width = 2;
  height = 2;
  findMask (&wmask, &hmask, width, height);
  startTile = fuelStartTile;
  strength = retrorocket->gameFile->getInt ("fuel_strength");
  capacity = retrorocket->gameFile->getInt ("fuel_sob_capacity");
  score = retrorocket->gameFile->getInt ("fuel_destroy_score");
  gas_refill_rate = retrorocket->gameFile->getInt ("gas_refill_rate");
  sobType = FUEL;
  right = x + (Fix32)8 * width;
  bottom = y + (Fix32)8 * height;
}

void Fuel::performBeamAction (Ship *ship) {
  capacity -= gas_refill_rate;
  if (capacity <= 0) {
    ship->player->gas += retrorocket->gameFile->getInt ("fuel_sob_capacity");
    retrorocket->network->sendConfPacket (ship);
    ship->player->score += retrorocket->gameFile->getInt ("fuel_fill_score");
    Sob::deleteSelf();
  }
}

void Fuel::advance() {
}

void Fuel::copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                      int *fieldTilessize, int *fieldPaletteSize) {
  Sob::copyTiles (fieldTiles, fieldPalette, fieldTilessize, fieldPaletteSize,
                  fuelTiles, sizeof (fuelTiles), fuelPal, sizeof (fuelPal),
                  &fuelStartTile);
}

///////////////////////////////////////////////////////////////////////////////

PowerPlant *PowerPlant::powerPlant;

int PowerPlant::powerplantStartTile;

PowerPlant::PowerPlant (Fix32 x, Fix32 y) : Sob (x, y) {
  tiles = powerplantTiles;

  width = 3;
  height = 3;
  findMask (&wmask, &hmask, width, height);
  startTile = powerplantStartTile;

  inactiveCounter = 0;
  active = true;
  powerPlant = this;
  animTimer = 0;
  currentFrame = 0;
  totalFrames = 5;

  sobType = POWERPLANT;

  right = x + (Fix32)8 * width;
  bottom = y + (Fix32)8 * height;
}

void PowerPlant::advance() {
  if (inactiveCounter > 0) {
    inactiveCounter--;
  } else {
    active = true;
  }

  if (active) {
    currentFrame++;
    if (currentFrame >= totalFrames) currentFrame = 0;
    setBgTile (SOB_BG, x + 16, y,
               startTile + width * height + currentFrame);
  }
}

void PowerPlant::performCollision (int damage, int shooter) {
  if (damage > retrorocket->gameFile->getInt ("powerplant_strength")) {
    // FIXME: only visible effect on master
    inactiveCounter = retrorocket->gameFile->getInt ("powerplant_inactive_time");
    active = false;
    setBgTile (SOB_BG, x + 16, y, blankStartTile);
    int d = retrorocket->gameFile->getInt ("particle_damage");
    int s = retrorocket->gameFile->getInt ("star_spread_speed");
    retrorocket->manager->particleCircle (16, 8,
                                   getCenterX(), getCenterY(),
                                   0, 0, d, s);
  }
}

void PowerPlant::copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                      int *fieldTilessize, int *fieldPaletteSize) {
  Sob::copyTiles (fieldTiles, fieldPalette, fieldTilessize, fieldPaletteSize,
                  powerplantTiles, sizeof (powerplantTiles),
                  powerplantPal, sizeof (powerplantPal),
                  &powerplantStartTile);
}

///////////////////////////////////////////////////////////////////////////////

int StrengthBar::strengthBarStartTile;

StrengthBar::StrengthBar (Fix32 x, Fix32 y, Sob *parent) : Sob (x, y) {
  tiles = strengthBarTiles;
  startTile = strengthBarStartTile;

  width = 2;
  height = 1;
  findMask (&wmask, &hmask, width, height);

  AnimStep as = {5, 1};
  animSteps.push_back (as);

  sobType = STRENGTHBAR;

  right = x + (Fix32)8 * width;
  bottom = y + (Fix32)8 * height;

  parentSob = parent;
}

void StrengthBar::displayStrength (int percent, bool sendPacket) {
  int currentFrame = (percent+12) / 20;
  if ((currentFrame != animSteps[0].frame) && sendPacket) {
    retrorocket->network->sendSobStrengthUpdatePacket (parentSob);
  }
  AnimStep as = {currentFrame, 1};
  animSteps.clear();
  animSteps.push_back (as);
}

void StrengthBar::advance() {
  Sob::advance();
}

void StrengthBar::copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                      int *fieldTilessize, int *fieldPaletteSize) {
  Sob::copyTiles (fieldTiles, fieldPalette, fieldTilessize, fieldPaletteSize,
                  strengthBarTiles, sizeof (strengthBarTiles),
                  strengthBarPal, sizeof (strengthBarPal),
                  &strengthBarStartTile);
}


///////////////////////////////////////////////////////////////////////////////

int RRMarker::markerStartTile;

RRMarker::RRMarker (Fix32 x, Fix32 y, int owner) : Sob (x, y) {
  tiles = markerTiles;
  startTile = markerStartTile;
  sobType = MARKER;
  this->owner = owner;
  width = 1;
  height = 1;
  findMask (&wmask, &hmask, width, height);
  right = x + (Fix32)8 * width;
  bottom = y + (Fix32)8 * height;
}

void RRMarker::advance() {
  Sob::advance();
}

void RRMarker::init() {
  loadBgPal (SOB_BG, owner, retrorocket->playerPals[owner]);
  Sob::init();
}

void RRMarker::showTile (int index, Fix32 x, Fix32 y, int tile) {
  setBgTile (SOB_BG, x, y, tile, owner);
}

void RRMarker::copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                      int *fieldTilessize, int *fieldPaletteSize) {
  Sob::copyTiles (fieldTiles, fieldPalette, fieldTilessize, fieldPaletteSize,
                  markerTiles, sizeof (markerTiles),
                  NULL, 0,
                  &markerStartTile, false);
}

///////////////////////////////////////////////////////////////////////////////

int Resource::resourceStartTile;

Resource::Resource (Fix32 x, Fix32 y) : Sob (x, y) {
  tiles = resourceTiles;
  sobType = RESOURCE;
  width = 2;
  height = 3;
  findMask (&wmask, &hmask, width, height);
  startTile = resourceStartTile;
  right = x + (Fix32)8 * width;
  bottom = y + (Fix32)8 * height;
}

void Resource::advance() {
}

void Resource::copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                          int *fieldTilessize, int *fieldPaletteSize) {
  Sob::copyTiles (fieldTiles, fieldPalette, fieldTilessize, fieldPaletteSize,
                  resourceTiles, sizeof (resourceTiles),
                  resourcePal, sizeof (resourcePal),
                  &resourceStartTile);
}

void Resource::performCollision (int damage, int shooter) {}

bool Resource::getCPixel (Fix32 xx, Fix32 yy, int damage, int shooter) {
  if (!visible) {
    return false;
  }
  // bounding box
  if ((xx < x) || (xx >= right) ||
      (yy < y) || (yy >= bottom)) {
    return false;
  }
  return true;
}


///////////////////////////////////////////////////////////////////////////////

int ResourceNode::resourceNodeStartTile;

ResourceNode::ResourceNode (Fix32 x, Fix32 y, int owner) : Sob (x, y) {

  tiles = resourceNodeTiles;
  this->owner = owner;
  sobType = NODE;
  timer = 0;
  width = 2;
  height = 2;

  AnimStep as = {0, 60};
  animSteps.push_back (as);
  AnimStep as2 = {1, 60};
  animSteps.push_back (as2);

  findMask (&wmask, &hmask, width, height);
  startTile = resourceNodeStartTile;
  maxStrength = strength = retrorocket->gameFile->getInt("building_strength"); 

  strengthBar = new StrengthBar (x+16, y+16, this);
  strengthBar->init();

  marker = new RRMarker (x-8, y+16, owner);
  marker->init();

  right = x + (Fix32)8 * width;
  bottom = y + (Fix32)8 * height;
  drawSOBsOnMap();
}

void ResourceNode::advance() {
  timer++;
  if ((timer % retrorocket->ships[owner]->player->nodeGatherRate.toUInt())  == 0 ) {
    retrorocket->ships[owner]->player->credits++;
  }
  Sob::advance();
  strengthBar->displayStrength((100*strength.toUInt())/maxStrength.toUInt());
}

void ResourceNode::performCollision (int damage, int shooter) {
  strength -= (Fix32)damage / retrorocket->players[owner]->buildingStrengthFactor;
  playSound(retrorocket->particleSound, retrorocket->particleSoundSize,
            retrorocket->config->getInt("volume_particle_ship"),
            x, y, retrorocket->config->getInt("rate_particle_ship"));
  if (strength < 0) {
    this->killer = shooter;
    deleteSelf();
  }
}

void ResourceNode::copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                              int *fieldTilessize, int *fieldPaletteSize) {
  Sob::copyTiles (fieldTiles, fieldPalette, fieldTilessize, fieldPaletteSize,
                  resourceNodeTiles, sizeof (resourceNodeTiles),
                  resourceNodePal, sizeof (resourceNodePal),
                  &resourceNodeStartTile);
}

///////////////////////////////////////////////////////////////////////////////

int ButtonRight::buttonrightStartTile;

ButtonRight::ButtonRight (Fix32 x, Fix32 y) : Sob (x, y) {
  tiles = buttonrightTiles;

  width = 1;
  height = 2;
  findMask (&wmask, &hmask, width, height);
  startTile = buttonrightStartTile;
  sobType = BUTTON;

  right = x + (Fix32)8 * width;
  bottom = y + (Fix32)8 * height;
}

void ButtonRight::advance() {
}

void ButtonRight::performCollision (int damage, int shooter) {}

void ButtonRight::copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                      int *fieldTilessize, int *fieldPaletteSize) {
  Sob::copyTiles (fieldTiles, fieldPalette, fieldTilessize, fieldPaletteSize,
                  buttonrightTiles, sizeof (buttonrightTiles),
                  buttonrightPal, sizeof (buttonrightPal),
                  &buttonrightStartTile);
}

///////////////////////////////////////////////////////////////////////////////

int ButtonLeft::buttonleftStartTile;

ButtonLeft::ButtonLeft (Fix32 x, Fix32 y) : Sob (x, y) {
  tiles = buttonleftTiles;

  width = 1;
  height = 2;
  findMask (&wmask, &hmask, width, height);
  startTile = buttonleftStartTile;
  sobType = BUTTON;

  right = x + (Fix32)8 * width;
  bottom = y + (Fix32)8 * height;
}

void ButtonLeft::advance() {
}

void ButtonLeft::performCollision (int damage, int shooter) {}

void ButtonLeft::copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                      int *fieldTilessize, int *fieldPaletteSize) {
  Sob::copyTiles (fieldTiles, fieldPalette, fieldTilessize, fieldPaletteSize,
                  buttonleftTiles, sizeof (buttonleftTiles),
                  buttonleftPal, sizeof (buttonleftPal),
                  &buttonleftStartTile);
}

///////////////////////////////////////////////////////////////////////////////

Gun::Gun (Fix32 x, Fix32 y) : Sob (x, y) {
  gunstation_rate = retrorocket->gameFile->getInt ("gunstation_rate");

  width = 3;
  height = 2;
  score = retrorocket->gameFile->getInt ("gun_score");
  findMask (&wmask, &hmask, width, height);
  strength = retrorocket->gameFile->getInt ("gun_strength");
  gunstation_reaction_distance =
    retrorocket->gameFile->getFloat ("gunstation_reaction_distance");

  sobType = GUN;

  right = x + (Fix32)8 * width;
  bottom = y + (Fix32)8 * height;
}

void Gun::advance() {

  if (visible && PowerPlant::powerPlant->active) {
    bool shoot = false;

    for (unsigned int i = 0; i < retrorocket->ships.size(); i++) {
      Mob *mob = retrorocket->ships[i];

      Fix32 distanceToMob = mob->distanceTo (this);

      if (distanceToMob < gunstation_reaction_distance) {
        shoot = true;
        break;
      }
    }
    
    if (shoot && !(rand() % gunstation_rate)) {
      Fix32 alfa = (Fix32)(rand() % DEG90) + angleOffset;
      Fix32 bulletSpeed = retrorocket->gameFile->getFloat ("bullet_speed");
      Fix32 vx = vectorXComp (alfa, bulletSpeed);
      Fix32 vy = vectorYComp (alfa, bulletSpeed);
      retrorocket->manager->newParticle (x + gunX, y + gunY, vx, vy,
                                  retrorocket->gameFile->getInt ("gunstation_damage"));
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

int GunNE::gunneStartTile;

GunNE::GunNE (Fix32 x, Fix32 y) : Gun (x, y) {
  tiles = gunneTiles;
  startTile = gunneStartTile;
  angleOffset = DEG270;
  gunX = 16;
  gunY = -4;
}

void GunNE::copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                      int *fieldTilessize, int *fieldPaletteSize) {
  Sob::copyTiles (fieldTiles, fieldPalette, fieldTilessize, fieldPaletteSize,
                  gunneTiles, sizeof (gunneTiles),
                  gunnePal, sizeof (gunnePal),
                  &gunneStartTile);
}

///////////////////////////////////////////////////////////////////////////////

int GunNW::gunnwStartTile;

GunNW::GunNW (Fix32 x, Fix32 y) : Gun (x, y) {
  tiles = gunnwTiles;
  startTile = gunnwStartTile;
  angleOffset = DEG180;
  gunX = 8;
  gunY = -4;
}

void GunNW::copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                      int *fieldTilessize, int *fieldPaletteSize) {
  Sob::copyTiles (fieldTiles, fieldPalette, fieldTilessize, fieldPaletteSize,
                  gunnwTiles, sizeof (gunnwTiles),
                  gunnwPal, sizeof (gunnwPal),
                  &gunnwStartTile);
}

///////////////////////////////////////////////////////////////////////////////

int GunSE::gunseStartTile;

GunSE::GunSE (Fix32 x, Fix32 y) : Gun (x, y) {
  tiles = gunseTiles;
  startTile = gunseStartTile;
  angleOffset = 0;
  gunX = 16;
  gunY = 20;
}

void GunSE::copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                      int *fieldTilessize, int *fieldPaletteSize) {
  Sob::copyTiles (fieldTiles, fieldPalette, fieldTilessize, fieldPaletteSize,
                  gunseTiles, sizeof (gunseTiles),
                  gunsePal, sizeof (gunsePal),
                  &gunseStartTile);
}

///////////////////////////////////////////////////////////////////////////////

int GunSW::gunswStartTile;

GunSW::GunSW (Fix32 x, Fix32 y) : Gun (x, y) {
  tiles = gunswTiles;
  startTile = gunswStartTile;
  angleOffset = DEG90;
  gunX = 8;
  gunY = 20;
}

void GunSW::copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                      int *fieldTilessize, int *fieldPaletteSize) {
  Sob::copyTiles (fieldTiles, fieldPalette, fieldTilessize, fieldPaletteSize,
                  gunswTiles, sizeof (gunswTiles),
                  gunswPal, sizeof (gunswPal),
                  &gunswStartTile);
}

///////////////////////////////////////////////////////////////////////////////

int SentryGun::sentryGunStartTile;

SentryGun::SentryGun (Fix32 x, Fix32 y, Fix32 angle, SentryType type) :
  Sob (x, y) {

  hasOwner = false;
  setupObject (x, y, angle, -1, type);
}

SentryGun::SentryGun (Fix32 x, Fix32 y, Fix32 angle, int owner, SentryType type) :
  Sob (x, y) {

  hasOwner = true;
  setupObject (x, y, angle, owner, type);
  drawSOBsOnMap();
}

void SentryGun::setupObject (Fix32 x, Fix32 y, Fix32 angle,
                             int owner, SentryType type) {
  gunstation_rate = retrorocket->gameFile->getInt ("gunstation_rate");
  seek_radius = retrorocket->gameFile->getInt ("seek_radius");
  bullet_speed = retrorocket->gameFile->getFloat ("bullet_speed");
  volume_shot = retrorocket->config->getInt("volume_shot");
  volume_tracking = retrorocket->config->getInt("volume_tracking");
  period_tracking = retrorocket->config->getInt("period_tracking");
  
  this->owner = owner;
  sobType = SENTRY;
  width = 2;
  height = 2;
  findMask (&wmask, &hmask, width, height);
  tiles = sentryGunTiles;
  startTile = sentryGunStartTile;
  sentryType = type;
  
  rotatePoint (11, 0, angle, &gunX, &gunY);
  gunX += 8;
  gunY += 8;
  angleOffset = angle;
  if (hasOwner) strength = retrorocket->gameFile->getInt("building_strength");
  else strength = retrorocket->gameFile->getInt ("gun_strength");
  maxStrength = strength;
  strengthBar = new StrengthBar (x+16, y+16, this);
  if (hasOwner) {
    marker = new RRMarker (x-8, y+16, owner);
  } else {
    marker = NULL;
  }
  advanceCounter = 0;

  AnimStep as = {0,1};
  animSteps.push_back (as);

  tracking = false;
  trackingPeriodCounter = 0;
  
  right = x + (Fix32)8 * width;
  bottom = y + (Fix32)8 * height;
}

void SentryGun::init() {
  Sob::init();
  strengthBar->init();
  if (marker) marker->init();
}

void SentryGun::performCollision (int damage, int shooter) {
  if (!hasOwner) {
    strength -= damage;
  } else {
    strength -= (Fix32) damage / retrorocket->players[owner]->
      buildingStrengthFactor;
  }
  playSound(retrorocket->particleSound, retrorocket->particleSoundSize,
            retrorocket->config->getInt("volume_particle_ship"),
            x, y, retrorocket->config->getInt("rate_particle_ship"));

  if (strength < 0) {
    this->killer = shooter;
    deleteSelf();
  }
}

void SentryGun::fire (Fix32 angle, bool sendPacket) {
  Fix32 bulletSpeed;
  if (hasOwner) bulletSpeed = retrorocket->ships[owner]->player->bulletSpeed;
  else bulletSpeed = bullet_speed;
  Fix32 vx = vectorXComp (angle, bulletSpeed);
  Fix32 vy = vectorYComp (angle, bulletSpeed);
  int damage = retrorocket->gameFile->getInt ("sentry_damage");
  Particle *p;
  if (hasOwner) {
    damage = (((Fix32)damage) *
              retrorocket->players[owner]->damageScaler).toInt();
    p = retrorocket->ships[owner]->
      newParticle (x + gunX, y + gunY, vx, vy, damage);
  } else {
    p = retrorocket->manager->newParticle (x + gunX, y + gunY, vx, vy, damage);
  }
  if (sentryType == HEATSEEKER) {
    p->particleType = Particle::HEATSEEKER;
  }
  if (sendPacket) retrorocket->network->sendSentryActionPacket (getID(), angle);
  playSound (retrorocket->shootBSound, retrorocket->shootBSoundSize,
             volume_shot, x, y);
}  

void SentryGun::advance() {
  if (visible) {
    strengthBar->displayStrength ((100*strength.toUInt()) /
                                  maxStrength.toUInt());

    int rate;
    if (hasOwner) rate = retrorocket->ships[owner]->player->sentryRate;
    else rate = gunstation_rate;

    advanceCounter++;
    trackingPeriodCounter++;

    // proceed if fire rate, proximity anim rate or tracking period
    if ((advanceCounter > rate) ||
        !(advanceCounter % 10)  ||
        (trackingPeriodCounter > period_tracking)) {

      // testing proximity
      Ship *target = NULL;
      Fix32 distance;

      if (hasOwner) {
        Ship::getClosestEnemyShip(&target, owner, &distance,
                                  getCenterX(), getCenterY());
      } else {
        Ship::getClosestShip(&target, &distance,
                             getCenterX(), getCenterY());
      }

      bool trackingNow = (distance < seek_radius) && target;
      // dont track if target is landed on home platform
      if (target) {
        if (retrorocket->field->landed (target) & Field::HOME) {
          trackingNow = false;
        }
      }
      
      if (!tracking && trackingNow) {
        animSteps.clear();        
        AnimStep as = {1, 1};
        animSteps.push_back (as);
        advanceCounter = 0;
        trackingPeriodCounter = period_tracking/2;
      } else if (tracking && !trackingNow) {
        animSteps.clear();        
        AnimStep as = {0, 1};
        animSteps.push_back (as);
      } 

      tracking = trackingNow;
      
      if (tracking && (trackingPeriodCounter > period_tracking)) {
        playSound (retrorocket->trackingSound, retrorocket->trackingSoundSize,
                   volume_tracking, x, y);
        trackingPeriodCounter = 0;        
      }
      
      if (advanceCounter > rate) {
        advanceCounter = 0;
        if (tracking) {
          if ((!hasOwner && retrorocket->network->master) ||
              (retrorocket->players[owner]->isLocalPlayer())) {
            switch (sentryType) {
              default:
              case RANDOM: {
                Fix32 randomAngle = Fix32 (rand() % DEG45);
                int sign = rand() % 2;
                Fix32 angle = angleOffset +
                  (Fix32)(sign ? -1 : 1) * randomAngle;
                fire (angle);
                break;
              }
              case DIRECTED: {
                // find target vector
                Fix32 tx = target->getCenterX() - getCenterX();
                Fix32 ty = target->getCenterY() - getCenterY();
                // find angle of target vector
                Fix32 targetAngle = vectorAngle (tx, ty);
                fire (targetAngle);
                break;
              }
              case HEATSEEKER: {
                // find time t to hit
                Fix32 Vsy = target->vy;
                Fix32 Vsx = target->vx;
                Fix32 Ys = target->getCenterY();
                Fix32 Xs = target->getCenterX();
                Fix32 Vp = bullet_speed;
                Fix32 Yp = getCenterY();
                Fix32 Xp = getCenterX();

                Fix32 a = Vp*Vp - Vsy*Vsy - Vsx*Vsx;
                Fix32 b = Vp*Vsy + Xp*Vsx - Ys*Vsy - Xs*Vsx;
                Fix32 c = Ys*Yp + Xs*Xp - Ys*Ys - Yp*Yp - Xs*Xs - Xp*Xp;

                Fix32 t = (-b + (b*b-(Fix32)4*a*c).sqroot() ) / ((Fix32)2*a) ;
                      
                // find target vector
                Fix32 tx = target->getCenterX() + Vsx*t - getCenterX();
                Fix32 ty = target->getCenterY() + Vsy*t - getCenterY();
                // find angle of target vector
                Fix32 targetAngle = vectorAngle (tx, ty);
                if ( (targetAngle > (angleOffset - DEG90)) &&
                     (targetAngle < (angleOffset + DEG90)) ) {
                  fire (targetAngle);
                }
                break;
              }
            }
          }
        }
      }
    }
  }    
  Sob::advance();
}

void SentryGun::copyTiles (u8 **fieldTiles, u16 **fieldPalette,
			   int *fieldTilessize, int *fieldPaletteSize) {
  Sob::copyTiles (fieldTiles, fieldPalette, fieldTilessize, fieldPaletteSize,
                  sentryGunTiles, sizeof (sentryGunTiles),
                  sentryGunPal, sizeof (sentryGunPal),
                  &sentryGunStartTile);
}

///////////////////////////////////////////////////////////////////////////////

int Door::doorStartTile;

Door::Door (Fix32 x, Fix32 y, Fix32 x2, Fix32 y2) : Sob (x, y) {
  tiles = NULL;
  sobType = DOOR;
  width = ((x2 - x) / 8).toInt();
  height = ((y2 - y) / 8).toInt();
  findMask (&wmask, &hmask, width, height);

  startTile = doorStartTile;
  
  open = false;

  right = x + (Fix32)8 * width;
  bottom = y + (Fix32)8 * height;
}

void Door::openDoor() {
  counter = retrorocket->gameFile->getInt ("doortime");
  open = true;
  hide();
}

void Door::advance() {
  if (open) {
    if (counter-- <= 0) {
      show();
      open = false;
    }
  }
}

void Door::show() {
  int i = 0;
  for (int h = 0; h < height; h++) {
    for (int w = 0; w < width; w++) {
      showTile (i, x + w * 8, y + h * 8, startTile);
      i++;
    }
  }
  visible = true;
}

bool Door::getCPixel (Fix32 xx, Fix32 yy, int damage, int shooter) {
  if (!visible) {
    return false;
  }
  if ((xx < x) || (yy < y) || (xx > right) || (yy > bottom)) {
    return false;
  }
  return true;
}

void Door::copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                      int *fieldTilessize, int *fieldPaletteSize) {
  Sob::copyTiles (fieldTiles, fieldPalette, fieldTilessize, fieldPaletteSize,
                  solidTile, sizeof (solidTile),
                  solidPal, sizeof (solidPal),
                  &doorStartTile);
}

///////////////////////////////////////////////////////////////////////////////

int PlatformSOB::platformStartTile;

PlatformSOB::PlatformSOB (Fix32 x, Fix32 y, int owner) : Sob (x, y) {
  this->owner = owner;
  sobType = PLATFORM;
  tiles = platformTiles;
  width = platformWidth()/8;
  height = 1;
  findMask (&wmask, &hmask, width, height);
  startTile = platformStartTile;
  platform_strength = retrorocket->gameFile->getInt("platform_strength");
  
  advanceCounter = 0;

  right = x + (Fix32)8 * width;
  bottom = y + (Fix32)8 * height;
}

void PlatformSOB::init() {
  Sob::init();

  if (owner >= retrorocket->network->numberOfShips) {
    owner = -1;
  }

  maxStrength = strength = platform_strength;

  if (owner >= 0) {
    marker = new RRMarker (x, y+8, owner);
    marker->init();
    if (retrorocket->rtsMode) {
      strengthBar = new StrengthBar (x+((width-1)*8), y+8, this);
      strengthBar->init();
    }
  }
}

void PlatformSOB::advance() {
  advanceCounter++;
  if (retrorocket->rtsMode && (owner >= 0)) {
    strengthBar->displayStrength ((100*strength.toUInt())/maxStrength.toUInt());
  }
}

void PlatformSOB::performCollision (int damage, int shooter) {
  if (retrorocket->rtsMode && (strength != -1)) {
    if (owner > -1) { 
      strength -= Fix32(damage) /
        retrorocket->players[owner]->buildingStrengthFactor;
    } else {
      strength -= damage;
    }
    playSound(retrorocket->particleSound, retrorocket->particleSoundSize,
              retrorocket->config->getInt("volume_particle_ship"),
              x, y, retrorocket->config->getInt("rate_particle_ship"));

    if (strength <= 0) {
      this->killer = shooter;
      deleteSelf();
    }
  }
}

void PlatformSOB::copyTiles (u8 **fieldTiles, u16 **fieldPalette,
                             int *fieldTilessize, int *fieldPaletteSize) {
  Sob::copyTiles (fieldTiles, fieldPalette, fieldTilessize, fieldPaletteSize,
                  platformTiles, sizeof (platformTiles),
                  platformPal, sizeof (platformPal),
                  &platformStartTile);
}

///////////////////////////////////////////////////////////////////////////////

void Gun::deleteSelf (bool sendPacket) {
  int d = retrorocket->gameFile->getInt ("particle_damage");
  Fix32 s = retrorocket->gameFile->getFloat ("star_spread_speed");
  retrorocket->manager->particleCircle (0, 8, getCenterX(), getCenterY(),
                                 0, 0, d, s);
  Sob::deleteSelf (sendPacket);
}

void ButtonLeft::deleteSelf (bool sendPacket) {
  Sob::deleteSelf (sendPacket);
}

void ButtonRight::deleteSelf (bool sendPacket) {
  Sob::deleteSelf (sendPacket);
}

void Door::deleteSelf (bool sendPacket) {
  Sob::deleteSelf (sendPacket);
}

void RRMarker::deleteSelf (bool sendPacket) {
  Sob::deleteSelf (sendPacket);
}

void BallMount::deleteSelf (bool sendPacket) {
  Sob::deleteSelf (sendPacket);
}

void StrengthBar::deleteSelf (bool sendPacket) {
  Sob::deleteSelf (sendPacket);
}

void Fuel::deleteSelf (bool sendPacket) {
  int d = retrorocket->gameFile->getInt ("particle_damage");
  Fix32 s = retrorocket->gameFile->getFloat ("star_spread_speed");
  retrorocket->manager->particleCircle (0, 8, getCenterX(), getCenterY(),
                                 0, 0, d, s);
  Sob::deleteSelf (sendPacket);
}

void PowerPlant::deleteSelf (bool sendPacket) {
  Sob::deleteSelf (sendPacket);
}

void Resource::deleteSelf (bool sendPacket) {
  Sob::deleteSelf (sendPacket);
}

void ResourceNode::deleteSelf (bool sendPacket) {
  int d = retrorocket->gameFile->getInt ("particle_damage");
  Fix32 s = retrorocket->gameFile->getFloat ("star_spread_speed");
  retrorocket->manager->particleCircle (0, 8, getCenterX(), getCenterY() - 8,
                                 0, 0, d, s);
  strengthBar->deleteSelf (false);
  marker->deleteSelf(false);
  Sob::deleteSelf (sendPacket);
  drawSOBsOnMap();
}

void SentryGun::deleteSelf (bool sendPacket) {
  int d = retrorocket->gameFile->getInt ("particle_damage");
  Fix32 s = retrorocket->gameFile->getFloat ("star_spread_speed");
  retrorocket->manager->particleCircle (0, 8, getCenterX(), getCenterY(),
                                 0, 0, d, s);
  strengthBar->deleteSelf (false);
  if (marker) marker->deleteSelf (false);
  Sob::deleteSelf (sendPacket);
  drawSOBsOnMap();
}

void PlatformSOB::deleteSelf (bool sendPacket) {
  int d = retrorocket->gameFile->getInt ("particle_damage");
  Fix32 s = retrorocket->gameFile->getFloat ("star_spread_speed");

  if (owner >= 0) {
    Ship* ship = retrorocket->ships[owner];
    ship->player->lives = 0;
    ship->player->dead = true;
    ship->strength = 0;
    retrorocket->playerDeathOrder.push_back (owner);
    retrorocket->players[owner]->setPermanentlyDead();
    strengthBar->deleteSelf (false);
    marker->deleteSelf(false);
    playSound (retrorocket->deadSound, retrorocket->deadSoundSize,
               retrorocket->config->getInt("volume_dead"), x, y);    
    int d = retrorocket->gameFile->getInt ("particle_damage");
    int s = retrorocket->gameFile->getInt ("star_spread_speed");
    retrorocket->manager->particleCircle (4, 16, ship->getCenterX(),
                                          ship->getCenterY(),
                                          ship->vx, ship->vy, d, s);
    retrorocket->manager->particleCircle (8, 16, ship->getCenterX(),
                                          ship->getCenterY(),
                                          ship->vx, ship->vy, d, 2*s);

    // delete all nodes and sentries owned by killed player
    vector<Sob*> sobsToDelete;
    
    for (unsigned int i = 0; i < retrorocket->sobs.size(); i++) {
      if (retrorocket->sobs[i]->owner == owner) {
        if (retrorocket->sobs[i]->sobType == NODE || 
            retrorocket->sobs[i]->sobType == SENTRY ) {
          sobsToDelete.push_back(retrorocket->sobs[i]);
        }
      }
    }
    for (unsigned int i = 0; i < sobsToDelete.size(); i++) {    
      sobsToDelete[i]->deleteSelf (false);
    }
  }

  retrorocket->manager->particleCircle (8, 12, getCenterX()-16, getCenterY(),
                                 0, 0, d, s/2);
  retrorocket->manager->particleCircle (16, 20, getCenterX(), getCenterY(),
                                 0, 0, d, s);
  retrorocket->manager->particleCircle (8, 12, getCenterX()+16, getCenterY(),
                                 0, 0, d, s/2);

  Sob::deleteSelf (sendPacket);
}


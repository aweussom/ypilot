/*
  utils.h
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
// Functions belonging nowhere else...
//
///////////////////////////////////////////////////////////////////////////////

#ifndef UTILS_H
#define UTILS_H

#ifdef NDS
#include <PA9.h>
#include <arm9/math.h>
#endif

#include "retrorocket.h"

///////////////////////////////////////////////////////////////////////////////

/** Fixed point number (1.19.12 bits)
    Used instead of floats for speed */
class Fix32 {

  s32 value;

public:
  const static int HUGEVAL = 524287;

  Fix32() { value = 0; }
  Fix32 (const Fix32 &v) { value = v.value; }
  Fix32 (int v) { value = v << 12; }
  //Fix32 (s32 v) { value = v << 12; }
  Fix32 (u64 v) { value = v << 12; }
  Fix32 (double v) { value = (int)(v * 4096); }

  // type conversion
  u32 toUInt() { return value >> 12; }  // fast, but incorrect if negative
  int toInt() {                         // medium fast
    if (value < 0) return -(abs().value >> 12);
    return value >> 12;
  }
  int toIntRound() {                    // slow
    return (int)round (toFloat());
  }
  float toFloat() { return (float)value / 4096; }

/*   operator int() { return toInt(); } */
/*   operator float() { return toFloat(); } */

  Fix32 sqroot() {
#ifdef NDS
    Fix32 f;
    f.value = sqrtf32 (value);
#else
    Fix32 f (sqrt (toFloat()));
#endif
    return f;
  }

  Fix32 arctan_rad() { return Fix32 (atan (toFloat())); }
  Fix32 cosinus_rad() { return Fix32 (cos (toFloat())); }
  Fix32 sinus_rad() { return Fix32 (sin (toFloat())); }

#ifdef NDS
  Fix32 cosinus_ds() {
    Fix32 f;
    f.value = PA_Cos (value >> 12) << 4;
    return f;
  }
  Fix32 sinus_ds() {
    Fix32 f;
    f.value = PA_Sin (value >> 12) << 4;
    return f;
  }
#else
  Fix32 cosinus_ds() { return dsToRad().cosinus_rad(); }
  Fix32 sinus_ds() { return dsToRad().sinus_rad(); }
#endif

  Fix32 dsToRad() { return (*this * 2 * M_PI) / DEG360; }
  Fix32 radToDS() { return (*this * DEG360) / (2 * M_PI); }

  Fix32 abs() {
    Fix32 f;
    if (value < 0) f.value = -value;
    else f.value = value;
    return f;
  }

  static Fix32 max (Fix32 a, Fix32 b) {
    if (a > b) return a;
    return b;
  }
  static Fix32 min (Fix32 a, Fix32 b) {
    if (a < b) return a;
    return b;
  }

  bool operator==(const Fix32 v) { return value == v.value; }
  bool operator!=(const Fix32 v) { return value != v.value; }
  bool operator<(const Fix32 v) { return value < v.value; }
  bool operator>(const Fix32 v) { return value > v.value; }
  bool operator<=(const Fix32 v) { return value <= v.value; }
  bool operator>=(const Fix32 v) { return value >= v.value; }
  void operator++(int i) { value += 1 << 12; }
  void operator++() { value += 1 << 12; }

  Fix32 operator%(Fix32 v) {
    return Fix32 (toInt() % v.toInt());
  }

  void operator&=(Fix32 v) {
    value &= v.value;
  }

  Fix32 operator&(Fix32 v) {
    Fix32 f;
    f.value = value & v.value;
    return f;
  }

  void operator+=(const Fix32 v) {
    value += v.value;
  }

  Fix32 operator+(const Fix32 v) {
    Fix32 f;
    f.value = value + v.value;
    return f;
  }

  void operator-=(const Fix32 v) {
    value -= v.value;
  }

  Fix32 operator-() {
    Fix32 f;
    f.value = -value;
    return f;
  }

  Fix32 operator-(const Fix32 v) {
    Fix32 f;
    f.value = value - v.value;
    return f;
  }

  void operator*=(const Fix32 v) {
    value = (((long long)value * (long long)v.value) >> 12);
  }

  Fix32 operator*(const Fix32 v) {
    Fix32 f;
    f.value = (((long long)value * (long long)v.value) >> 12);
    return f;
  }

  Fix32 operator/(const Fix32 v) {
    Fix32 f;
#ifdef NDS
    f.value = divf32 (value, v.value);
#else
    f.value = (((long long)value<<12) / v.value);
#endif
    return f;
  }
};

///////////////////////////////////////////////////////////////////////////////

/** print panic message and hang */
extern "C" void panic (const char *fmt, ...);

int getFileSize (const char *filename);
Fix32 getCurrentRanking();
void rotatePoint (Fix32 x, Fix32 y, Fix32 angle, Fix32 *xx, Fix32 *yy);
int readBinary (u8 **buffer, const char *filename);
void message (int y, const char *fmt, ...);
void message (const char *fmt, ...);
void infoText (const char *fmt, ...);
void waitKey (int key);
bool matchExt (const char *filename, const char *ext);
string stripSuffix (string str, string suffix);

static inline Fix32 vectorAngleAccurate (Fix32 x, Fix32 y) {
  Fix32 angle;
  if (x == 0) {
    if (y > 0) {
      angle = DEG90;
    } else {
      angle = -DEG90;
    }
  } else {
    angle = (y / x).arctan_rad().radToDS();
    if (x < 0) {
      angle += DEG180;
    }
  }
  return angle;
}

static inline Fix32 vectorAngle (Fix32 x, Fix32 y) {
#ifdef NDS
  return Fix32 (DEG360 - PA_GetAngle (0, 0, x.toInt(), y.toInt()));
#else
  return vectorAngleAccurate (x, y);
#endif
}

static inline Fix32 vectorLength (Fix32 x, Fix32 y) {
#ifdef NDS
  return Fix32 (PA_TrueDistance (0, 0, x.toInt(), y.toInt()));
#else
  return (x * x + y * y).sqroot();
#endif
}

static inline Fix32 vectorXComp (Fix32 angle, Fix32 length) {
  return angle.cosinus_ds() * length;
}

static inline Fix32 vectorYComp (Fix32 angle, Fix32 length) {
  return angle.sinus_ds() * length;
}

static inline Fix32 vectorAngleComp (Fix32 angle, Fix32 x, Fix32 y) {
  return vectorXComp (angle, x) + vectorYComp (angle, y);
}

static inline Fix32 vectorXCompAccurate (Fix32 angle, Fix32 length) {
  return angle.dsToRad().cosinus_rad() * length;
}

static inline Fix32 vectorYCompAccurate (Fix32 angle, Fix32 length) {
  return angle.dsToRad().sinus_rad() * length;
}

static inline int k_ipow(int base, int exp) {
  //integer pow 
  int res=1;
  for(int i=0;i<exp;i++)
    res=res*base;
  return res;
}

static inline int toAngleStep (Fix32 angle) {
  return angle.toInt() >> 4;
}

static inline Fix32 fromAngleStep (int angleSteps) {
  return Fix32 (angleSteps << 4);
}

static inline int vblToCSec (int vbl) {
  return (int)(vbl*100/60);
}

static inline int msb (int number) {
  if (!number) return 0;
  for (int i=0;;i++) {
    number = number >> 1;
    if (!number) return (1 << i);
  }
}

///////////////////////////////////////////////////////////////////////////////

#endif

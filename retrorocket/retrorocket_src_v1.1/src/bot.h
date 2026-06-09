#ifndef BOT_H_
#define BOT_H_
#include <iostream>
#include "player.h"
class Bot : public LocalPlayer 
{
protected:
  Fix32 simx;
  Fix32 simy;
  Fix32 tox,toy;
  bool liftoff;
  Fix32 startx;
  Fix32 starty;
  Fix32 angle;
  bool lastcollision;
  int counter;
  int checkahead;
  int maxsteps;
  int liftoffc;
  virtual void updateMenus() {}
public:
  Bot(unsigned int id);
  virtual ~Bot();
//   virtual void advance() {}
  virtual void setWeaponA (int weapon);
  virtual void setWeaponB (int weapon);
  virtual void advance();
  virtual void registerShip (Ship *ship);
  virtual void shopUpgrade (Upgrades upgrade, bool ignorePrice = false);
  static int getUpgradePrice (int level, const char* upgradePriceTag);
  virtual void setPermanentlyDead() {}
  virtual bool checkPossibleCollision(Fix32 x, Fix32 y);
  double speed(){return sqrt(pow(ship->vx.toFloat(),2) + pow(ship->vy.toFloat(),2));}
  int checkFutureCollision(Fix32 fx, Fix32 fy, int n);
  int checkFutureCollision(int upton);
  void setCheckAhead(int n){checkahead = n;}
  void setMaxSteps(int n){maxsteps = n;}
  void pursueclosest();
  //bool checkPossibleCollision(Fix32 nx, Fix32 ny);
};



#endif

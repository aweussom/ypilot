#ifndef BOTROCKET_H_
#define BOTROCKET_H_

#include "retrorocket.h"

class BotRocket : public RetroRocket  
{ 
protected: 
  int checkahead;
  int maxsteps;
public: 
  BotRocket(int n, int maxn) : RetroRocket () {checkahead = n; maxsteps = maxn;} 
  virtual ~BotRocket(){} 
  virtual void start(); 
//   static void joinGame (Menu *menu); 
  virtual void playMaster (bool autogen = false);
  void newBotGame ();
  void playGame(); 
  void registerShip(Ship * ship); 
}; 

#endif

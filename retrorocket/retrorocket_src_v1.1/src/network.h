/*
  network.h
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
// Networking classes
//
///////////////////////////////////////////////////////////////////////////////

#ifndef DSTODS_H
#define DSTODS_H

#include "retrorocket.h"
#include <iostream>

///////////////////////////////////////////////////////////////////////////////

// refuses to talk with other versions
#define NETPROTOCOL_VERSION  2

/** packet types */
enum {
  PACKET_ACTION,
  PACKET_SENTRYACTION,
  PACKET_STRENGTHUPDATE,
  PACKET_HEATSEEKER,
  PACKET_BALL,
  
  PACKET_CONF,
  PACKET_UPGRADE,
  PACKET_SOBCREATE,
  PACKET_SOBDELETE,
  PACKET_DELETEPARTICLE,

  PACKET_NEWGAME,
  PACKET_STARTACK,
  PACKET_STARTGAME,
  PACKET_ENDGAME,
  PACKET_ENDACK,

  PACKET_ENDSTATE,
  PACKET_ABORTGAME,
  PACKET_COLLECT_BALL,
  PACKET_NEXT_LEVEL,
  PACKET_PICKUP_BALL,
  PACKET_CREATE_BALL,

  PACKET_SOB_STRENGTH_UPDATE,

  PACKET_PING,
  PACKET_PONG,

  NUMBER_OF_PACKETS
};

///////////////////////////////////////////////////////////////////////////////

/** base packet struct */
struct Packet {
  int packettype;
  int timestamp;
  virtual int size() = 0;
  Packet();
  virtual ~Packet() {}
};

// new user input -> update ship state
struct ActionPacket : public Packet {
  int id;
  Shipstate shipstate;  

  ActionPacket() { packettype = PACKET_ACTION; }
  int size() { return sizeof (*this); }
};

// sentry gun fires
struct SentryActionPacket : public Packet {
  int id;
  Fix32 angle;
  
  SentryActionPacket() { packettype = PACKET_SENTRYACTION; }
  int size() { return sizeof (*this); }
};

// master decides strength
// sends this when a ship changes strength
struct StrengthUpdatePacket : public Packet {
  int id;
  int strength;
  int hitBy;

  StrengthUpdatePacket() { packettype = PACKET_STRENGTHUPDATE; }
  int size() { return sizeof (*this); }
};

// heat seeker
// sends this when heat seeker is seeking on ship that has an action
struct HeatSeekerPacket : public Packet {
  int id;
  ParticleState particleState;

  HeatSeekerPacket() { packettype = PACKET_HEATSEEKER; }
  int size() { return sizeof (*this); }
};

// packet with updated state of a ball
struct BallPacket : public Packet {
  int id;
  int owner;
  BallState ballState;

  BallPacket() { packettype = PACKET_BALL; }
  int size() { return sizeof (*this); }
};

// a ship has collected a ball
struct CollectBallPacket : public Packet {
  int shipID;

  CollectBallPacket() { packettype = PACKET_COLLECT_BALL; }
  int size() { return sizeof (*this); }
};

// a ship has picked up a ball
struct PickUpBallPacket : public Packet {
  int shipID;
  int ballID;
  
  PickUpBallPacket() { packettype = PACKET_PICKUP_BALL; }
  int size() { return sizeof (*this); }
};

struct BallCreatePacket : public Packet {
  int ballPosIndex;
  
  BallCreatePacket() { packettype = PACKET_CREATE_BALL; }
  int size() { return sizeof (*this); }
};


// change level
struct NextLevelPacket : public Packet {
  NextLevelPacket() { packettype = PACKET_NEXT_LEVEL; }
  int size() { return sizeof (*this); }
};

// ship has new configuration
// sends this e.g. when action on platform
struct ConfPacket : public Packet {
  int id;

  // ship data
  Fix32 gas;
  int weaponA;
  int weaponB;
  int roundsA;
  int roundsB;

  // player data
  int userMaxGas;
  int userMaxWeaponA;
  int userMaxWeaponB;

  ConfPacket() { packettype = PACKET_CONF; }
  int size() { return sizeof (*this); }
};

// sends this when user buys an upgrade
struct UpgradePacket : public Packet {
  int id;

  Upgrades upgrade;

  UpgradePacket() { packettype = PACKET_UPGRADE; }
  int size() { return sizeof (*this); }
};

// master sends this when new SOB is created
struct SobCreatePacket : public Packet {
  Sob::SobType type;
  Fix32 x, y;
  Fix32 angle;
  int owner;
  SentryGun::SentryType sentryType;

  SobCreatePacket() { packettype = PACKET_SOBCREATE; }
  int size() { return sizeof (*this); }
};

// master sends this when SOB is deleted
struct SobDeletePacket : public Packet {
  int id;
  int killer;

  SobDeletePacket() { packettype = PACKET_SOBDELETE; }
  int size() { return sizeof (*this); }
};

// master sends this when particle hits ship or ball
struct DeleteParticlePacket : public Packet {
  int id;

  DeleteParticlePacket() { packettype = PACKET_DELETEPARTICLE; }
  int size() { return sizeof (*this); }
};

// master starts new game
struct NewGamePacket : public Packet {
  char fieldFilename[FILENAMESIZE];

  bool autogen;
  int worldgenWidth;
  int worldgenHeight;
  int worldgenSeaLevel;
  int worldgenGraininess;
  int randomSeed;

  int players;
  int aiPlayers;
  int crc;
  int gameMode;

  NewGamePacket() { packettype = PACKET_NEWGAME; }
  int size() { return sizeof (*this); }
};

// client has loaded game and is ready to start
struct StartAckPacket : public Packet {
  int id;

  StartAckPacket() { packettype = PACKET_STARTACK; }
  char playerName[NAME_SIZE+1];
  int size() { return sizeof (*this); }
};

// master sends this when all clients are ready
struct StartGamePacket : public Packet {
  StartGamePacket() { packettype = PACKET_STARTGAME; }
  char playerNames[MAX_SHIPS][NAME_SIZE+1];
  int size() { return sizeof (*this); }
};

// master sends this when game ends
struct EndGamePacket : public Packet {
  EndGamePacket() { packettype = PACKET_ENDGAME; }
  int size() { return sizeof (*this); }
};

// client sends this to acknowledge game end
struct EndAckPacket : public Packet {
  int id;

  Fix32 ranking;
  int score;
  int bestLapTime; 
  bool bestLapMade;
  
  EndAckPacket() { packettype = PACKET_ENDACK; }
  int size() { return sizeof (*this); }
};

// master sends game results to clients to end game completely
struct EndStatePacket : public Packet {
  int winner;
  Fix32 ranking[MAX_SHIPS];
  int score[MAX_SHIPS];
  int lapTimes[MAX_SHIPS];
  bool bestLapMade[MAX_SHIPS];
  int playerResultPositions[MAX_SHIPS];
  EndStatePacket() { packettype = PACKET_ENDSTATE; }
  int size() { return sizeof (*this); }
};

// sent by any player choosing to quit
struct AbortGamePacket : public Packet {
  AbortGamePacket() { packettype = PACKET_ABORTGAME; }
  int size() { return sizeof (*this); }
};

// sent when a sob changes strength
struct SobStrengthUpdatePacket : public Packet {
  int id;
  Fix32 strength;
  SobStrengthUpdatePacket() { packettype = PACKET_SOB_STRENGTH_UPDATE; }
  int size() { return sizeof (*this); }
};

// Ping packet
struct PingPacket : public Packet {
  int vblcount;
  int senderid;
  PingPacket() { packettype = PACKET_PING; }
  int size() { return sizeof (*this); }
};

// Pong packet
struct PongPacket : public Packet {
  int pingvblcount;
  int pongvblcount;
  PongPacket() { packettype = PACKET_PONG; }
  int size() { return sizeof (*this); }
};

///////////////////////////////////////////////////////////////////////////////

/** Base class for network
    When playing, there is always a network object, even in single player.
    In single player, subclass NoNetwork is used
    In multi player, the network object returned from openNetwork*()
    in system.cpp is used
 */
class Network {

protected:
  char *fieldFilename;

  virtual void sendPacketToAll (Packet *packet) = 0;
  virtual void sendPacketToMaster (Packet *packet) = 0;
  virtual void sendPacketToUser (int userid, Packet *packet) = 0;
  virtual void sendPacketBroadcast (Packet *packet) = 0;

public:
  // game is autogen
  bool autogen;
  
  NewGamePacket newGamePacket;
  bool newGamePacketReceived;
  bool startGamePacketReceived;
  int startAckPacketsReceived;
  bool endGamePacketReceived;
  int endAckPacketsReceived;
  EndAckPacket endAckPackets[MAX_SHIPS];
  EndStatePacket endStatePacket;
  bool endStatePacketReceived;
  
  // list of received packets not yet processed
  vector<Packet*> packetQueue;

  int numberOfShips;

  int aiPlayers;

  // true if local player is master
  bool master;

  Network();
  virtual ~Network();

  /** must be called every VBL */
  virtual void advance();

  virtual void handlePacket (Packet *packet);

  virtual void sendPingPacket();
  virtual void sendActionPacket (Ship *ship);
  virtual void sendSentryActionPacket (int id, Fix32 angle);
  virtual void sendStrengthUpdatePacket (Ship *ship);
  virtual void sendHeatSeekerPacket (Particle *particle);
  virtual void sendBallPacket (Ball *ball);
  virtual void sendConfPacket (Ship *ship);
  virtual void sendUpgradePacket (int id, Upgrades upgrade);
  virtual void sendSobStrengthUpdatePacket (Sob *sob);
  virtual void sendSobCreatePacket (Sob *sob);
  virtual void sendSobDeletePacket (Sob *sob);
  virtual void sendDeleteParticlePacket (int id);
  virtual void sendCollectBallPacket (Ship *ship);
  virtual void sendPickUpBallPacket (Ship *ship, Ball *ball);
  virtual void sendNextLevelPacket();
  virtual void sendAbortGamePacket();
  virtual void sendBallCreatePacket(int ballPosIndex);

  // called by network subclasses to initiate a game
  virtual void sendNewGamePacket (int players, int aiPlayers, bool autogen,
                                  char *fieldFilename);
  virtual bool checkForNewGamePacket();

  // wait for all clients to get ready to start
  virtual void waitForStart();

  // called by master to initiate and complete ending protocol
  virtual void endGame (int winner); 

  // called by slaves after receiving endgamepacket, ending protocol
  virtual int waitForEndStats();  

};

///////////////////////////////////////////////////////////////////////////////

class NoNetwork : public Network {

protected:
  void sendPacketToAll (Packet *packet) {}
  void sendPacketToMaster (Packet *packet) {}
  void sendPacketToUser (int userid, Packet *packet) {}
  void sendPacketBroadcast (Packet *packet) {}

public:
  NoNetwork();
};

///////////////////////////////////////////////////////////////////////////////

#endif

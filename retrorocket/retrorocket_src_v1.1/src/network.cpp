/*
  network.cpp
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

Network::Network() {}

Network::~Network() {}

Packet::Packet() {
  timestamp = retrorocket->vblcount;
}

void Network::handlePacket (Packet *msg) {
  clearText(0, 0);
  //outputText(0,0, "Drift : %d    ", (retrorocket->vblcount - msg->timestamp));
  switch(msg->packettype) {
    case PACKET_ACTION: {
      ActionPacket *packet = (ActionPacket*)msg;
      Ship *ship = retrorocket->ships[packet->id];
      ship->newState (packet->shipstate);
      break;
    }
    case PACKET_SENTRYACTION: {
      SentryActionPacket *packet = (SentryActionPacket*)msg;
      ((SentryGun*)(retrorocket->sobs[packet->id]))->fire (packet->angle,
                                                           false);
      break;
    }
    case PACKET_STRENGTHUPDATE: {
      StrengthUpdatePacket *packet = (StrengthUpdatePacket*)msg;
      Ship *ship = retrorocket->ships[packet->id];
      ship->strength = packet->strength;
      ship->hitBy = packet->hitBy;
      if (ship->strength <= 0 && packet->hitBy >= 0) {
        if (packet->hitBy != packet->id) {
          retrorocket->players[packet->hitBy]->frags++;
        }
      }
      break;
    }
    case PACKET_HEATSEEKER: {
      HeatSeekerPacket *packet = (HeatSeekerPacket*)msg;
      retrorocket->manager->newParticleState (packet->id,
                                              packet->particleState);
      break;
    }
    case PACKET_BALL: {
      BallPacket *packet = (BallPacket*)msg;
      Ball *ball = retrorocket->balls[packet->id];
      ball->newBallState (packet->owner, packet->ballState);
      break;
    }
    case PACKET_COLLECT_BALL: {
      CollectBallPacket *packet = (CollectBallPacket*)msg;
      Ship *ship = retrorocket->ships[packet->shipID];
      if (!retrorocket->rtsMode) {
        ship->player->score += 
          retrorocket->gameFile->getInt ("ball_score") * 
          ship->getNumberOfChilds() *
          ship->getNumberOfChilds(); 
        ship->collectBalls();
        ship->resurrect();
      } else {
        ship->collectBalls();
      }
      break;
    }
    case PACKET_PICKUP_BALL: {
      PickUpBallPacket *packet = (PickUpBallPacket*)msg;
      Ship *ship = retrorocket->ships[packet->shipID];
      Ball *ball = retrorocket->balls[packet->ballID];
      Mob *lastChild = ship->getLastChild();

      lastChild->child = ball;
      ball->release (ship->id);
      break;
    }
    case PACKET_CREATE_BALL: {
      BallCreatePacket *packet = (BallCreatePacket*) msg;
      int index = packet->ballPosIndex;
      retrorocket->balls[0]->startX = retrorocket->ballpoints[index].x.toInt();;
      retrorocket->balls[0]->startY = retrorocket->ballpoints[index].y.toInt();;
      retrorocket->balls[0]->init();  
      break;
    }
    case PACKET_NEXT_LEVEL: {
      retrorocket->nextLevel();
      break;
    }
    case PACKET_CONF: {
      ConfPacket *packet = (ConfPacket*)msg;
      retrorocket->ships[packet->id]->player->gas = packet->gas;
      retrorocket->players[packet->id]->setWeaponA (packet->weaponA);
      retrorocket->players[packet->id]->setWeaponB (packet->weaponB);
      retrorocket->ships[packet->id]->roundsA = packet->roundsA;
      retrorocket->ships[packet->id]->roundsB = packet->roundsB;
      retrorocket->players[packet->id]->userMaxGas = packet->userMaxGas;
      retrorocket->players[packet->id]->userMaxWeaponA = packet->userMaxWeaponA;
      retrorocket->players[packet->id]->userMaxWeaponB = packet->userMaxWeaponB;
      break;
    }
    case PACKET_UPGRADE: {
      UpgradePacket *packet = (UpgradePacket*)msg;      
      retrorocket->players[packet->id]->shopUpgrade (packet->upgrade, true);
      break;
    }
    case PACKET_SOB_STRENGTH_UPDATE: {
      SobStrengthUpdatePacket *packet = (SobStrengthUpdatePacket*)msg;      
      retrorocket->sobs[packet->id]->updateStrength (packet->strength, false);
      break;
    }
    case PACKET_SOBCREATE: {
      SobCreatePacket *packet = (SobCreatePacket*)msg;      
      switch (packet->type) {
        case Sob::SENTRY: {
          SentryGun *s = new SentryGun (packet->x, packet->y, packet->angle,
                                        packet->owner,
                                        packet->sentryType);
          s->init();
          break;
        }
        case Sob::NODE: {
          ResourceNode *r = new ResourceNode (packet->x, packet->y,
                                              packet->owner);
          r->init();
          break;
        }
        case Sob::PLATFORM: {
          PlatformSOB *p = new PlatformSOB (packet->x, packet->y,
                                            packet->owner);
          p->init();
          Platform platform;
          platform.x1 = p->x;
          platform.y = p->y;
          platform.x2 = p->x + PlatformSOB::platformWidth();
          platform.properties = 0;
          platform.homeShip = p->owner;
          retrorocket->field->platforms.push_back (platform);
          break;
        }
        default: {
          panic("Creating illegal SOB");
          break;
        }
      }
      break;     
    }
    case PACKET_SOBDELETE: {
      SobDeletePacket *packet = (SobDeletePacket*)msg;
      retrorocket->sobs[packet->id]->killer = packet->killer;
      retrorocket->sobs[packet->id]->deleteSelf (false);
      break;
    }
    case PACKET_DELETEPARTICLE: {
      DeleteParticlePacket *packet = (DeleteParticlePacket*)msg;
      retrorocket->manager->deleteParticle(packet->id);
      break;
    }
    case PACKET_NEWGAME: {
      newGamePacket = *((NewGamePacket*)msg);
      newGamePacketReceived = true;
      retrorocket->gameMode = newGamePacket.gameMode;
      break;
    }
    case PACKET_STARTACK: {      
      StartAckPacket *packet = (StartAckPacket*)msg;
      strcpy (retrorocket->playerNames[packet->id], packet->playerName);
      startAckPacketsReceived++;
      break;
    }
    case PACKET_STARTGAME: {
      StartGamePacket *packet = (StartGamePacket*)msg;

      startGamePacketReceived = true;

      for (int i = 0; i < numberOfShips; i++) {
        strcpy (retrorocket->playerNames[i], packet->playerNames[i]);
      }

      break;
    }
    case PACKET_ENDGAME: {
      endGamePacketReceived = true;
      break;
    }
    case PACKET_ENDACK: {
      EndAckPacket *packet = (EndAckPacket*)msg; 
      endAckPacketsReceived++;
      endAckPackets[packet->id] = *packet;
      break;
    }
    case PACKET_ENDSTATE: {
      endStatePacket = *((EndStatePacket*)msg);
      endStatePacketReceived = true;
      break;
    }
    case PACKET_ABORTGAME: {
      retrorocket->abortGame();
      break;
    }
    case PACKET_PING: {
//       PingPacket *packet = (PingPacket *) msg;
//       PongPacket pongpacket;
//       pongpacket.pingvblcount = packet->vblcount;
//       pongpacket.pongvblcount = retrorocket->vblcount;
//       sendPacketToUser(packet->senderid, &pongpacket);
      break;
    }
    case PACKET_PONG: {
      PongPacket *packet = (PongPacket *) msg;
      retrorocket->vblcount = packet->pongvblcount +
        (retrorocket->vblcount - packet->pingvblcount) / 2;
      break;
    }
    default:
      panic ("Net protocol error");
  }
}

void Network::advance() {
  vector<Packet*> tmpQueue;

  if (!master) {
    if (((retrorocket->vblcount +
          (retrorocket->localShip << 6)) % 1000) == 0) {
      sendPingPacket();
    }
  }

  // copy to tmpQueue and empty packetQueue
  int h = startCritical();
  for (unsigned int i = 0; i < packetQueue.size(); i++) {
    tmpQueue.push_back (packetQueue[i]);
  }
  packetQueue.clear();
  endCritical (h);

  // process packets in tmpQueue
  for (unsigned int i = 0; i < tmpQueue.size(); i++) {
    handlePacket (tmpQueue[i]);
    free (tmpQueue[i]);
  }
}

void Network::sendPingPacket() {
  PingPacket ping;
  ping.vblcount = retrorocket->vblcount;
  ping.senderid = retrorocket->localShip;
  sendPacketToAll(&ping);
}

void Network::sendActionPacket (Ship *ship) {
  ActionPacket packet;
  packet.shipstate = ship->getState();
  packet.shipstate.actions &= ~(ACTION_BEAM|ACTION_MENU);
  packet.id = ship->id;
  if (packet.shipstate.actions & ACTION_SHOOT_B) {
    sendPacketToAll (&packet) ;
  } else {
    sendPacketBroadcast (&packet);  
  }
}

void Network::sendSentryActionPacket (int id, Fix32 angle) {
  SentryActionPacket packet;
  packet.id = id;
  packet.angle = angle;
  sendPacketToAll (&packet) ;
}

void Network::sendStrengthUpdatePacket (Ship *ship) {
  if (ship->strength <= 0 && ship->hitBy >= 0) {
    if (ship->hitBy != (int)ship->id) {
      retrorocket->players[ship->hitBy]->frags++;
    }
  }

  StrengthUpdatePacket packet;
  packet.strength = ship->strength;
  packet.id = ship->id;
  packet.hitBy = ship->hitBy;
  sendPacketToAll (&packet) ;
}

void Network::sendHeatSeekerPacket (Particle *particle) {
  HeatSeekerPacket packet;
  packet.id = particle->id;
  packet.particleState = particle->getState();
  sendPacketBroadcast (&packet);
}

void Network::sendBallPacket (Ball *ball) {
  BallPacket packet;
  packet.id = ball->id;
  packet.owner = ball->owner;
  packet.ballState = ball->getBallState();
  sendPacketBroadcast (&packet);
}

void Network::sendBallCreatePacket (int ballPosIndex) {
  BallCreatePacket packet;
  packet.ballPosIndex = ballPosIndex;
  sendPacketToAll (&packet);
}

void Network::sendConfPacket (Ship *ship) {
  ConfPacket packet;
  packet.id = ship->id;
  packet.gas = ship->player->gas;
  if (ship->weaponA) {
    packet.weaponA = ship->weaponA->type;
  } else {
    packet.weaponA = -1;
  }
  if (ship->weaponB) {
    packet.weaponB = ship->weaponB->type;
  } else {
    packet.weaponB = -1;
  }
  packet.roundsA = ship->roundsA;
  packet.roundsB = ship->roundsB;
  packet.userMaxGas = ship->player->userMaxGas;
  packet.userMaxWeaponA = ship->player->userMaxWeaponA;
  packet.userMaxWeaponB = ship->player->userMaxWeaponB;
  sendPacketToAll (&packet);
}

void Network::sendUpgradePacket (int id, Upgrades upgrade) {
  UpgradePacket packet;
  packet.id = id;
  packet.upgrade = upgrade;
  sendPacketToAll (&packet);
}

void Network::sendSobStrengthUpdatePacket (Sob *sob) {
  SobStrengthUpdatePacket packet;
  packet.id = sob->getID();
  packet.strength = sob->strength;
  sendPacketToAll (&packet);
}

void Network::sendSobCreatePacket(Sob *sob) {
  SobCreatePacket packet;
  packet.x = sob->x;
  packet.y = sob->y;
  packet.type = sob->sobType;
  packet.owner = sob->owner;
  if (sob->sobType == Sob::SENTRY) {
    packet.angle = ((SentryGun*)sob)->angleOffset;
    packet.sentryType = ((SentryGun*)sob)->sentryType;
  }
  sendPacketToAll (&packet);
}

void Network::sendSobDeletePacket(Sob *sob) {
  SobDeletePacket packet;
  packet.id = sob->getID();
  packet.killer = sob->killer;
  sendPacketToAll (&packet);
}

void Network::sendDeleteParticlePacket(int id) {
  DeleteParticlePacket packet;
  packet.id = id;
  sendPacketToAll (&packet) ;
}

void Network::sendCollectBallPacket (Ship *ship) {
  CollectBallPacket packet;
  packet.shipID = ship->id;
  sendPacketToAll (&packet);
}

void Network::sendPickUpBallPacket (Ship *ship, Ball *ball) {
  PickUpBallPacket packet;
  packet.shipID = ship->id;
  packet.ballID = ball->id;
  sendPacketToAll (&packet);
}

void Network::sendNextLevelPacket() {
  NextLevelPacket packet;
  sendPacketToAll (&packet);
}

void Network::waitForStart() {
  fprintf (stderr, "DB3");
  infoText ("Waiting...");
  if (master) {
    startAckPacketsReceived = 0;
    while (startAckPacketsReceived < numberOfShips-aiPlayers-1) {
      waitVBL();
      advance();
    }

    // copy own player name, other names come from start ack packets
    strcpy (retrorocket->playerNames[0], getUserName());
    // copy bot player names
    for (int i = 0; i < aiPlayers; i++) {
      char name[LINESIZE];
      sprintf (name, "Bot%d", i);
      strcpy (retrorocket->playerNames[numberOfShips-aiPlayers+i], name);
    }

    if (autogen) {
      for (unsigned int i = 0; i < retrorocket->sobs.size(); i++) {
        if (retrorocket->sobs[i]->sobType == Sob::PLATFORM)
          sendSobCreatePacket(retrorocket->sobs[i]);
      }
    }
    
    // distribute player names and start game
    StartGamePacket packet;
    for (int i = 0; i < numberOfShips; i++) {
      strcpy (packet.playerNames[i], retrorocket->playerNames[i]);
    }
    sendPacketToAll (&packet);

  } else {
    StartAckPacket packet;
    packet.id = retrorocket->localShip;
    strcpy(packet.playerName, getUserName());  
    sendPacketToMaster (&packet);
    startGamePacketReceived = false;
    while (!startGamePacketReceived) {
      waitVBL();
      advance();
    }

    sendPingPacket();
  }

  endGamePacketReceived = false;
}

Fix32 calculateNewRanking (int id, Fix32 currentRanking,
                           vector<int> *playerOrder) {

  bool better = true;

  // calculate ranking, based on ranking of other players
  for (unsigned int i = 0; i < retrorocket->players.size(); i++) {
    int otherID = (*playerOrder)[i];

    if (retrorocket->ships[id]->drawWith.find (otherID) ==
        retrorocket->ships[id]->drawWith.end()) {
      if (otherID == id) {
        better = false;
      } else {
        Fix32 rankingOther =
          retrorocket->network->endAckPackets[otherID].ranking;

        if (better) {
          currentRanking += ((Fix32)1 - currentRanking) * rankingOther
            / RANK_IMPACT_FACTOR;
        } else {
          currentRanking -= ((Fix32)1 - rankingOther) * currentRanking
            / RANK_IMPACT_FACTOR;
        }
      }
    }
  }
  
  return currentRanking;
}

void Network::endGame (int winner) {
  if (winner >= 0) retrorocket->playerDeathOrder.push_back (winner);

  endAckPacketsReceived = 0;

  EndGamePacket packet;
  sendPacketToAll (&packet);

  while (endAckPacketsReceived < numberOfShips-aiPlayers-1) {
    waitVBL();
    advance();
  }
  
  // master ack data
  endAckPackets[0].ranking = getCurrentRanking();
  endAckPackets[0].id = 0;
  endAckPackets[0].score = retrorocket->players[0]->score;
  endAckPackets[0].bestLapTime = retrorocket->players[0]->bestLapTime;
  endAckPackets[0].bestLapMade = retrorocket->ships[0]->bestLapMade;
  // bot ack data
  for (int i = 0; i < aiPlayers; i++) {
    int j = numberOfShips - aiPlayers;
    endAckPackets[j].ranking = DEFAULT_RANKING;
    endAckPackets[j].id = j;
    endAckPackets[j].score = retrorocket->players[j]->score;
    endAckPackets[j].bestLapTime = retrorocket->players[j]->bestLapTime;
    endAckPackets[j].bestLapMade = retrorocket->ships[j]->bestLapMade;
  }

  vector<int> *playerOrder;
  vector<int> playerScoreOrder;

  multimap<int,int> playerScoreMap;

  if ((retrorocket->gameMode == 0) || (retrorocket->gameMode == 3)) {

    playerOrder = &retrorocket->playerDeathOrder;

  } else {

    if (retrorocket->gameMode == 2) { // game is thrust, sort on score
      // create sorted map of <score,playerid>
      for (unsigned int i = 0; i < retrorocket->players.size(); i++) {
        playerScoreMap.insert (pair<int,int>
                               (retrorocket->network->endAckPackets[i].score,
                                retrorocket->network->endAckPackets[i].id));
      }
    } else { // sort on kills
      // create sorted map of <score,playerid>
      for (unsigned int i = 0; i < retrorocket->players.size(); i++) {
        playerScoreMap.insert (pair<int,int>
                               (retrorocket->players[i]->frags, i));
      }
    }

    { // iterate over all scores
      multimap<int,int>::iterator curItem = playerScoreMap.begin();
      while (curItem != playerScoreMap.end()) {
        int currentKey = curItem->first;

        // iterate over all equal scores
        set<int> drawWith;
        pair<multimap<int,int>::iterator,multimap<int,int>::iterator> p;
        p = playerScoreMap.equal_range (currentKey);
        multimap<int,int>::iterator i;
        for (i = p.first; i != p.second; i++) {
          drawWith.insert (i->second);
        }
      
        // iterate over all equal scores, to add drawWith vector to all ships
        // with this score
        for (i = p.first; i != p.second; i++) {
          set<int> *shipDrawWith = &retrorocket->ships[i->second]->drawWith;
          *shipDrawWith = drawWith;
          shipDrawWith->erase (shipDrawWith->find (i->second));
          playerScoreOrder.push_back (i->second);
        }

        // go to next unique score
        curItem = p.second;
      }
    }
    playerOrder = &playerScoreOrder;
  }

  // make end state packet based on playerOrder and drawWith
  for (unsigned int currentID=0;
       currentID < retrorocket->players.size(); currentID++) {
    endStatePacket.ranking[currentID] =
      calculateNewRanking (currentID, endAckPackets[currentID].ranking,
                           playerOrder);
    endStatePacket.score[currentID] = endAckPackets[currentID].score;
    endStatePacket.lapTimes[currentID] = endAckPackets[currentID].bestLapTime;
    endStatePacket.bestLapMade[currentID] =
      endAckPackets[currentID].bestLapMade;

    // calculate result positions
    int resultPosition = 0;
    int prevID = -1;
    for (int i = 0, j = (int)playerOrder->size() - 1; j >= 0; j--, i++) {
      int id = (*playerOrder)[j];
      Ship *ship = retrorocket->ships[id];

      if (ship->drawWith.find (prevID) == ship->drawWith.end()) {
        resultPosition = i;
      }

      if (id == (int)currentID) break;
      if (ship->drawWith.find (currentID) != ship->drawWith.end()) break;

      prevID = id;
    }
    endStatePacket.playerResultPositions[currentID] = resultPosition;
  }

  endStatePacket.winner = winner;
  sendPacketToAll (&endStatePacket);
}

void Network::sendAbortGamePacket() {
  AbortGamePacket packet;
  sendPacketToAll (&packet) ;
}

int Network::waitForEndStats() {
  endStatePacketReceived = false;

  EndAckPacket packet;
  packet.id = retrorocket->localShip;
  packet.ranking = getCurrentRanking();
  packet.score = retrorocket->players[retrorocket->localShip]->score;
  packet.bestLapTime =
    retrorocket->players[retrorocket->localShip]->bestLapTime;
  packet.bestLapMade = retrorocket->ships[retrorocket->localShip]->bestLapMade;
  sendPacketToMaster (&packet);

  while (!endStatePacketReceived) {
    waitVBL();
    advance();
  }
  return endStatePacket.winner;
}

void Network::sendNewGamePacket (int players, int aiPlayers, bool autogen,
                                 char *fieldFilename) {
  NewGamePacket packet;
  strcpy (packet.fieldFilename, fieldFilename);
  packet.worldgenWidth = retrorocket->worldgen.width;
  packet.worldgenHeight = retrorocket->worldgen.height;
  packet.worldgenSeaLevel = retrorocket->worldgen.seaLevel;
  packet.worldgenGraininess = retrorocket->worldgen.grainy;
  packet.randomSeed = retrorocket->fieldSeed;
  packet.autogen = autogen;
  packet.crc = retrorocket->myCRC;
  packet.players = players;
  packet.gameMode = retrorocket->gameMode;
  packet.aiPlayers = aiPlayers;
  numberOfShips = players + aiPlayers;
  sendPacketToAll (&packet) ;
  fprintf (stderr, "DB2");
}

bool Network::checkForNewGamePacket() {
  if (newGamePacketReceived) {
    strncpy (fieldFilename, newGamePacket.fieldFilename, FILENAMESIZE);
    autogen = newGamePacket.autogen;
    retrorocket->fieldSeed = newGamePacket.randomSeed;
    retrorocket->receivedCRC = newGamePacket.crc;
    aiPlayers = newGamePacket.aiPlayers;
    numberOfShips = newGamePacket.players + newGamePacket.aiPlayers;
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////

NoNetwork::NoNetwork() {
  numberOfShips = 1 + aiPlayers;
  autogen = false;
  master = true;
  retrorocket->localShip = 0;
}


/*
  lobby.cpp
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
// Lobby network object
// For communication with liblobby
//
///////////////////////////////////////////////////////////////////////////////

#include <MessageQueue.h>
#include <802.11.h>
#include <lobby.h>

#include <PA9.h>

#define TYPES_DEFINED

#include "retrorocket.h"

#define LOBBY_GAMECODE 5623

#define STREAM_ACK 0x0001
#define STREAM     0x8001

static int wirelessChannel = 2;

///////////////////////////////////////////////////////////////////////////////
// liblobby has a number of limitations and bugs
// the following represents a workaround to the limited room handling support

#define PACKET_LOBBY_WORKAROUND NUMBER_OF_PACKETS

/** Packet which triggers a reinitialisation of liblobby in all
    clients that receives it.
    Sent to all known clients when a user leaves a room or a room should
    be deleted because these things are not working in liblobby
 */
struct LobbyWorkAroundPacket : public Packet {
  LobbyWorkAroundPacket() { packettype = PACKET_LOBBY_WORKAROUND; }
  int size() { return sizeof (*this); }
};

static bool lobbyWorkaround = false;
static bool hasCreatedOrJoinedRooms;
static bool lobbyInited = false;

extern "C" { void LOBBY_Shutdown(); }

///////////////////////////////////////////////////////////////////////////////

/** represents a liblobby based network object */
class Lobby : public Network {

protected:
  LPLOBBY_ROOM activeRoom;

  bool initOK;

  void deleteRoomItems (Menu *menu);
  void updateRoomMenu (Menu *menu);
  void sendPacketToUser (int userid, Packet *packet);
  void sendPacketToAll (Packet *packet);
  void sendPacketToMaster (Packet *packet);
  void sendPacketBroadcast (Packet *packet);
  void displayUsersInRoom (LPLOBBY_ROOM room);
  static void startGame (Menu *menu);
  static void chooseRoom (Menu *menu);

public:
  Lobby();
  ~Lobby();
  bool init (bool isMaster, char *fieldFilename, int min, int max,
             int aiPlayers);
  static void userCallback (LPLOBBY_USER user, unsigned long reason);
  static void receivePacket(unsigned char *data, int length, LPLOBBY_USER from);
};

static Lobby *lobby;

///////////////////////////////////////////////////////////////////////////////

void setWirelessChannel (int i) {
  wirelessChannel = i;
  lobbyWorkaround = true;
}

void resetLobby() {
  if (lobbyWorkaround) {
    LOBBY_Shutdown();
    lobbyInited = false;
    lobbyWorkaround = false;
  }

  if (!lobbyInited) {
    LOBBY_Init();
    SetChannel (wirelessChannel);
    LOBBY_SetStreamHandler (STREAM_ACK, &Lobby::receivePacket);
    LOBBY_SetStreamHandler (STREAM, &Lobby::receivePacket);
    LOBBY_SetUserInfoCallback (&Lobby::userCallback);
    lobbyInited = true;
  }
}

void lobbyVBL() {
  IPC_RcvCompleteCheck();
  if (lobbyInited) {
    LOBBY_Update();
  }
}

void initNet() {
  lobby = NULL;
  if (!IPC_Init()) {
    panic ("Can't init IPC");
  }
  IPC_SetChannelCallback (0, &LWIFI_IPC_Callback) ;
  PA_VBLFunctionInit (lobbyVBL);
}

Network *openNetwork (bool isMaster, char *fieldFilename, int min, int max,
                      int aiPlayers, bool autogen) {
  lobby = new Lobby();
  lobby->autogen = autogen;

  if (lobby->init (isMaster, fieldFilename, min, max, aiPlayers)) {
    if (lobby->numberOfShips - aiPlayers <= 1) {
      NoNetwork *nn = new NoNetwork;
      nn->numberOfShips = lobby->numberOfShips;
      nn->aiPlayers = aiPlayers;
      delete lobby;
      return nn;
    }
  } else {
    delete lobby;
    return NULL;
  }
  return lobby;
}

///////////////////////////////////////////////////////////////////////////////

Lobby::Lobby() {
  resetLobby();
  activeRoom = NULL;
}

void sendLobbyWorkaround() {
  LobbyWorkAroundPacket packet;
  int users = LOBBY_GetNumberOfKnownUsers();
  vector<LPLOBBY_USER> userList;

  for (int i = 0; i < users; i++) {
    userList.push_back (LOBBY_GetUserByID (i));
  }

  for (int i = 0; i < (int)userList.size(); i++) {
    LOBBY_SendToUser (userList[i],
                      STREAM_ACK, (unsigned char *)(&packet), packet.size());
  }

  // wait long enough to make sure all packets are sent
  for (int i = 0; i < 20; i++) waitVBL();

  lobbyWorkaround = true;
}

Lobby::~Lobby() {
  if (hasCreatedOrJoinedRooms) {
    sendLobbyWorkaround();
  }

  if (lobbyWorkaround) {
    LOBBY_Shutdown();
    lobbyInited = false;
    lobbyWorkaround = false;
  }

  lobby = NULL;
}

void Lobby::userCallback (LPLOBBY_USER user, unsigned long reason) {
  switch (reason) {
    case USERINFO_REASON_REGOGNIZE:
      break;
    case USERINFO_REASON_TIMEOUT:
      if (!retrorocket->gameDone) {
        retrorocket->timeoutReceived = true;
        retrorocket->abortGame();
      }
      break;
    case USERINFO_REASON_RETURN:
      break;
    case USERINFO_REASON_ROOMCREATED:
      break;
    case USERINFO_REASON_ROOMCHANGE:
      break;
  }
}

void Lobby::receivePacket(unsigned char *data, int length, LPLOBBY_USER from) {
  Packet *packet = (Packet *)malloc (length);
  if (!packet) panic ("Out of memory");
  memcpy (packet, (const char*)data, length);

  if (packet->packettype == PACKET_LOBBY_WORKAROUND) {
    lobbyWorkaround = true;
  } else if (retrorocket) {
    if (lobby) {
      lobby->packetQueue.push_back (packet);
    }
  }
}

void Lobby::sendPacketToAll (Packet *packet) {
  LOBBY_SendToRoom (activeRoom, STREAM_ACK,
                    (unsigned char *)packet, packet->size()) ;
}

void Lobby::sendPacketToUser (int userid, Packet *packet) {
  LOBBY_SendToUser (LOBBY_GetRoomUserBySlot (activeRoom, userid), STREAM_ACK,
                    (unsigned char *)packet, packet->size()) ;
}


void Lobby::sendPacketToMaster (Packet *packet) {
  sendPacketToUser (0, packet);
}

void Lobby::sendPacketBroadcast (Packet *packet) {
  LOBBY_Broadcast (STREAM, (unsigned char *)packet, packet->size());
}

void Lobby::deleteRoomItems (Menu *menu) {
  // free items
  for (int i = 0; i < menu->getNumberOfItems(); i++) {
    free ((void*)menu->getItemText (i));
  }
  menu->clearItems();
}

void Lobby::chooseRoom (Menu *menu) {
  int roomID = menu->getChosenItem()->actionArg;
  LPLOBBY_ROOM chosenRoom = LOBBY_GetRoomByID (roomID);

  if (LOBBY_GetUsercountInRoom (chosenRoom) <
      LOBBY_GetMaxUsercountInRoom (chosenRoom)) {

    lobby->deleteRoomItems (menu);

    LOBBY_JoinRoom (chosenRoom);

    // only proceed if join succeeded
    while (!LOBBY_GetRoomByID (ROOMID_MYROOM)) waitVBL();

    lobby->activeRoom = chosenRoom;
    hasCreatedOrJoinedRooms = true;

    // display user list
    Menu userMenu ("Players", ROWS - 6);
    userMenu.setBack();
    userMenu.presentMenuAsync();

    bool done = false;

    // wait for new game packet
    while (!userMenu.isDone() && !done && !lobbyWorkaround) {
      userMenu.advance();
      lobby->advance();
      waitVBL();

      lobby->displayUsersInRoom (lobby->activeRoom);

      // exit if game start packed received
      done = lobby->checkForNewGamePacket();
    }

    if (!done) {
      sendLobbyWorkaround();
      resetLobby();
      menu->show();
    }

    lobby->initOK = done;
  }
}

void Lobby::startGame (Menu *menu) {
  // hide room
  // FIXME: does not work
//   LOBBY_SetRoomVisibility (0);

  lobby->sendNewGamePacket (LOBBY_GetUsercountInRoom (lobby->activeRoom),
                            lobby->aiPlayers, lobby->autogen,
                            lobby->fieldFilename);
  menu->remove();
  lobby->initOK = true;
}

void Lobby::updateRoomMenu (Menu *menu) {
  int rooms = LOBBY_GetNumberOfKnownRooms();

  deleteRoomItems (menu);

  // create items
  for (int i = 0; i < rooms; i++) {
    char *roomName = (char*)malloc (MENUITEMSIZE + 1);
    if (!roomName) panic ("Out of memory");
    LPLOBBY_ROOM room = LOBBY_GetRoomByID (i);

    // only add valid rooms
    if ((LOBBY_GetRoomGameCode (room) == LOBBY_GAMECODE) &&
        (LOBBY_GetRoomGameVersion (room) == NETPROTOCOL_VERSION)) {

      string mapName = LOBBY_GetRoomName (room);
      mapName = stripSuffix (mapName, ".game");
      snprintf (roomName, MENUITEMSIZE + 1, "%s (%d/%d) [%s]",
                mapName.c_str(),
                LOBBY_GetUsercountInRoom (room),
                LOBBY_GetMaxUsercountInRoom (room),
                LOBBY_GetUserName(LOBBY_GetRoomUserBySlot (room,0)));
    
      roomName[MENUITEMSIZE] = 0;
      menu->addItem (roomName, chooseRoom, i);
    }
  }
  menu->updateMenu();
}

void Lobby::displayUsersInRoom (LPLOBBY_ROOM room) {
  LPLOBBY_USER myself = LOBBY_GetUserByID (USERID_MYSELF);

  clearText (8, ROWS);
  outputText (2, 8, "Current: %d  Max: %d", 
             LOBBY_GetUsercountInRoom (room),
             LOBBY_GetMaxUsercountInRoom (room));

  for (int i = 0; i < LOBBY_GetUsercountInRoom (room); i++) {
    LPLOBBY_USER user = LOBBY_GetRoomUserBySlot (room, i);
    if (user == myself) {
      retrorocket->localShip = i;
    }
    setTextColor(i+PREDEFINED_TEXTCOLORS);
    outputText (2, 10 + i, LOBBY_GetUserName (user));
    setTextColor(WHITE);
  }
}

bool Lobby::init (bool isMaster, char *fieldFilename, int min, int max,
                  int aiPlayers) {
  hasCreatedOrJoinedRooms = false;

  this->aiPlayers = aiPlayers;

  this->fieldFilename = fieldFilename;
  newGamePacketReceived = false;
  master = isMaster;
  initOK = false;

  if (master) {

    // create room
    LOBBY_CreateRoom (fieldFilename, max - aiPlayers,
                      LOBBY_GAMECODE, NETPROTOCOL_VERSION);
    activeRoom = LOBBY_GetRoomByID (ROOMID_MYROOM);
    hasCreatedOrJoinedRooms = true;

    // create user list
    Menu userMenu ("Players", ROWS - 6);
    userMenu.setBack();
    userMenu.presentMenuAsync();
    bool okItem = false;
    while (!userMenu.isDone() && !lobbyWorkaround) {

      userMenu.advance();
      waitVBL();

      displayUsersInRoom (activeRoom);
      if (!okItem && LOBBY_GetUsercountInRoom (activeRoom) >= min) {
        okItem = true;
        userMenu.addItem ("Start game", startGame);
        userMenu.updateMenu();
      }
      if (okItem && LOBBY_GetUsercountInRoom (activeRoom) < min) {
        okItem = false;
        userMenu.clearItems();
        userMenu.updateMenu();
      }
    }

  } else {

    // create menu for selecting rooms
    Menu roomMenu ("Select room to join");
    roomMenu.setBack();
    roomMenu.presentMenuAsync();

    while (!roomMenu.isDone() && !lobby->newGamePacketReceived &&
           !lobbyWorkaround) {
      updateRoomMenu (&roomMenu);
      roomMenu.updateMenu();
      roomMenu.advance();
      waitVBL();
    }
    if (!initOK) retrorocket->mainMenu->show();
  }

  return initOK;
}

///////////////////////////////////////////////////////////////////////////////


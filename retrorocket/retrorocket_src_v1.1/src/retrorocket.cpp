/*
  retrorocket.cpp
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

RetroRocket *retrorocket;

#ifdef PROF
bool enableProfiler = false;

void saveProf (Menu *menu) {
  cygprofile_end();
  clearText();
  outputText (1, 1, "Data saved");
  while (true) waitVBL();
}
#endif

RetroRocket::RetroRocket() {
  retrorocket = NULL;

  vblcount = 0;

  config = new Config (MAIN_CONF);

  timeoutReceived = false;

  // init worldgen
  worldgen.width_tmp = 64;
  worldgen.height_tmp = 64;
  worldgen.grainy = 1024;
  worldgen.seaLevel = 180;
  worldgen.players = 2;
  worldgen.autogenBuffer = NULL;
  
  // create main menu
  mainMenu = new Menu ("Main Menu (" REV ")");
  mainMenu->setCallback (mainMenuCallback);

  // create autogen menu
  autogenMenu = new Menu ("Create New Field");
  autogenMenu->addItem ("Width     ", 32, 255, &worldgen.width_tmp);
  autogenMenu->addItem ("Height    ", 24, 255, &worldgen.height_tmp);
  autogenMenu->addItem ("Players   ",
                        1, config->countID ("color_player"),
                        &worldgen.players);
  autogenMenu->addItem ("Graininess", 2, 8192, &worldgen.grainy);
  autogenMenu->addItem ("Sea level ", 0, 254,
                        &worldgen.seaLevel, WorldGen::previewAutogen);
  autogenMenu->addItem ("Generate new", WorldGen::createNewAutogen);
  autogenMenu->addItem ("Play", WorldGen::placeAutogen);

  // create game menu
  gameMenu = new Menu ("Select Game", 3);

  // create game config menu
  gameModeList.push_back ("Survival");
  gameModeList.push_back ("Kills");
  gameModeList.push_back ("Thrust");
  gameModeList.push_back ("Strategy");
  
  userMinPlayers = 1;
  computerPlayers = 0;
  gameMode = 0;
  requiredKills = 5;

  gameConfigMenu = new Menu ("Setup Game Parameters");
  gameConfigMenu->setCallback (mainMenuCallback);
  gameConfigMenu->addItem ("Players   ", 1, 4, &userMinPlayers);
// AI player code not finished, removed from release
//   gameConfigMenu->addItem ("AI players    ", 0, 4, &computerPlayers);
  gameConfigMenu->addItem ("Game Mode ", &gameMode, &gameModeList);
  gameConfigMenu->addItem ("Kills     ", 0, 100, &requiredKills);
  gameConfigMenu->addItem ("Continue  ", gameMenu, setupGameMenu);

  mainMenu->addItem ("New game", gameConfigMenu);

  // create join menuitem
  mainMenu->addItem ("Join game", joinGame);

  // create config menu
  Menu *configMenu = new Menu ("Settings");

  musicTypeList.push_back ("off");
  musicTypeList.push_back ("main");
  musicTypeList.push_back ("random");

  soundSettingList.push_back ("off");
  soundSettingList.push_back ("some");
  soundSettingList.push_back ("all");

  boolList.push_back ("off");
  boolList.push_back ("on");

  configMenu->addItem ("Intro music    ", &musicSetting, &boolList,
                       musicAction);
  configMenu->addItem ("Game music     ", &gameMusic, &musicTypeList,
                       saveSettings);
  configMenu->addItem ("Sound effects  ", &soundSetting, &soundSettingList,
                       saveSettings);
  configMenu->addItem ("Net channel    ", 1, 13, &channel, changeChannel);

  // create control menu
  Menu *controlMenu = new Menu ("Customize Controls");
  controlMenu->addItem ("Rotate left", assignKey);
  controlMenu->addItem ("Rotate right", assignKey);
  controlMenu->addItem ("Thrust", assignKey);
  controlMenu->addItem ("Trigger A", assignKey);
  controlMenu->addItem ("Trigger B", assignKey);
  controlMenu->addItem ("Beam", assignKey);
  controlMenu->addItem ("Menu", assignKey);
  configMenu->addItem ("Controls", controlMenu);

  mainMenu->addItem ("Settings", configMenu);

#ifdef PROF
  mainMenu->addItem ("Save profiling data", saveProf);
#endif

  // init music
  loadMod (config->getString ("music_filename"),
           config->getInt ("music_game_pos"),
           config->getInt ("music_game_random_patterns"),
           config->getInt ("music_silent_pattern"));

  // init sounds
  thrustSoundSize = readBinary (&thrustSound,
                                config->getString ("thrust_sound"));
  shootASoundSize = readBinary (&shootASound,
                                config->getString ("shootA_sound"));
  shootBSoundSize = readBinary (&shootBSound,
                                config->getString ("shootB_sound"));
  deadSoundSize = readBinary (&deadSound,
                              config->getString ("dead_sound"));
  particleSoundSize = readBinary (&particleSound,
                                  config->getString ("particle_sound"));
  playSoundThrust (thrustSound, thrustSoundSize);
  trackingSoundSize = readBinary (&trackingSound,
                                  config->getString ("tracking_sound"));

  // init splash screen
  ConfigContext c;
  if (!config->findID ("splash", &c)) panic ("Parse error: splash");

  readBinary ((u8**)&splashInfo, config->getString (&c));
  splashTilesSize = readBinary ((u8**)&splashTiles,
                                config->getString (&c));
  splashMapSize = readBinary ((u8**)&splashMap, config->getString (&c));
  readBinary ((u8**)&splashPal, config->getString (&c));

  bot = false;

  // Create player palettes

  ConfigContext configContext;

  if (config->findID ("color_player", &configContext)) {
    do {
      unsigned short *pal =
        (unsigned short*)malloc ( (PLAYERPAL_GRADIENTS+3) *
                                  sizeof (unsigned short));
      if (!pal) panic ("Out of memory");
      pal[0] = 0;       // transparent
      int color = config->getHex (&configContext);
      int red =   (color & 0xFF0000) >> 19;
      int green = (color & 0x00FF00) >> 11;
      int blue =  (color & 0x0000FF) >> 3;
      for ( int i = 0; i < PLAYERPAL_GRADIENTS; i++) {
        pal[i+1] = (1 << 15) |
          ( (blue  + (31 - blue)  * i / PLAYERPAL_GRADIENTS) << 10) |
          ( (green + (31 - green) * i / PLAYERPAL_GRADIENTS) <<  5) |
          (  red   + (31 - red)   * i / PLAYERPAL_GRADIENTS);
      }
      pal[PLAYERPAL_GRADIENTS+1] = 1 << 15; // black
      pal[PLAYERPAL_GRADIENTS+2] = 0xffff;  // white
      playerPals.push_back (pal);
    } while (config->next (&configContext));
  } else {
    panic ("Missing color player def");
  } 
}

void RetroRocket::setupGameMenu (Menu *menu) {
  retrorocket->gameMenu->clearItems();
  string suffix = ".";
  suffix.append (GAMEEXT);
  retrorocket->gameMenu->setSuffixToStrip (suffix);
  retrorocket->gameMenu->setCallback (Field::printFieldInfo);

  if (retrorocket->gameMode == 0 || retrorocket->gameMode == 1) {
    retrorocket->gameMenu->addItem ("-Generate new-", retrorocket->autogenMenu);
  }

  vector<char*>filenames;
  getFileList (GAMEEXT, &filenames);
  if (filenames.size() == 0) panic ("No games in datadir");
  for (int i = 0; i < (int)filenames.size(); i++) {
    if (Field::fieldIsOfGivenType (filenames[i], retrorocket->gameMode,
                                   retrorocket->userMinPlayers,
                                   retrorocket->computerPlayers)) {
      retrorocket->gameMenu->addItem (filenames[i], retrorocket->newGame);
    }
  }
}

void RetroRocket::start() {
  network = NULL;
  
  { // read settings
    Config c;
    if (c.load (LOCAL_CONFIG_FILE)) {
      // settings from local.conf
      localControls.left = c.getInt ("left");
      localControls.right = c.getInt ("right");
      localControls.thrust = c.getInt ("thrust");
      localControls.triggerA = c.getInt ("triggera");
      localControls.triggerB = c.getInt ("triggerb");
      localControls.menu = c.getInt ("menu");
      localControls.beam = c.getInt ("beam");
      musicSetting = c.getInt ("music");
      gameMusic = c.getInt ("gamemusic");
      soundSetting = c.getInt ("sound");
      channel = c.getInt ("channel");
    } else {
      // default settings
      localControls.left = config->getInt ("left");
      localControls.right = config->getInt ("right");
      localControls.thrust = config->getInt ("thrust");
      localControls.triggerA = config->getInt ("triggera");
      localControls.triggerB = config->getInt ("triggerb");
      localControls.menu = config->getInt ("menu");
      localControls.beam = config->getInt ("beam");
      musicSetting = config->getInt ("music");
      gameMusic = config->getInt ("gamemusic");
      soundSetting = config->getInt ("sound");
      channel = config->getInt ("channel");
    }
  }

  updatefrequency = config->getInt("updatefrequency");

  setWirelessChannel (channel);

  initIntroGfx();
  loadBg (splashInfo, splashMap, splashMapSize, splashTiles, splashTilesSize,
          splashPal, FRONT_BG);
  showSplash();
  oldMusicSetting = 0;
  musicAction (NULL);
  gameDone = true;
  mainMenu->presentMenuAsync();
}

RetroRocket::~RetroRocket() {
  free (splashPal);
  free (splashMap);
  free (splashTiles);
  free (splashInfo);
  freeMod();
  // TODO: this starts at 1 because of autogen item, which is error prone
  for (int i = 1; i < gameMenu->getNumberOfItems(); i++) {
    free ((void*)gameMenu->getItemText (i));
  }
  Menu::freeAllMenus (mainMenu);
  delete config;
}

void RetroRocket::showSplash() {
  showBG (FRONT_BG);
}  

void RetroRocket::hideSplash() {
  hideBG (FRONT_BG);
}

void RetroRocket::mainMenuCallback (Menu *menu) {
  retrorocket->showSplash();
}

void RetroRocket::changeChannel (Menu *menu) {
  setWirelessChannel (retrorocket->channel);
  saveSettings (NULL);
}

void RetroRocket::musicAction (Menu *menu) {
  if ((retrorocket->oldMusicSetting == 0) && (retrorocket->musicSetting == 1)) {
    playMod (retrorocket->config->getInt ("music_intro_pos"));
  } else if ((retrorocket->oldMusicSetting == 1) &&
             (retrorocket->musicSetting == 0)) {
    stopMod();
  }
  retrorocket->oldMusicSetting = retrorocket->musicSetting;
  saveSettings (NULL);
}

void RetroRocket::advance() {
#ifdef PROF
  if (sysGetInput() & Y) {
    enableProfiler = true;
    cygprofile_enable();
  }
  if (sysGetInput() & X) {
    enableProfiler = false;
    cygprofile_disable();
  }
#endif
  vblcount++;
  mainMenu->advance();
  if (timeoutReceived) {
    endGame();
    cleanupGame();
    timeoutReceived = false;
  }
  if (!gameDone) {
    // Plot MOBs (should be right after VBL to avoid flickering)
    for (unsigned int i = 0; i < mobs.size(); i++) mobs[i]->plot();
    advanceGame();
    if (gameDone) {
      endGame();
      cleanupGame();
      timeoutReceived = false;
    }
  }
}

void RetroRocket::nextLevel() {
  cleanupLevel();
  delete manager;
  initIntroGfx();
  infoText ("Loading...");
  initGameGfx();
  field = new Field (levelFilenames[currentLevel]);
  initLevel();
}

void RetroRocket::initLevel() {
  collectedBalls = 0;

  clearText();
  infoText ("Loading...");

  // load ships
  for (int i = 0; i < network->numberOfShips; i++) {
    new Ship (players[i]);
  }

  // create particle manager
  manager = new ParticleManager();
    
  // show field
  clearText();
  showBG (FRONT_BG);
  showBG (BACK_BG);
  showBG (SOB_BG);
  showMap();

  // init field/system
  //waitVBL();
  // init mobs
  for (unsigned int i = 0; i < mobs.size(); i++) mobs[i]->init();
  // init sobs
  for (unsigned int s = 0; s < sobs.size(); s++) sobs[s]->init();
  // show map
  showMap();

  // set focus sprite
  setFocus (ships[localShip]);
}

void RetroRocket::cleanupLevel() {
  setFocus (NULL);
  delete field;
  for (unsigned int i = 0; i < mobs.size(); i++) delete mobs[i];
  ballpoints.clear();
  balls.clear();
  mobs.clear();
  ships.clear();
  for (unsigned int i = 0; i < sobs.size(); i++) delete sobs[i];
  sobs.clear();
  for (unsigned int i = 0; i < mobs.size(); i++) {
    deleteSprite (mobs[i]->spritenum);
  }
}

void RetroRocket::quitGame (Menu *menu) {
  retrorocket->network->sendAbortGamePacket();
  retrorocket->abortGame();
}

void RetroRocket::abortGame() {
  gameDone = true;
  winner = -2;
}

void RetroRocket::advanceGame() {

  // Print stats to screen
  Ship *ls = ships[watchedShip];
  setTextColor (WHITE);
  clearText (ROWS-3, ROWS-1);

  int x1 = 1;
  
#ifdef PROF
  outputText (0, 21, "%c", enableProfiler ? '*' : ' ');
#endif

  if (!rtsMode && (gameMode != 1)) {
    outputText (x1, 21, "L: %d", ls->player->lives);
    x1 += 7;
  }

  if (gameMode == 1) {
    outputText (x1, 21, "K: %d", ls->player->frags);
    x1 += 7;
  }

  outputText (x1, 21, "S: %d", ls->strength);
  x1 += 7;

  if (rtsMode) {
    outputText (x1, 21, "$: %d", ls->player->credits.toUInt());
    x1 += 8;
  }
  if (balls.size()) {
    outputText (x1, 21, "Sc: %d", ls->player->score);
    x1 += 8;
  }

  int x2 = 1;

  outputText (x2, 22, "A: %d", ls->roundsA);
  x2 += 7;

  outputText (x2, 22, "B: %d", ls->roundsB);
  x2 += 7;

  outputText (x2, 22, "G: %d", ls->player->gas.toUInt());
  x2 += 8;

  int ms = vblToCSec (ls->lapTime);

  if (retrorocket->field->hasLaps()) {
    int x = x1 > x2 ? x1 : x2;

    outputText (x, 21, "t: %2d:%02d", (int)(ms/100), ms%100);  
    if (ls->bestLapMade) {
      int ms = vblToCSec (ls->player->bestLapTime);
      outputText (x, 22, "T: %2d:%02d", (int)(ms/100), ms%100);
    }
  }

  // update network (handle incoming packets)
  network->advance();

  // check ship-ship collisions
  if (retrorocket->network->master) {
    unsigned int n = retrorocket->ships.size();
    bool collision = false;
    for (unsigned int i = 0; i < n; i++) {
      Ship *shipA = retrorocket->ships[i];
      for (unsigned int j = 0; j < n; j++) {
        if (i != j) {
          Ship *shipB = retrorocket->ships[j];
          if (!shipA->player->dead &&
              !shipB->player->dead &&
              shipA->collidesWith (shipB)) {
            shipA->strength = 0;
            shipB->strength = 0;
            collision = true;
            if ((shipA->player->lives <= 1) && (shipB->player->lives <= 1)) {
              shipA->drawWith.insert (shipB->id);
              shipB->drawWith.insert (shipA->id);
            }
            shipA->hitBy = -1;
            shipB->hitBy = -1;
          }
        }
      }
    }
    if (collision) {
      for (unsigned int i = 0; i < n; i++) {
        Ship *ship = retrorocket->ships[i];
        if ((ship->strength <= 0) && (!ship->player->dead)) {
          retrorocket->network->sendStrengthUpdatePacket (ship);
        }
      }
    }
  }

  // advance mobs
  for (unsigned int i = 0; i < mobs.size(); i++) mobs[i]->advance();
  // advance sobs
  for (unsigned int i = 0; i < sobs.size(); i++) sobs[i]->advance();
  // advance players
  for (unsigned int i = 0; i < players.size(); i++) players[i]->advance();

  // collect ball
  if (network->master && balls.size()) {
    int requiredBalls;
    if (balls.size() > 1) {
      requiredBalls = required_balls;
    } else {
      requiredBalls = balls.size();
    }
    bool changeLevel = false;
    for (unsigned int i = 0; i < ships.size(); i++) {
      if (ships[i]->getNumberOfChilds()) { // has at least one ball
        if (ships[i]->y < outer_space) {
          collectedBalls += ships[i]->getNumberOfChilds();
          ships[i]->player->score +=
            gameFile->getInt ("ball_score") * 
            ships[i]->getNumberOfChilds() *
            ships[i]->getNumberOfChilds(); 
          if (collectedBalls >= requiredBalls) {
            changeLevel = true;
            currentLevel++;
            // TODO: level score could depend on time?
            ships[i]->player->score += field->score;
          } else {
            ships[i]->collectBalls();
            ships[i]->resurrect();
          }
          network->sendCollectBallPacket (ships[i]);
        }
      }
    }
    if (changeLevel) {
      // TODO: what about laptime?

      if (currentLevel >= (int)levelFilenames.size()) {
        winner = 0;
        bool draw = true;
        for (unsigned int s = 1; s < ships.size(); s++) {
          if (ships[s]->player->score != ships[winner]->player->score) {
            draw = false;
          }
          if (ships[s]->player->score > ships[winner]->player->score) {
            winner = s;
          }
        }
        if (draw) winner = -1;

        gameDone = true;
        network->endGame (winner);
        return;
      }

      network->sendNextLevelPacket();
      nextLevel();
    }
  }

  // Game over
  if (network->master) {
    if (finish) {
      finishCountdown--;
      if (finishCountdown <= 0) {
        // find if there are survivors
        for (unsigned int i = 0; i < ships.size(); i++) {
          if (ships[i]->player->lives > 0) {
            winner = i;
            break;
          }
        }
        gameDone = true;
        network->endGame (winner);
        return;
      }
    } else {
      if (gameMode == 1) {
        // end if someone has enough kills
        for (unsigned int i = 0; i < ships.size(); i++) {
          if (ships[i]->player->frags >= requiredKills) {
            finish = true;
            finishCountdown = gameFile->getInt ("end_time");
          }
        }
      } else {
        // see if survivors
        // end if <= 1

        int playersAlive = 0;
        for (unsigned int i = 0; i < ships.size(); i++) {
          if (ships[i]->player->lives > 0) {
            playersAlive++;
          }
        }

        finish = ((playersAlive==0) || (playersAlive==1 && ships.size()>1));
        if (finish) finishCountdown = gameFile->getInt ("end_time");
      }
    }
  } else {
    if (network->endGamePacketReceived) {
      winner = network->waitForEndStats();
      gameDone = true;
    }
  }
}

// play one round of TR
void RetroRocket::playGame() {
  showGameIntroText();

  network->waitForStart();
  watchedShip = localShip;

  gameDone = false;
  winner = -1;

  mainMenu->hide();

  // create players
  if (network->master) {
    for (int i = 0; i < network->numberOfShips - network->aiPlayers; i++) {
      if (i == (int)localShip) {
        players.push_back (new LocalPlayer (i));
      } else {
        players.push_back (new RemotePlayer (i));
      }
    }
    for (int i = 0; i < network->aiPlayers; i++) {
      players.push_back (new Bot (network->numberOfShips -
                                  network->aiPlayers + i));
    }
  } else {
    for (int i = 0; i < network->numberOfShips; i++) {
      if (i == (int)localShip) {
        players.push_back (new LocalPlayer (i));
      } else {
        players.push_back (new RemotePlayer (i));
      }
    }
  }

  initLevel();

  finishCountdown = 0;
  finish = false;

  if (retrorocket->gameMusic == 2) {
    playRandomMod();
  } else if ((retrorocket->gameMusic == 1) &&
             (retrorocket->musicSetting == 0)) {
    playMod (config->getInt ("music_intro_pos"));
  } else if (retrorocket->gameMusic == 0) {
    stopMod();
  }
}

void RetroRocket::endGame() {
  modifySoundThrust(0,0);
  if (retrorocket->gameMusic == 2) {
    playMod (config->getInt ("music_game_over_pos"));
  }

  clearText();
  hideMap();

  // remove any open menus
  ((LocalPlayer*)players[localShip])->menu->remove();

  if (winner == -2) {
    delete network;
    network = NULL;

    message ("Game aborted");
  } else {
    // test for highscore
    Config c;
    c.load (HIGHSCOREFILE);
    Fix32 oldHighscore = 0;
    int oldBestLap = INT_MAX;

    // read from highscore file
    ConfigContext configContext;
    if (c.findID (fieldFilename, &configContext)) {
      oldHighscore = c.getFloat (&configContext);
      if (field->hasLaps()) {
        int lt = c.getInt (&configContext);
        if (lt != -1) oldBestLap = lt;
      }
    }
    
    if (field->hasLaps()) {
      // print if best lap is made
      if (ships[localShip]->bestLapMade) {
        if (ships[localShip]->player->bestLapTime < oldBestLap) {
          int ms = vblToCSec (ships[localShip]->player->bestLapTime);
          outputText (1, ROWS-1, "New best lap: %d:%02d",
                      (int)(ms/100), ms%100);
          oldBestLap = ships[localShip]->player->bestLapTime;
        }
      } else if (oldBestLap == INT_MAX) {
        oldBestLap = -1; // flag that no lap has yet been made
      }
    }

    if (retrorocket->maxNumberPlayers == 1) {
      // single player, print highscore
      if (ships[localShip]->player->score > oldHighscore.toInt()) {
        outputText (1, ROWS-2, "New highscore: %d",
                    ships[localShip]->player->score);
        oldHighscore = ships[localShip]->player->score;
      }

      delete network;
      network = NULL;

    } else if (ships.size() > 1) {
      // multi player, print ranking
      outputText (1, 4, "Pos Player    Rank");
      if (balls.size()) {
        outputText (20, 4, "Score");
      } else {
        outputText (20, 4, "Kills");
      }
      if (field->hasLaps()) outputText (26, 4, "Lap");

      // make sorted list
      multimap<int,int> positionMap;
      for (unsigned int i=0; i < players.size(); i++) {
        int pos = network->endStatePacket.playerResultPositions[i];
        positionMap.insert (pair<int,int> (pos,i));
      }
            
      multimap<int,int>::iterator iter = positionMap.begin();
      for (int lineNum = 6; iter != positionMap.end(); iter++, lineNum++) {
        // print stats
        int shipID = iter->second;
        setTextColor(shipID+PREDEFINED_TEXTCOLORS);
        outputText (1, lineNum, "%d",
                    network->endStatePacket.playerResultPositions[shipID]+1);
        outputText (5, lineNum, "%s", playerNames[shipID]);
        outputText (15, lineNum, "%d",
                    (network->endStatePacket.ranking[shipID]*1000).
                    toIntRound());
        if (balls.size()) {
          outputText (20, lineNum, "%d",
                      network->endStatePacket.score[shipID]);
        } else {
          outputText (20, lineNum, "%d", players[shipID]->frags);
        }
        if (field->hasLaps()) {
          if (network->endStatePacket.bestLapMade[shipID]) {
            int ms = vblToCSec (network->endStatePacket.lapTimes[shipID]);
            outputText (26, lineNum, "%d:%02d", (int)(ms/100), ms%100);
          } else {
            outputText (26, lineNum, "--:--",
                        network->endStatePacket.lapTimes[shipID]);
          }
        }
      }
      oldHighscore = network->endStatePacket.ranking[retrorocket->localShip];
    } else { // multi player game, played by single player
      if (oldHighscore == 0) oldHighscore = DEFAULT_RANKING;
    }

    setTextColor(WHITE);
    
    // write to highscore file
    c.deleteID (fieldFilename);
    c.newID (fieldFilename, &configContext);
    c.addFloat (oldHighscore.toFloat(), &configContext);
    if (field->hasLaps()) {
      c.addInt (oldBestLap, &configContext);
    }
    c.save (HIGHSCOREFILE);

    delete network;
    network = NULL;

    if (ships.size() == 1) {
      message ("Game over");
    } else {
      message (1, "Results");
    }
  }

  delete manager;

  if ((retrorocket->gameMusic != 1) && (retrorocket->musicSetting == 1)) {
    playMod (config->getInt ("music_intro_pos"));
  } else if ((retrorocket->gameMusic != 0) && (retrorocket->musicSetting == 0)) stopMod();
}

void RetroRocket::cleanupGame() {
  cleanupLevel();

  for (unsigned int i = 0; i < mobs.size(); i++) delete players[i];
  players.clear();

  for (unsigned int i = 0; i < levelFilenames.size(); i++) {
    free (levelFilenames[i]);
  }
  levelFilenames.clear();
  delete gameFile;
  initIntroGfx();
  loadBg (splashInfo, splashMap, splashMapSize, splashTiles, splashTilesSize,
          splashPal, FRONT_BG);

  mainMenu->show();
}

void RetroRocket::loadGame (char *filename, bool autogen) {
  clearText();
  infoText ("Loading...");

  gameFile = new Config (filename);

  if (gameFile->exists ("resource")) {
    rtsMode = true;
  } else {
    rtsMode = false;
  }

  currentLevel = 0;
  myCRC = 0;

  ConfigContext configContext;

  initGameGfx();

  if (autogen) {
    field = new Field (gameFile, &worldgen);
    minNumberPlayers = worldgen.players;
    maxNumberPlayers = worldgen.players;
  } else {
    minNumberPlayers = gameFile->getInt ("minplayers");
    maxNumberPlayers = gameFile->getInt ("maxplayers");
    if (gameFile->findID ("level", &configContext)) {
      do {
        const char *fn = gameFile->getString (&configContext);
        char *copiedFilename = (char*)malloc (strlen (fn) + 1);
        if (!copiedFilename) panic ("Out of memory");
        strcpy (copiedFilename, fn);
        levelFilenames.push_back (copiedFilename);
      } while (gameFile->next (&configContext));
      field = new Field (levelFilenames[0]);
    } else {
      field = new Field (gameFile);
    }
  }

  // make bitmask of all weapons that are available for this game
  allowedWeaponA = 0;
  allowedWeaponB = 0;
  // TODO: check against Weapon::name() instead
  if (gameFile->exists ("streamgun"))  allowedWeaponA |= WEAPONA_STREAM;
  if (gameFile->exists ("spraygun"))   allowedWeaponA |= WEAPONA_SPRAY;
  if (gameFile->exists ("doublegun"))  allowedWeaponA |= WEAPONA_DOUBLE;
  if (gameFile->exists ("tripplegun")) allowedWeaponA |= WEAPONA_TRIPPLE;
  if (gameFile->exists ("single"))     allowedWeaponB |= WEAPONB_SINGLE;
  if (gameFile->exists ("buck"))       allowedWeaponB |= WEAPONB_BUCK;
  if (gameFile->exists ("seeker"))     allowedWeaponB |= WEAPONB_SEEKER;
  if (gameFile->exists ("mine"))       allowedWeaponB |= WEAPONB_MINE;
  if (gameFile->exists ("sentrygun"))  allowedWeaponB |= WEAPONB_SENTRYGUN;
  if (gameFile->exists ("heatsentrygun"))
    allowedWeaponB |= WEAPONB_HEATSENTRYGUN;
  if (gameFile->exists ("resource"))   allowedWeaponB |= WEAPONB_RESOURCE_NODE;
  if (gameFile->exists ("mono"))       allowedWeaponB |= WEAPONB_THRUSTSINGLE;
  if (gameFile->exists ("bomb"))       allowedWeaponB |= WEAPONB_BOMB;
  if (gameFile->exists ("asmissile"))  allowedWeaponB |= WEAPONB_ASM;

  if (rtsMode) {
    WeaponA::ammoPrice = retrorocket->gameFile->getInt ("weapon_a_ammo_price");
    WeaponA::quanta = retrorocket->gameFile->getInt ("weapon_a_ammo_quantum");
  }


  if (balls.size()) {
    required_balls = gameFile->getInt ("required_balls");
    outer_space = gameFile->getInt ("outer_space");
  }

  // clear player death order
  playerDeathOrder.clear();

  // Calculate CRC (binary CRC updated by readBinary(util.h))
  myCRC += config->getCRC();
  myCRC += gameFile->getCRC();
}

void RetroRocket::showGameIntroText() {
  ConfigContext configContext;
  if (gameFile->findID ("introtext", &configContext)) {
    clearText();

    int y = 1;
    do {
      int x = 1;
      while (gameFile->moreEls (&configContext)) {
        const char *str = gameFile->getString (&configContext);
        outputText (x, y, str);
        x += strlen (str) + 1;
      }
      y++;
    } while (gameFile->next (&configContext));

    setTextColor (BLUE);
    outputText (1, ROWS - 2, "(Press A to continue)");

    waitKey (A);
    clearText();
  }
}

void RetroRocket::assignKey (Menu *menu) {
  retrorocket->mainMenu->hide();

  clearText();
  setTextColor (WHITE);
  infoText ("Press key for %s...", menu->getCurrentItemText());

  // wait for key release
  while (sysGetInput()) waitVBL(); 

  // get new key
  int newKey = 0;
  do {
    waitVBL();
  } while (!(newKey = msb (sysGetInput())));

  // register new key
  switch (menu->getChoice()) {
    case 0: retrorocket->localControls.left     = newKey; break;
    case 1: retrorocket->localControls.right    = newKey; break;
    case 2: retrorocket->localControls.thrust   = newKey; break;
    case 3: retrorocket->localControls.triggerA = newKey; break;
    case 4: retrorocket->localControls.triggerB = newKey; break;
    case 5: retrorocket->localControls.beam     = newKey; break;
    case 6: retrorocket->localControls.menu     = newKey; break;
  }

  // wait for key release
  while (sysGetInput()) waitVBL(); 

  saveSettings (NULL);

  retrorocket->mainMenu->show();
}

void RetroRocket::saveSettings (Menu *menu) {
  Config c;
  c.putInt ("left",            retrorocket->localControls.left);
  c.putInt ("right",           retrorocket->localControls.right);  
  c.putInt ("thrust",          retrorocket->localControls.thrust); 
  c.putInt ("triggera",        retrorocket->localControls.triggerA);
  c.putInt ("triggerb",        retrorocket->localControls.triggerB);
  c.putInt ("menu",            retrorocket->localControls.menu);
  c.putInt ("beam",            retrorocket->localControls.beam);  
  c.putInt ("music",           retrorocket->musicSetting);
  c.putInt ("gamemusic",       retrorocket->gameMusic);
  c.putInt ("sound",           retrorocket->soundSetting);
  c.putInt ("channel",         retrorocket->channel);
  c.save (LOCAL_CONFIG_FILE);
}

void RetroRocket::playAutogen (Menu *menu) {
  strcpy (retrorocket->fieldFilename, AUTOGEN_GAMEFILE);
  retrorocket->loadGame (retrorocket->fieldFilename, true);

  free (retrorocket->worldgen.autogenBuffer);
  retrorocket->worldgen.autogenBuffer = NULL;

  retrorocket->playMaster (true);
}

void RetroRocket::newGame (Menu *menu) {
  retrorocket->fieldSeed = rand();

  strcpy (retrorocket->fieldFilename, menu->getItemText (menu->getChoice()));
  retrorocket->loadGame (retrorocket->fieldFilename);
  retrorocket->playMaster();
}

void RetroRocket::playMaster (bool autogen) {
  if (maxNumberPlayers > 1) {
    // host multiplayer game (dstods)
    network = openNetworkMaster (fieldFilename,
                                 minNumberPlayers, maxNumberPlayers,
                                 computerPlayers, autogen);
    if (!network) {
      cleanupGame();
      return;
    }

  } else {
    network = new NoNetwork;
  }

  playGame();
}

void RetroRocket::joinGame (Menu *menu) {
  retrorocket->computerPlayers = 0;

  retrorocket->network = openNetworkSlave (retrorocket->fieldFilename);

  if (!retrorocket->network) {
    return;
  }

  retrorocket->worldgen.width =
    retrorocket->network->newGamePacket.worldgenWidth;
  retrorocket->worldgen.height =
    retrorocket->network->newGamePacket.worldgenHeight;
  retrorocket->worldgen.seaLevel =
    retrorocket->network->newGamePacket.worldgenSeaLevel;
  retrorocket->worldgen.grainy =
    retrorocket->network->newGamePacket.worldgenGraininess;
  retrorocket->worldgen.players = retrorocket->network->numberOfShips;
  retrorocket->worldgen.platforms.clear();

  retrorocket->loadGame (retrorocket->fieldFilename,
                         retrorocket->network->autogen);

  if (retrorocket->myCRC != retrorocket->receivedCRC) {
    panic("CRC mismatch");
  }

  retrorocket->playGame();
}


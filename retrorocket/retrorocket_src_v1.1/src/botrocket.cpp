#include "botrocket.h"
// #include "bot.h"
/*******

	BotRocket a RR Wrapper to 
	make it easier to debug 

*******/


void BotRocket::start()
{
//   if(client){
//     strcpy (retrorocket->fieldFilename, "TR_Tropulus.game");
//     strcpy (fieldFilename, "TR_Tropulus.game");
//     joinGame(NULL);
//   }else{
    strcpy (fieldFilename, "TR_Tropulus.game");
    newBotGame();
//   }
  
}
// void BotRocket::joinGame (Menu *menu)
// {
// }
void BotRocket::playMaster (bool autogen) {
  if (maxNumberPlayers > 1) {
    // host multiplayer game (dstods)
    network = openNetworkMaster (fieldFilename,
                                 1, 2,
                                 2, autogen);
    if (!network) {
      cleanupGame();
      return;
    }

  } else {
    network = new NoNetwork;
  }

  playGame();
}
void BotRocket::newBotGame ()
{
  fieldSeed = rand();

  loadGame ("TR_Tropulus.game");
  playMaster();
}
void BotRocket::playGame() {
  showGameIntroText();
  network->waitForStart();

  gameDone = false;
  winner = -1;

  mainMenu->hide();

  // create players

  players.push_back (new Bot(0));
  ((Bot*)players.at(0))->setCheckAhead(checkahead);
  ((Bot*)players.at(0))->setMaxSteps(maxsteps);
  players.push_back (new Bot(1));
  ((Bot*)players.at(1))->setCheckAhead(checkahead);
  ((Bot*)players.at(1))->setMaxSteps(maxsteps);

//   for (int i = 0; i < network->numberOfShips; i++) {
//     if (i == (int)localShip) {
//       players.push_back (new Bot(i));
//     } else {
//       players.push_back (new RemotePlayer (i));
//     }
//   }

  initLevel();

  finishCountdown = 0;
  finish = false;

  if (retrorocket->gameMusic == 2) {
    playRandomMod();
  } else if ((retrorocket->gameMusic == 1) && (retrorocket->musicSetting == 0)) {
    playMod (config->getInt ("music_intro_pos"));
  } else if (retrorocket->gameMusic == 0) {
    stopMod();
  }
}

// /*

//   c1: start->newgame endre til fastfilnavn
//   c2: start->joinGame (kall med null)
//   void RetroRocket::newGame (Menu *menu) {
//   retrorocket->fieldSeed = rand();

//   //strcpy (retrorocket->fieldFilename, menu->getItemText (menu->getChoice()));
//   minNumberPlayers = 2;
//   maxNumberPlayers = 2;
//   retrorocket->loadGame (retrorocket->fieldFilename);
//   retrorocket->playMaster();
//   }
// */

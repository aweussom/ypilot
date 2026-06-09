/*
  player.cpp
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

Player::Player (unsigned int id) {
  this->id = id; 
  frags = 0;

  memset (&upgradeLevels, 0, sizeof (UpgradeLevels));

  if (retrorocket->allowedWeaponA) {
    autoFireRate = retrorocket->gameFile->getInt ("autofire_rate");
  }
  if (retrorocket->allowedWeaponB) {
    repetitionRate =
      retrorocket->gameFile->getInt ("weapon_b_max_repetition_rate");
  }
  damageScaler = 1;
  buildingStrengthFactor = 1;
  bulletSpeed = retrorocket->gameFile->getFloat ("bullet_speed");

  if (retrorocket->allowedWeaponB & WEAPONB_SENTRYGUN) {
    sentryRate = retrorocket->gameFile->getInt ("sentry_gun_rate");
  }

  if (retrorocket->rtsMode) {
    credits = retrorocket->gameFile->getFloat ("start_credits");
    nodeGatherRate =
      retrorocket->gameFile->getFloat ("resource_node_income_rate");
  } else {
    credits = 0;
  }

  weightFactor = 1;
  rotSpeed = retrorocket->gameFile->getFloat ("ship_rotspeed");
  thrust = retrorocket->gameFile->getFloat ("ship_thrust");
  maxStrength = retrorocket->gameFile->getInt ("ship_strength");
  maxLives = retrorocket->gameFile->getInt ("lives");
  userMaxGas = maxGas = retrorocket->gameFile->getInt ("ship_gas");

  gas = userMaxGas;

  if (retrorocket->gameFile->exists ("roundsa")) {
    userMaxWeaponA = maxWeaponA = retrorocket->gameFile->getInt ("roundsa");
  } else {
    userMaxWeaponA = maxWeaponA = 0;
  }
  if (retrorocket->gameFile->exists ("roundsb")) {
    userMaxWeaponB = maxWeaponB = retrorocket->gameFile->getInt ("roundsb");
  } else {
    userMaxWeaponB = maxWeaponB = 0;
  }

  lives = maxLives;
  score = 0;
  immortal = false;
  weaponAInventory = 0;
  bestLapTime = INT_MAX;

  // create weapons

  // weapon A
  if (retrorocket->allowedWeaponA & WEAPONA_STREAM) {
    WeaponA *wpn = new SingleStream (this);
    weaponAAvailable.push_back (wpn);
  }
  if (retrorocket->allowedWeaponA & WEAPONA_SPRAY) {
    WeaponA *wpn = new SprayStream (this);
    weaponAAvailable.push_back (wpn);
  }
  if (retrorocket->allowedWeaponA & WEAPONA_DOUBLE) {
    WeaponA *wpn = new DoubleStream (this);
    weaponAAvailable.push_back (wpn);
  }
  if (retrorocket->allowedWeaponA & WEAPONA_TRIPPLE) {
    WeaponA *wpn = new TriStream (this);
    weaponAAvailable.push_back (wpn);
  }

  // normal weapon B
  if (retrorocket->allowedWeaponB & WEAPONB_SINGLE) {
    WeaponB *wpn = new SingleShot (this);
    weaponBAvailable.push_back (wpn);
  }
  if (retrorocket->allowedWeaponB & WEAPONB_THRUSTSINGLE) {
    WeaponB *wpn = new ThrustSingleShot (this);
    weaponBAvailable.push_back (wpn);
  }
  if (retrorocket->allowedWeaponB & WEAPONB_BUCK) {
    WeaponB *wpn = new BuckShot (this);
    weaponBAvailable.push_back (wpn);
  }
  if (retrorocket->allowedWeaponB & WEAPONB_SEEKER) {
    WeaponB *wpn = new HeatSeeker (this);
    weaponBAvailable.push_back (wpn);
  }
  if (retrorocket->allowedWeaponB & WEAPONB_MINE) {
    WeaponB *wpn = new MineShot (this);
    weaponBAvailable.push_back (wpn);
  }

  // powerful weapon B
  if (retrorocket->allowedWeaponB & WEAPONB_ASM) {
    WeaponB *wpn = new ASM (this);
    weaponBAvailable.push_back (wpn);
  }
  if (retrorocket->allowedWeaponB & WEAPONB_BOMB) {
    WeaponB *wpn = new Bomb (this);
    weaponBAvailable.push_back (wpn);
  }

  // building seed weapons
  if (retrorocket->allowedWeaponB & WEAPONB_RESOURCE_NODE) {
    WeaponB *wpn = new ResourceNodeSeed (this);
    weaponBAvailable.push_back (wpn);
  }
  if (retrorocket->allowedWeaponB & WEAPONB_SENTRYGUN) {
    WeaponB *wpn = new SentrySeed (this);
    weaponBAvailable.push_back (wpn);
  }
  if (retrorocket->allowedWeaponB & WEAPONB_HEATSENTRYGUN) {
    WeaponB *wpn = new HeatSentrySeed (this);
    weaponBAvailable.push_back (wpn);
  }
}

Player::~Player() {
  for (unsigned int i = 0; i < weaponAAvailable.size(); i++) {
    delete weaponAAvailable[i];
  }
  weaponAAvailable.clear();

  for (unsigned int i = 0; i < weaponBAvailable.size(); i++) {
    delete weaponBAvailable[i];
  }
  weaponBAvailable.clear();
}

void Player::registerShip (Ship *ship) {
  this->ship = ship;
}

void Player::updateAction() {
  if (!dead) {
    // Rotate
    if (newActions & ACTION_LEFT) ship->rotateLeft();
    else if (newActions & ACTION_RIGHT) ship->rotateRight();
    else ship->stopRotate();

    // Thrust
    if (newActions & ACTION_THRUST) ship->giveThrust();
    else ship->stopThrust();

    // Shoot A and B
    if (newActions & ACTION_SHOOT_A) ship->shootA();
    if (newActions & ACTION_SHOOT_B) ship->shootB();
  }
}

void Player::shopUpgrade (Upgrades upgrade, bool ignorePrice) {
  switch (upgrade) {
    case UPGRADE_WEIGHT:
      if (ignorePrice || (credits >=
                          getUpgradePrice (upgradeLevels.weight,
                                           "upgrade_weight_price"))) {
        weightFactor -=
          weightFactor *
          retrorocket->gameFile->getFloat ("upgrade_weight_factor");

        if (isLocalPlayer()) {
          retrorocket->network->sendUpgradePacket (id, UPGRADE_WEIGHT);
        }

        if (!ignorePrice) credits -=
                            getUpgradePrice (upgradeLevels.weight,
                                             "upgrade_weight_price");       
        upgradeLevels.weight++;
      }
      break;
    case UPGRADE_THRUST:
      if (ignorePrice || (credits >=
                          getUpgradePrice (upgradeLevels.thrust,
                                           "upgrade_thrust_price"))) {
        thrust +=
          retrorocket->gameFile->getFloat ("upgrade_thrust_increment");

        if (isLocalPlayer()) {
          retrorocket->network->sendUpgradePacket (id, UPGRADE_THRUST);
        }

        if (!ignorePrice) credits -=
                            getUpgradePrice (upgradeLevels.thrust,
                                             "upgrade_thrust_price");
        upgradeLevels.thrust++;
      }
      break;
    case UPGRADE_SHIP_STRENGTH:
      if (ignorePrice || (credits >=
                          getUpgradePrice (upgradeLevels.shipStrength,
                                           "upgrade_ship_strength_price"))) {
        maxStrength *=
          (int) retrorocket->gameFile->getFloat ("upgrade_ship_strength_increment");

        if (isLocalPlayer()) {
          retrorocket->network->sendUpgradePacket (id, UPGRADE_SHIP_STRENGTH);
        }

        if (!ignorePrice) credits -=
                            getUpgradePrice (upgradeLevels.shipStrength,
                                             "upgrade_ship_strength_price");
        upgradeLevels.shipStrength++;
      }
      break;
    case UPGRADE_SHIP_CAPACITY:
      if (ignorePrice || (credits >=
                          getUpgradePrice (upgradeLevels.capacity,
                                           "upgrade_capacity_price"))) {

        maxGas +=
          retrorocket->gameFile->getInt ("upgrade_gas_capacity_increment");

        maxWeaponA +=
          retrorocket->gameFile->getInt ("upgrade_weapona_capacity_increment");

        maxWeaponB +=
          retrorocket->gameFile->getInt ("upgrade_weaponb_capacity_increment");

        if (isLocalPlayer()) {
          retrorocket->network->sendUpgradePacket (id, UPGRADE_SHIP_CAPACITY);
        }

        if (!ignorePrice) credits -=
                            getUpgradePrice (upgradeLevels.capacity,
                                             "upgrade_capacity_price");
        upgradeLevels.capacity++;

        updateMenus();
      }
      break;
    case UPGRADE_BUILDING_STRENGTH:
      if (ignorePrice || (credits >=
                          getUpgradePrice
                          (upgradeLevels.buildingStrength,
                           "upgrade_building_strength_price"))) {

        buildingStrengthFactor *= retrorocket->gameFile->
          getFloat ("upgrade_building_strength_increment");

        if (isLocalPlayer()) {
          retrorocket->network->
            sendUpgradePacket (id, UPGRADE_BUILDING_STRENGTH);
        }

        if (!ignorePrice) credits -=
                            getUpgradePrice (upgradeLevels.buildingStrength,
                                             "upgrade_building_strength_price");
        upgradeLevels.buildingStrength++;
      }
      break;
    case UPGRADE_NODE_GATHER_RATE:
      if (ignorePrice || (credits >=
                          getUpgradePrice (upgradeLevels.nodeGatherRate,
                                           "upgrade_node_gather_price"))) {

        nodeGatherRate -= 
          nodeGatherRate *
          retrorocket->gameFile->getFloat ("upgrade_node_gather_rate_factor");

        if (isLocalPlayer()) {
          retrorocket->network->
            sendUpgradePacket (id, UPGRADE_NODE_GATHER_RATE);
        }

        if (!ignorePrice) credits -=
                            getUpgradePrice (upgradeLevels.nodeGatherRate,
                                             "upgrade_node_gather_price");
        upgradeLevels.nodeGatherRate++;
      }
      break;
    case UPGRADE_FIRING_RATE:
      if (ignorePrice || (credits >=
                          getUpgradePrice (upgradeLevels.firingRate,
                                           "upgrade_firing_rate_price"))) {

        autoFireRate -= 
          (int)(autoFireRate *
                retrorocket->gameFile->getFloat ("upgrade_firing_rate_factor"));

        repetitionRate -=
          (int)(repetitionRate *
                retrorocket->gameFile->getFloat ("upgrade_firing_rate_factor"));

        if (isLocalPlayer()) {
          retrorocket->network->sendUpgradePacket (id, UPGRADE_FIRING_RATE);
        }

        if (!ignorePrice) credits -=
                            getUpgradePrice (upgradeLevels.firingRate,
                                             "upgrade_firing_rate_price");
        upgradeLevels.firingRate++;
      }
      break;
    case UPGRADE_PARTICLE_SPEED:
      if (ignorePrice || (credits >=
                          getUpgradePrice (upgradeLevels.bulletSpeed,
                                           "upgrade_bullet_speed_price"))) {

        bulletSpeed +=
          retrorocket->gameFile->getFloat ("upgrade_bullet_speed_increment");

        if (isLocalPlayer()) {
          retrorocket->network->sendUpgradePacket (id, UPGRADE_PARTICLE_SPEED);
        }

        if (!ignorePrice) credits -=
                            getUpgradePrice (upgradeLevels.bulletSpeed,
                                             "upgrade_bullet_speed_price");
        upgradeLevels.bulletSpeed++;
      }
      break;
    case UPGRADE_DAMAGE:
      if (ignorePrice || (credits >=
                          getUpgradePrice (upgradeLevels.damage,
                                           "upgrade_damage_price"))) {

        damageScaler *=
          retrorocket->gameFile->getFloat ("upgrade_damage_increment");

        if (isLocalPlayer()) {
          retrorocket->network->sendUpgradePacket (id, UPGRADE_DAMAGE);
        }

        if (!ignorePrice) credits -=
                            getUpgradePrice (upgradeLevels.damage,
                                             "upgrade_damage_price");
        upgradeLevels.damage++;
      }
      break;
    case UPGRADE_SENTRYGUN_RATE:
      if (ignorePrice || (credits >=
                          getUpgradePrice (upgradeLevels.sentryRate,
                                           "upgrade_sentry_rate_price"))) {

        sentryRate -=
          (int)(sentryRate *
                retrorocket->gameFile->getFloat ("upgrade_sentry_rate_factor"));

        if (isLocalPlayer()) {
          retrorocket->network->sendUpgradePacket (id, UPGRADE_SENTRYGUN_RATE);
        }

        if (!ignorePrice) credits -=
                            getUpgradePrice (upgradeLevels.sentryRate,
                                             "upgrade_sentry_rate_price");
        upgradeLevels.sentryRate++;
      }
      break;
    case UPGRADE_STEALTH_LEVEL:
      if (ignorePrice || (credits >=
                          getUpgradePrice (upgradeLevels.stealth,
                                           "upgrade_stealth_price"))) {
        if (isLocalPlayer()) {
          retrorocket->network->sendUpgradePacket (id, UPGRADE_STEALTH_LEVEL);
        }

        if (!ignorePrice) credits -=
                            getUpgradePrice (upgradeLevels.stealth,
                                             "upgrade_stealth_price");
        upgradeLevels.stealth++;
        if (upgradeLevels.stealth >
            retrorocket->players[retrorocket->localShip]->upgradeLevels.radar) {
          hideMapSprite (id);
        }
        drawSOBsOnMap();

      }
      break;
    case UPGRADE_RADAR_LEVEL:
      if (ignorePrice || (credits >=
                          getUpgradePrice (upgradeLevels.radar,
                                           "upgrade_radar_price"))) {
        if (isLocalPlayer()) {
          retrorocket->network->sendUpgradePacket (id, UPGRADE_RADAR_LEVEL);
        }

        if (!ignorePrice) credits -=
                            getUpgradePrice (upgradeLevels.radar,
                                             "upgrade_radar_price");
        upgradeLevels.radar++;
        for (unsigned int i = 0; i < retrorocket->ships.size(); i++) {
          if (retrorocket->players[i]->upgradeLevels.stealth <=
              retrorocket->players[retrorocket->localShip]->
              upgradeLevels.radar) {
            showMapSprite (i);
          }
        }
        drawSOBsOnMap();

      }
      break;
  }
}

int Player::getUpgradePrice (int level, const char* upgradePriceTag) {
  int price = retrorocket->gameFile->getInt (upgradePriceTag);
  return price * k_ipow (2, level);
}


///////////////////////////////////////////////////////////////////////////////

void RemotePlayer::registerShip (Ship *ship) {
  Player::registerShip (ship);

  if (weaponAAvailable.size() > 0) {
    ship->weaponA = weaponAAvailable[0];
  } else {
    ship->weaponA = NULL;
  }
  if (weaponBAvailable.size() > 0) {
    ship->weaponB = weaponBAvailable[0];
  } else {
    ship->weaponB = NULL;
  }
}

void RemotePlayer::setWeaponA (int weapon) {
  if (weapon == -1)
    return;
  for (unsigned int i = 0; i < weaponAAvailable.size(); i++) {
    if (weaponAAvailable[i]->type == weapon) {
      ship->weaponA = weaponAAvailable[i];
      return;
    }
  }
  panic("Can't set wpn A: %d", weapon);
}

void RemotePlayer::setWeaponB (int weapon) {
  if (weapon == -1)
    return;
  for (unsigned int i = 0; i < weaponBAvailable.size(); i++) {
    if (weaponBAvailable[i]->type == weapon) {
      ship->weaponB = weaponBAvailable[i];
      return;
    }
  }
  panic("Can't set wpn B: %d", weapon);
}

void RemotePlayer::advance() {
  updateAction();
}

///////////////////////////////////////////////////////////////////////////////

LocalPlayer::LocalPlayer (unsigned int id) : Player (id) {
  curWeaponA = 0;
  curWeaponB = 0;
  vblcount = 0;

  menuRemoved = true;

  // setup menu
  {
    homeMenu = new Menu ("Home Menu", 4);
    homeMenu->setQuitable();

    shipConfigMenu = new Menu ("Configure Ship", 4);
    shipConfigMenu->setCallback (updateShipConfig);

    homeMenu->addItem ("Configure ship", shipConfigMenu);

    homeMenu->addItem ("Save configuration", requestConfigFilename);

    loadConfigMenu = new Menu ("Load Configuration", 4);

    string suffix = ".";
    suffix.append (USERCONFEXT);
    loadConfigMenu->setSuffixToStrip (suffix);

    homeMenu->addItem ("Load configuration", loadConfigMenu);

    if (retrorocket->rtsMode) {
      // ammo shop
      buyAmmoMenu = new Menu ("Buy Ammunition", 6);
      buyAmmoMenu->setCallback (displayInventory);
      homeMenu->addItem ("Buy ammunition", buyAmmoMenu);

      // weapon shop
      buyWeaponsMenu = new Menu ("Buy Weapons", 6);
      buyWeaponsMenu->setCallback (displayWeaponPrice);
      homeMenu->addItem ("Buy weapons", buyWeaponsMenu);

      // upgrade shop
      buyUpgradesMenu = new Menu ("Buy Upgrades", 6);
      buyUpgradesMenu->setCallback (upgradeStatus);
      buyUpgradesMenu->addItem ("Buildings: Improved strength",
                                shopUpgradeAction,
                                (int)UPGRADE_BUILDING_STRENGTH);
      buyUpgradesMenu->addItem ("Buildings: Sentry gun speed",
                                shopUpgradeAction,
                                (int)UPGRADE_SENTRYGUN_RATE);
      buyUpgradesMenu->addItem ("Buildings: $ gather rate",
                                shopUpgradeAction,
                                (int)UPGRADE_NODE_GATHER_RATE);
      buyUpgradesMenu->addItem ("Ship: Hull strength", shopUpgradeAction,
                                (int)UPGRADE_SHIP_STRENGTH);
      buyUpgradesMenu->addItem ("Ship: Load capacity", shopUpgradeAction,
                                (int)UPGRADE_SHIP_CAPACITY);
      buyUpgradesMenu->addItem ("Ship: Radar level", shopUpgradeAction,
                                (int)UPGRADE_RADAR_LEVEL);
      buyUpgradesMenu->addItem ("Ship: Reduced weight", shopUpgradeAction,
                                (int)UPGRADE_WEIGHT);
      buyUpgradesMenu->addItem ("Ship: Stealth level", shopUpgradeAction,
                                (int)UPGRADE_STEALTH_LEVEL);
      buyUpgradesMenu->addItem ("Ship: Thrust engines", shopUpgradeAction,
                                (int)UPGRADE_THRUST);
      buyUpgradesMenu->addItem ("Weapons: Firing rate", shopUpgradeAction,
                                (int)UPGRADE_FIRING_RATE);
      buyUpgradesMenu->addItem ("Weapons: Damage factor", shopUpgradeAction,
                                (int)UPGRADE_DAMAGE);
      buyUpgradesMenu->addItem ("Weapons: Projectile speed", shopUpgradeAction,
                                (int)UPGRADE_PARTICLE_SPEED);
      homeMenu->addItem ("Buy upgrades", buyUpgradesMenu);
    }

    // quit menu
    Menu *quitMenu = new Menu ("Really quit?", 4);
    quitMenu->addItem ("Yes", RetroRocket::quitGame);

    homeMenu->addItem ("Quit game", quitMenu);
  }

  { // create player list menu (for use after player is dead)
    playerListMenu = new Menu ("You died");
    for (int i = 0; i < retrorocket->network->numberOfShips; i++) {
      char *itemText = (char*)malloc (MENUITEMSIZE + 1);
      snprintf (itemText, MENUITEMSIZE + 1, "Watch %s",
                retrorocket->playerNames[i]);
      itemText[MENUITEMSIZE] = 0;
      playerListMenu->addItem (itemText);
    }
    playerListMenu->setCallback (changeFocus);
  }

  // if not rtsmode -- all available weapons are initially bought
  if (!retrorocket->rtsMode) {
    vector<WeaponA*>::iterator i = weaponAAvailable.begin();
    while (i != weaponAAvailable.end()) {
      i = buyWeaponA (i);
    }
    vector<WeaponB*>::iterator j = weaponBAvailable.begin();
    while (j != weaponBAvailable.end()) {
      j = buyWeaponB (j);
    }
  }

  updateMenus();

  menu = homeMenu;
}

void LocalPlayer::changeFocus (Menu *menu) {
  int chosenPlayer = menu->getChoice();
  setFocus (retrorocket->ships[chosenPlayer]);
  retrorocket->watchedShip = chosenPlayer;
}

LocalPlayer::~LocalPlayer() {
  Menu::freeAllMenus (homeMenu);

  for (unsigned int i = 0; i < weaponABought.size(); i++) {
    delete weaponABought[i];
  }
  for (unsigned int i = 0; i < weaponBBought.size(); i++) {
    delete weaponBBought[i];
  }

  for (int i = 1; i < playerListMenu->getNumberOfItems(); i++) {
    free ((void*)playerListMenu->getItemText (i));
  }
  Menu::freeAllMenus (playerListMenu);
}

void LocalPlayer::updateAction() {
  if (!dead) {
    // send updated actions to clients
    if (newActions != oldActions) {

      // send conf packet if action on platform
      if (retrorocket->field->landed (ship)) {
        retrorocket->network->sendConfPacket (ship);
      }

      // send action packets (ship and ball)
      retrorocket->network->sendActionPacket (ship);
      vblcount = 0;
      if (ship->child) ((Ball*)ship->child)->sendBallPacket();

      retrorocket->ships[id]->shootATime = 0;
    } else if (vblcount > retrorocket->updatefrequency) {
      // send new action packet if long time since last time
      retrorocket->network->sendActionPacket (ship);
      if (ship->child) ((Ball*)ship->child)->sendBallPacket();
      vblcount = 0;
    }

    // handle local actions

    // Beam
    if (newActions & ACTION_BEAM) ship->beam();
    // TODO: gas containters?

    // Display menu if at home
    if ((newActions & ACTION_MENU)) {
      if ((retrorocket->field->landed (ship) & Field::HOME) && !dead) {
        hideMap();
        homeMenu->presentMenuAsync();
        menuRemoved = false;
      }
    }
    Player::updateAction();
  }
}

void LocalPlayer::advance() {
  vblcount++;
  getInput();

  if (!menuRemoved) {
    if (menu->isDone()) {
      menu->remove();
      showMap();
      menuRemoved = true;
    } else {
      menu->advance();
    }
  }
}

void LocalPlayer::setPermanentlyDead() {
  if (retrorocket->network->numberOfShips > 2) {
    // show player list menu if there are more than two players
    menu->remove();
    playerListMenu->presentMenuAsync (id);
    playerListMenu->setChoice (id);
    menu = playerListMenu;
    menuRemoved = false;
  }
}

void LocalPlayer::updateMenus() {
  // ship config
  weaponAList.clear();
  for (unsigned int i = 0; i < weaponABought.size(); i++) {
    weaponAList.push_back (weaponABought[i]->name());
  }
  weaponBList.clear();
  for (unsigned int i = 0; i < weaponBBought.size(); i++) {
    weaponBList.push_back (weaponBBought[i]->name());
  }
  shipConfigMenu->updateMenu();

  if (retrorocket->rtsMode) {

    // buy ammo
    buyAmmoMenu->clearItems();
    buyAmmoMenu->addItem ("Weapon A: All Purpose Ammo", shopAmmo, 0);
    for (unsigned int i = 0; i < weaponBBought.size(); i++) {
      Weapon *wpn = weaponBBought[i];
      buyAmmoMenu->addItem (wpn->fullName(), shopAmmo, (long int)wpn);
    }
    buyAmmoMenu->updateMenu();

    // buy weapons
    buyWeaponsMenu->clearItems();

    for (unsigned int i = 0; i < weaponAAvailable.size(); i++) {
      Weapon *wpn = weaponAAvailable[i];
      buyWeaponsMenu->addItem (wpn->fullName(), shopWeaponA, i);
    }
    for (unsigned int i = 0; i < weaponBAvailable.size(); i++) {
      WeaponB *wpn = (WeaponB*)weaponBAvailable[i];
      buyWeaponsMenu->addItem (wpn->fullName(), shopWeaponB, i);
    }
    buyWeaponsMenu->updateMenu();
  }

  // update list of config filenames
  for (int i = 0; i < loadConfigMenu->getNumberOfItems(); i++) {
    free ((void*)loadConfigMenu->getItemText (i));
  }
  loadConfigMenu->clearItems();
  vector<char*>filenames;
  getFileList (USERCONFEXT, &filenames);
  for (unsigned int i = 0; i < filenames.size(); i++) {
    loadConfigMenu->addItem (filenames[i], loadConfig);
  }

  shipConfigMenu->clearItems();
  shipConfigMenu->addItem ("Gas ", 0, maxGas,
                           &userMaxGas);
  shipConfigMenu->addItem ("#A  ", 0, maxWeaponA,
                           &userMaxWeaponA);
  shipConfigMenu->addItem ("#B  ", 0, maxWeaponB,
                           &userMaxWeaponB);
  shipConfigMenu->addItem ("Weapon A", &curWeaponA, &weaponAList);
  shipConfigMenu->addItem ("Weapon B", &curWeaponB, &weaponBList);
}

void LocalPlayer::loadConfig (Menu *menu) {
  const char *filename = menu->getCurrentItemText();
  Config c(filename);
  LocalPlayer *player = (LocalPlayer*)retrorocket->
    players[retrorocket->localShip];
  player->userMaxGas = c.getInt ("gas");
  if (player->userMaxGas > player->maxGas) {
    player->userMaxGas = player->maxGas;
  }
  player->userMaxWeaponA = c.getInt ("roundsa");
  if (player->userMaxWeaponA > player->maxWeaponA) {
    player->userMaxWeaponA = player->maxWeaponA;
  }
  player->userMaxWeaponB = c.getInt ("roundsb");
  if (player->userMaxWeaponB > player->maxWeaponB) {
    player->userMaxWeaponB = player->maxWeaponB;
  }
  const char *a = c.getString ("weapona");
  const char *b = c.getString ("weaponb");
  for (unsigned int i = 0; i < player->weaponABought.size(); i++) {
    const char *s = player->weaponABought[i]->name();
    if (!strcmp (s, a)) {
      player->curWeaponA = i;
    }
  }
  for (unsigned int i = 0; i < player->weaponBBought.size(); i++) {
    const char *s = player->weaponBBought[i]->name();
    if (!strcmp (s, b)) {
      player->curWeaponB = i;
    }
  }
  updateShipConfig (NULL);
}

void LocalPlayer::updateShipConfig (Menu *menu) {
  LocalPlayer *player = (LocalPlayer*)retrorocket->
    players[retrorocket->localShip];
  player->updateShipConfig();
}

void LocalPlayer::updateShipConfig() {
  if ((curWeaponA != -1) && (weaponABought.size() > 0)) {
    ship->weaponA = weaponABought[curWeaponA];
  } else {
    ship->weaponA = NULL;
  }
  if ((curWeaponB != -1) && (weaponBBought.size() > 0)) {
    ship->weaponB = weaponBBought[curWeaponB];
  } else {
    ship->weaponB = NULL;
  }
}

void LocalPlayer::registerShip (Ship *ship) {
  Player::registerShip (ship);
  updateShipConfig();
}

void LocalPlayer::requestConfigFilename (Menu *menu) {
  LocalPlayer *player = (LocalPlayer*)retrorocket->
    players[retrorocket->localShip];
  menu->hide();
  readLine (player->saveConfig, "Enter filename:");
}

void LocalPlayer::saveConfig (char *str) {
  LocalPlayer *player = (LocalPlayer*)retrorocket->
    players[retrorocket->localShip];

  Config c;
  c.putInt ("gas", player->userMaxGas);
  c.putInt ("roundsa", player->userMaxWeaponA);
  c.putInt ("roundsb", player->userMaxWeaponB);
  if (player->ship->weaponA) {
    c.putString ("weapona", player->ship->weaponA->name());
  } else {
    c.putString ("weapona", "<none>");
  }
  if (player->ship->weaponB) {
    c.putString ("weaponb", player->ship->weaponB->name());
  } else {
    c.putString ("weaponb", "<none>");
  }
  char filename[FILENAMESIZE];
  sprintf (filename, "%s.%s", str, USERCONFEXT);
  c.save (filename);

  player->updateMenus();
  player->menu->show();
}

void LocalPlayer::shopAmmo (Menu *menu) {
  WeaponB *wpn = (WeaponB*)menu->getChosenItem()->actionArg;
  LocalPlayer *player = (LocalPlayer*)retrorocket->
    players[retrorocket->localShip];
  if (wpn) {
    // weapon B ammo
    if (player->credits >= wpn->ammoPrice) {
      wpn->inventory += wpn->quanta;
      player->credits -= wpn->ammoPrice;
    }
  } else {
    // weapon A ammo
    if (player->credits >= WeaponA::ammoPrice) {
      player->weaponAInventory += WeaponA::quanta;
      player->credits -= WeaponA::ammoPrice;
    }
  }
}

void LocalPlayer::shopWeaponA (Menu *menu) {
  LocalPlayer *player = (LocalPlayer*)retrorocket->
    players[retrorocket->localShip];
  int availableID = menu->getChosenItem()->actionArg;

  vector<WeaponA*>::iterator i = player->weaponAAvailable.begin();
  i += availableID;
  Fix32 price = player->weaponAAvailable[availableID]->price;
  
  if (player->credits >= price) {
    player->buyWeaponA (i);
    player->credits -= price;
  }
}

void LocalPlayer::shopWeaponB (Menu *menu) {
  LocalPlayer *player = (LocalPlayer*)retrorocket->
    players[retrorocket->localShip];
  int availableID = menu->getChosenItem()->actionArg;

  vector<WeaponB*>::iterator i = player->weaponBAvailable.begin();
  i += availableID;
  Fix32 price = player->weaponBAvailable[availableID]->price;
  
  if (player->credits >= price) {
    player->buyWeaponB (i);
    player->credits -= price;
  }
}

void LocalPlayer::upgradeStatus (Menu *menu) {
  LocalPlayer *player =
    (LocalPlayer*)retrorocket->players[retrorocket->localShip];

  clearText (ROWS-5,ROWS-5);
  setTextColor(GREEN);

  switch ((Upgrades)menu->getChosenItem()->actionArg) {
    case UPGRADE_WEIGHT:
      outputText (1, ROWS-5, "Now: lv%d (%.2fx)",
                  player->upgradeLevels.weight,
                  player->weightFactor.toFloat());
      outputText (20, ROWS-5, "Next: $%d",
                  getUpgradePrice (player->upgradeLevels.weight,
                                           "upgrade_weight_price"));
      break;
    case UPGRADE_THRUST:
      outputText (1, ROWS-5, "Now: lv%d", player->upgradeLevels.thrust);
      outputText (20, ROWS-5, "Next: $%d",
                  getUpgradePrice (player->upgradeLevels.thrust,
                                           "upgrade_thrust_price"));
      break;
    case UPGRADE_SHIP_STRENGTH:
      outputText (1, ROWS-5, "Now: lv%d (%d)",
                  player->upgradeLevels.shipStrength,
                  player->maxStrength);
      outputText (20, ROWS-5, "Next: $%d",
                  getUpgradePrice (player->upgradeLevels.shipStrength,
                                           "upgrade_ship_strength_price"));
      break;
    case UPGRADE_SHIP_CAPACITY:
      outputText (1, ROWS-5, "G:%d A:%d B:%d",
                  player->maxGas,
                  player->maxWeaponA,
                  player->maxWeaponB);
      outputText (20, ROWS-5, "Next: $%d",
                  getUpgradePrice (player->upgradeLevels.capacity,
                                           "upgrade_capacity_price"));
      break;
    case UPGRADE_BUILDING_STRENGTH:
      outputText (1, ROWS-5, "Now: lv%d (%f)",
                  player->upgradeLevels.buildingStrength,
                  (player->buildingStrengthFactor).toFloat());
      outputText (20, ROWS-5, "Next: $%d",
                  getUpgradePrice (player->upgradeLevels.buildingStrength,
                                           "upgrade_building_strength_price"));
      break;
    case UPGRADE_FIRING_RATE:
      outputText (1, ROWS-5, "Now: lv%d",
                  player->upgradeLevels.firingRate);
      outputText (20, ROWS-5, "Next: $%d",
                  getUpgradePrice (player->upgradeLevels.firingRate,
                                           "upgrade_firing_rate_price"));
      break;
    case UPGRADE_PARTICLE_SPEED:
      outputText (1, ROWS-5, "Now: lv%d",
                  player->upgradeLevels.bulletSpeed);
      outputText (20, ROWS-5, "Next: $%d",
                  getUpgradePrice (player->upgradeLevels.bulletSpeed,
                                           "upgrade_bullet_speed_price"));
      break;
    case UPGRADE_DAMAGE:
      outputText (1, ROWS-5, "Now: lv%d (%.2fx)",
                  player->upgradeLevels.damage, player->damageScaler.toFloat());
      outputText (20, ROWS-5, "Next: $%d",
                  getUpgradePrice (player->upgradeLevels.damage,
                                           "upgrade_damage_price"));
      break;
    case UPGRADE_SENTRYGUN_RATE:
      outputText (1, ROWS-5, "Now: lv%d",
                  player->upgradeLevels.sentryRate);
      outputText (20, ROWS-5, "Next: $%d",
                  getUpgradePrice (player->upgradeLevels.sentryRate,
                                           "upgrade_sentry_rate_price"));
      break;
    case UPGRADE_NODE_GATHER_RATE:
      outputText (1, ROWS-5, "Now: lv%d",
                  player->upgradeLevels.nodeGatherRate);
      outputText (20, ROWS-5, "Next: $%d",
                  getUpgradePrice (player->upgradeLevels.nodeGatherRate,
                                           "upgrade_node_gather_price"));
      break;
    case UPGRADE_STEALTH_LEVEL:
      outputText (1, ROWS-5, "Now: lv%d",
                  player->upgradeLevels.stealth);
      outputText (20, ROWS-5, "Next: $%d",
                  getUpgradePrice (player->upgradeLevels.stealth,
                                           "upgrade_stealth_price"));
      break;
    case UPGRADE_RADAR_LEVEL:
      outputText (1, ROWS-5, "Now: lv%d", player->upgradeLevels.radar);
      outputText (20, ROWS-5, "Next: $%d",
                  getUpgradePrice (player->upgradeLevels.radar,
                                           "upgrade_radar_price"));
      break;
  }
}

void LocalPlayer::shopUpgradeAction (Menu *menu) {
  LocalPlayer *player = (LocalPlayer*)retrorocket->
    players[retrorocket->localShip];
  Upgrades upgrade = (Upgrades)menu->getChosenItem()->actionArg;
  player->shopUpgrade (upgrade);
}

vector<WeaponA*>::iterator
LocalPlayer::buyWeaponA (vector<WeaponA*>::iterator i) {
  WeaponA *wpn = *i;
  weaponABought.push_back (wpn);
  vector<WeaponA*>::iterator r = weaponAAvailable.erase (i);
  updateMenus();
  return r;
}

vector<WeaponB*>::iterator
LocalPlayer::buyWeaponB (vector<WeaponB*>::iterator i) {
  WeaponB *wpn = *i;
  weaponBBought.push_back (wpn);
  vector<WeaponB*>::iterator r = weaponBAvailable.erase (i);
  updateMenus();
  return r;
}

void LocalPlayer::displayInventory (Menu *menu) {
  LocalPlayer *player = (LocalPlayer*)retrorocket->
    players[retrorocket->localShip];

  clearText (ROWS-5, ROWS-5);
  setTextColor(GREEN);

  WeaponB *wpn = (WeaponB*)menu->getChosenItem()->actionArg;
  if (wpn) {
    outputText (1, ROWS-5, "Inventory: %d", wpn->inventory);
    outputText (17, ROWS-5, "Buy %2d: $%d", wpn->quanta,
                wpn->ammoPrice.toInt());
  } else {
    outputText (1, ROWS-5, "Inventory: %d", player->weaponAInventory);
    outputText (17, ROWS-5, "Buy %2d: $%d", WeaponA::quanta,
                WeaponA::ammoPrice.toInt());
  }
}

void LocalPlayer::displayWeaponPrice (Menu *menu) {
  LocalPlayer *player = (LocalPlayer*)retrorocket->
    players[retrorocket->localShip];

  clearText (ROWS-5, ROWS-5);
  setTextColor(GREEN);

  if (shopWeaponA == menu->getChosenItem()->action) {
    Weapon *wpn = player->weaponAAvailable[menu->getChosenItem()->actionArg];
    outputText (20, ROWS-5, "Buy: $%d", wpn->price.toInt());
  } else {
    Weapon *wpn = player->weaponBAvailable[menu->getChosenItem()->actionArg];
    outputText (20, ROWS-5, "Buy: $%d", wpn->price.toInt());
  }
}

void LocalPlayer::setWeaponA (int weapon) {
  if (weapon == -1)
    return;
  for (unsigned int i = 0; i < weaponABought.size(); i++) {
    if (weaponABought[i]->type == weapon) {
      ship->weaponA = weaponABought[i];
      return;
    }
  }
  panic("Can't set wpn A: %d", weapon);
}

void LocalPlayer::setWeaponB (int weapon) {
  if (weapon == -1)
    return;
  for (unsigned int i = 0; i < weaponBBought.size(); i++) {
    if (weaponBBought[i]->type == weapon) {
      ship->weaponB = weaponBBought[i];
      return;
    }
  }
  panic("Can't set wpn B: %d", weapon);
}

void LocalPlayer::getInput() {
  if (homeMenu->isDone()) {
    oldActions = newActions;
    newActions = 0;

    unsigned int sysInput = sysGetInput();

    // Rotate
    if (sysInput & retrorocket->localControls.left) {
      newActions |= ACTION_LEFT;
    } else if (sysInput & retrorocket->localControls.right) {
      newActions |= ACTION_RIGHT;
    }

    // Thrust
    if (sysInput & retrorocket->localControls.thrust) {
      newActions |= ACTION_THRUST;
    }

    // Shoot A
    if (ship->weaponA) {
      if (sysInput & retrorocket->localControls.triggerA) {
        newActions |= ACTION_SHOOT_A;
      }
    }

    // Shoot B
    if (sysInput & retrorocket->localControls.triggerB) {
      if (shootBKeyPressed == false) {
        newActions |= ACTION_SHOOT_B;
      }
      shootBKeyPressed = true;
    } else {
      shootBKeyPressed = false;
    }

    // Beam
    if (sysInput & retrorocket->localControls.beam) {
      newActions |= ACTION_BEAM;
    }

    // Display Menu
    if ((sysInput & retrorocket->localControls.menu) || getPointer()) {
      newActions |= ACTION_MENU;
    }

    updateAction();
  }
}


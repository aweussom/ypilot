/*
  field.cpp
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
#include "sys/dir.h"

int autogenTileLUT[20] = {
  0,                           // 
  6,                           //       w
  TILE_HFLIP | 6,              //     e
  2,                           //     e w
  5,                           //   s 
  TILE_HFLIP | TILE_VFLIP | 4, //   s   w
  TILE_VFLIP | 4,              //   s e
  7,                           //   s e w
  TILE_VFLIP | 5,              // n      
  TILE_HFLIP | 4,              // n     w    
  4,                           // n   e    
  TILE_VFLIP | 7,              // n   e w  
  3,                           // n s    
  TILE_HFLIP | 8,              // n s   w  
  8,                           // n s e  
  1,                           // n s e w OR center filled
  TILE_HFLIP | 1,
  TILE_VFLIP | 1,
  TILE_HFLIP | TILE_VFLIP | 1
};

Field::Field (char *filename) {
  srand (retrorocket->fieldSeed);

  Config file (filename);
  load (&file);
}

Field::Field (Config *file) {
  srand (retrorocket->fieldSeed);

  load (file);
}

Field::Field (Config *c, WorldGen *worldgen) {
  srand (retrorocket->fieldSeed);

  worldgen->generate();
  worldgen->removeThinLinesFilter();

  width = worldgen->width * 8;
  height = worldgen->height * 8;

  createFromBM (c, worldgen->width, worldgen->height,
                worldgen->autogenBuffer, worldgen->seaLevel);
  foundBM = false;

  // create SOB bg
  foundSOB = true;
  mapSOB = (u16*)calloc (1, mapsize);
  if (!mapSOB) panic ("Out of memory");
  tilessizeSOB = 0;
  paletteSizeSOB = 0;
  Sob::initSOBs (&tilesSOB, &paletteSOB, &tilessizeSOB, &paletteSizeSOB);

  // platforms
  PlatformSOB::copyTiles (&tilesSOB, &paletteSOB,
                          &tilessizeSOB, &paletteSizeSOB);
  RRMarker::copyTiles (&tilesSOB, &paletteSOB,
                     &tilessizeSOB, &paletteSizeSOB);
  for (unsigned int i = 0; i < worldgen->platforms.size(); i++) {
    Platform platform;
    RRPoint p = worldgen->platforms[i];
    platform.x1 = p.x * 8;
    platform.y = p.y * 8;
    platform.x2 = (p.x * 8) + PlatformSOB::platformWidth();
    platform.properties = 0;
    platform.homeShip = i;
    platforms.push_back (platform);
    new PlatformSOB (platform.x1, platform.y, i);
  }

  if (c->exists ("score")) {
    score = c->getInt ("score");
  } else {
    score = 0;
  }

  loadBg (info, mapBG, mapsize, tilesBG, tilessizeBG, paletteBG, BACK_BG);
  loadBg (info, mapSOB, mapsize, tilesSOB, tilessizeSOB, paletteSOB, SOB_BG);

  drawMap (collisionMap, collisionTiles, info, width, height,
           0, 0, SCREEN_WIDTH, SCREEN_HEIGHT-32);

  findBlankTile();
}

void Field::createFromBM (Config *c, int bmWidth, int bmHeight,
                          u8 *bmBuffer, int seaLevel) {
  // info
  info = (int*)malloc (3 * sizeof (int));
  if (!info) panic ("Out of memory");
  info[0] = BG_LARGEMAP;
  info[1] = width;
  info[2] = height;

  ConfigContext configContext;

  if (c->findID ("tileset", &configContext)) {
    FILE *fp;
    const char *tileFilename, *palFilename;
    u8 *thisTileSet;
    
    tilesBG = (u8*)malloc (TILESETSIZE * c->countID ("tileset") );
    if (!tilesBG) panic ("Out of memory");
    thisTileSet = tilesBG;
    
    paletteBG = NULL;
    paletteSizeBG = 0;

    do {
      tileFilename = c->getString (&configContext);
      palFilename = c->getString (&configContext);

      if ((fp = fopen (tileFilename, "rb"))) {
        if ( TILESETSIZE > (int)fread ( thisTileSet, 1, TILESETSIZE, fp ) )
          panic ( "File read error" );
      } else {
        panic ( "Can not open %s", tileFilename );
      }
      fclose (fp);
      
      // update color index to correct palette
      for (int i = 0; i < TILESETSIZE; i++) {
        if (*(thisTileSet + i)) {
          *(thisTileSet + i) += paletteSizeBG / sizeof (u16);
        }
      }

      thisTileSet += TILESETSIZE;

      if (!paletteBG) {
        // read first palette
        paletteSizeBG = readBinary ((u8**)&paletteBG, palFilename);
      }
      else {
        // append palette
        u8 *tmp;
        int tmpSize = readBinary ((u8**)&tmp, palFilename);
        paletteBG = (u16*)realloc (paletteBG, paletteSizeBG + tmpSize);
        memcpy (((u8*)paletteBG) + paletteSizeBG, tmp, tmpSize);
        free (tmp);
        paletteSizeBG += tmpSize;
      }

    } while (c->next (&configContext));

    tilessizeBG = TILESETSIZE * c->countID("tileset");

  } else {
    panic ("Parse error: tileset");
  }

  foundFG = false;
  foundBG = true;

  // map
  mapsize = bmWidth * bmHeight * sizeof (u16);
  mapBG = (u16*)malloc (mapsize);
  if (!mapBG) panic ("Out of memory");
  for (int y = 0; y < bmHeight; y++) {
    for (int x = 0; x < bmWidth; x++) {
      u8 *buf = bmBuffer;
      int center = buf[y * bmWidth + x] > seaLevel ? 1 : 0;
      int lutIndex;
      if (!center) {
        int north = (y > 0) ?
          (buf[(y-1)*bmWidth+x] > seaLevel ? 1 : 0) : 0;
        int south = (y < bmHeight-1) ?
          (buf[(y+1)*bmWidth+x] > seaLevel ? 1 : 0) : 0;
        int east = (x < bmWidth-1) ?
          (buf[y*bmWidth+(x+1)] > seaLevel ? 1 : 0) : 0;
        int west = (x > 0) ?
          (buf[y*bmWidth+(x-1)] > seaLevel ? 1 : 0) : 0;
        lutIndex = (north<<3) | (south<<2) | (east<<1) | west;
      } else {
        lutIndex = 15;
      }
      // randomize orientation of center/filled tile
      if (lutIndex == 15)
        lutIndex += rand() % 4;
      
      mapBG[y * bmWidth + x] = autogenTileLUT[lutIndex] +
        ( rand() % c->countID("tileset") ) * AUTOGENTILES;

    }
  }

  // collision (same as map)
  collisionTilessize = tilessizeBG;
  collisionTiles = tilesBG;
  collisionMap = mapBG;
  foundCM = false;
}

/** Find the tile that represents completely blank tile
    Used for speeding up collision detection */
void Field::findBlankTile() {
  for (int i = 0; i < (collisionTilessize >> 6); i++) {
    bool blank = true;

    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (*(collisionTiles + (i * 8 * 8) + (y * 8) + x)
            != TRANSPARENT_COLOR) {
          blank = false;
        }
      }
      if (blank) {
        blankTile = i;
        return;
      }
    }
  }
}

bool Field::fieldIsOfGivenType (char *filename,
                                int gameMode, int userMinPlayers,
                                int computerPlayers) {
  Config c (filename);

  int minShips = c.getInt ("minplayers");
  int maxShips = c.getInt ("maxplayers");
  bool isThrust = c.exists ("ball") || c.exists ("thrustmode");
  bool isRTS = c.exists ("resource") || c.exists ("rtsmode");;

  int minPlayers = userMinPlayers + computerPlayers;

  if (gameMode >= 2 && computerPlayers > 0) return false;
  if (gameMode == 1 && minPlayers <= 1) return false;

  return (((gameMode == 0 || gameMode == 1) && !isThrust && !isRTS) ||
    (gameMode == 2 && isThrust) || (gameMode == 3 && isRTS)) &&
    (minPlayers >= minShips && minPlayers <= maxShips);
}

void Field::printFieldInfo (Menu *menu) {
  if (menu->getNumberOfItems() <= 0) return;

  retrorocket->hideSplash();

  if (retrorocket->gameMode == 0 || retrorocket->gameMode == 1) {
    if (menu->getChoice() == 0) {
      hideMap();
      clearText (ROWS-2, ROWS);
      outputText (1, ROWS-2, "Players: 1-%d",
                  retrorocket->config->countID ("color_player"));
      return;
    }
  }

  const char *filename = menu->getCurrentItemText();

  Config c (filename);

  int minShips = c.getInt ("minplayers");
  int maxShips = c.getInt ("maxplayers");

  ConfigContext configContext;

  bool bm = false;

  u8 *bitmap = NULL;
  int *inf = NULL;
  u16 *cMap = NULL;
  u8 *cTiles = NULL;
  int w = 256;
  int h = 192;

  if (c.findID ("bitmap", &configContext)) {
    bm = true;

    w = c.getInt ("width");
    h = c.getInt ("height");

    readBinary ((u8**)&bitmap, c.getString (&configContext));
    int level = c.getInt (&configContext);
    
    drawBM (bitmap, w/8, h/8, level, 32, 32, SCREEN_WIDTH-64, SCREEN_HEIGHT-64);
    showMap();

    free (bitmap);
  } else if (c.exists ("collision")) {
    w = c.getInt ("width");
    h = c.getInt ("height");

    readBinary ((u8**)&inf, c.getString ("info"));

    c.findID ("collision", &configContext);
    readBinary ((u8**)&cTiles, c.getString (&configContext));
    readBinary ((u8**)&cMap, c.getString (&configContext));

    drawMap (cMap, cTiles, inf, w, h, 32, 32,
             SCREEN_WIDTH-64, SCREEN_HEIGHT-64);
    showMap();

    free (inf);
    free (cMap);
    free (cTiles);
  } else if (!bm) {
    hideMap();
  }

  clearText (ROWS-2, ROWS);

  if (minShips != maxShips) {
    outputText (1, ROWS-2, "Players: %d-%d", minShips, maxShips);
  } else {
    outputText (1, ROWS-2, "Players: %d", minShips);
  }

  Config high;
  high.load (HIGHSCOREFILE);
  if (high.findID (filename, &configContext)) {
    if (maxShips == 1) {
      outputText (1, ROWS-1, "Highscore: %d", high.getInt (&configContext));
    } else {
      outputText (1, ROWS-1, "Ranking: %d",
                  (int)(high.getFloat (&configContext) * 1000));
    }
    if (c.exists ("waypoint")) {
      int ms = vblToCSec (high.getInt (&configContext));
      if (ms > 0) {
        outputText (20, ROWS-1, "Lap: %d:%02d", (int)(ms/100), ms%100);
      }
    }
  }
}

void Field::load (Config *c) {
  width = c->getInt ("width");
  height = c->getInt ("height");

  ConfigContext configContext;

  foundBM = false;
  foundFG = false;
  foundBG = false;

  if (c->findID ("bitmap", &configContext)) {
    foundBM = true;
    readBinary ((u8**)&bmBuffer, c->getString (&configContext));
    createFromBM (c, width/8, height/8, bmBuffer, c->getInt (&configContext));
  } else {
    readBinary ((u8**)&info, c->getString ("info"));

    if (c->findID ("foreground", &configContext)) {
      tilessizeFG = readBinary (&tilesFG, c->getString (&configContext));
      mapsize = readBinary ((u8**)&mapFG, c->getString (&configContext));
      paletteSizeFG = readBinary ((u8**)&paletteFG,
                                  c->getString (&configContext));
      foundFG = true;
    }
    if (c->findID ("background", &configContext)) {
      tilessizeBG = readBinary (&tilesBG, c->getString (&configContext));
      mapsize = readBinary ((u8**)&mapBG, c->getString (&configContext));
      paletteSizeBG = readBinary ((u8**)&paletteBG,
                                  c->getString (&configContext));
      foundBG = true;
    } 
    if (!foundBG && !foundFG) panic ("Parse error: background");

    if (c->findID ("collision", &configContext)) {
      foundCM = true;
      collisionTilessize = readBinary ((u8**)&collisionTiles,
                                       c->getString (&configContext));
      readBinary ((u8**)&collisionMap, c->getString (&configContext));
    } else {
      panic ("Parse error: collision");
    }
  }

  if (c->findID ("start", &configContext)) {
    do {
      RRPoint p;
      p.x = c->getInt (&configContext);
      p.y = c->getInt (&configContext);
      startingPoints.push_back (p);
    } while (c->next (&configContext));
  }

  // create SOB bg
  foundSOB = false;
  mapSOB = (u16*)calloc (1, mapsize);
  if (!mapSOB) panic ("Out of memory");
  tilessizeSOB = 0;
  paletteSizeSOB = 0;
  Sob::initSOBs (&tilesSOB, &paletteSOB, &tilessizeSOB, &paletteSizeSOB);

  bool markerFound = false;
  if (c->exists ("sentrygun") || c->exists ("sentry") || retrorocket->rtsMode) {
    foundSOB = true;
    SentryGun::copyTiles (&tilesSOB, &paletteSOB,
                          &tilessizeSOB, &paletteSizeSOB);
    StrengthBar::copyTiles (&tilesSOB, &paletteSOB,
                         &tilessizeSOB, &paletteSizeSOB);
    RRMarker::copyTiles (&tilesSOB, &paletteSOB,
                         &tilessizeSOB, &paletteSizeSOB);
    markerFound = true;
  }

  if (c->findID ("fuel", &configContext)) {
    foundSOB = true;
    Fuel::copyTiles (&tilesSOB, &paletteSOB, &tilessizeSOB, &paletteSizeSOB);
    do {
      int x = c->getInt (&configContext);
      int y = c->getInt (&configContext);
      new Fuel (x, y);
    } while (c->next (&configContext));
  }

  if (c->findID ("sentry", &configContext)) {
    foundSOB = true;
    do {
      int x = c->getInt (&configContext);
      int y = c->getInt (&configContext);
      Fix32 angle = c->getFloat (&configContext);
      new SentryGun (x, y, angle, SentryGun::RANDOM);
    } while (c->next (&configContext));
  }

  if (c->findID ("door", &configContext)) {
    foundSOB = true;
    Door::copyTiles (&tilesSOB, &paletteSOB, &tilessizeSOB, &paletteSizeSOB);
    do {
      int x = c->getInt (&configContext);
      int y = c->getInt (&configContext);
      int x2 = c->getInt (&configContext);
      int y2 = c->getInt (&configContext);
      new Door (x, y, x2, y2);
    } while (c->next (&configContext));
  }

  if (c->findID ("powerplant", &configContext)) {
    foundSOB = true;
    PowerPlant::copyTiles (&tilesSOB, &paletteSOB,
                           &tilessizeSOB, &paletteSizeSOB);
    do {
      int x = c->getInt (&configContext);
      int y = c->getInt (&configContext);
      new PowerPlant (x, y);
    } while (c->next (&configContext));
  }

  if (c->findID ("resource", &configContext)) {
    foundSOB = true;
    Resource::copyTiles (&tilesSOB, &paletteSOB,
                         &tilessizeSOB, &paletteSizeSOB);
    ResourceNode::copyTiles (&tilesSOB, &paletteSOB,
                         &tilessizeSOB, &paletteSizeSOB);
    do {
      int x = c->getInt (&configContext);
      int y = c->getInt (&configContext);
      new Resource (x, y);
    } while (c->next (&configContext));
  }

  if (c->findID ("ball", &configContext)) {
    foundSOB = true;
    BallMount::copyTiles (&tilesSOB, &paletteSOB,
                          &tilessizeSOB, &paletteSizeSOB);
    do {
      int x = c->getInt (&configContext) & ~7;
      int y = c->getInt (&configContext) & ~7;
      Ball *ball = new Ball (x, y + 3, retrorocket->balls.size(),
                             c->getString (&configContext));
      retrorocket->balls.push_back (ball);
    } while (c->next (&configContext));
  }

  if (c->findID ("ballpoint", &configContext)) {
    do {
      RRPoint p;
      p.x = c->getInt (&configContext) ;
      p.y = c->getInt (&configContext) ;
      retrorocket->ballpoints.push_back (p);
    } while (c->next (&configContext));
  }

  if (c->findID ("buttonright", &configContext)) {
    foundSOB = true;
    ButtonRight::copyTiles (&tilesSOB, &paletteSOB,
                            &tilessizeSOB, &paletteSizeSOB);
    do {
      int x = c->getInt (&configContext);
      int y = c->getInt (&configContext);
      new ButtonRight (x, y);
    } while (c->next (&configContext));
  }

  if (c->findID ("buttonleft", &configContext)) {
    foundSOB = true;
    ButtonLeft::copyTiles (&tilesSOB, &paletteSOB,
                           &tilessizeSOB, &paletteSizeSOB);
    do {
      int x = c->getInt (&configContext);
      int y = c->getInt (&configContext);
      new ButtonLeft (x, y);
    } while (c->next (&configContext));
  }

  bool foundGun = false;

  if (c->findID ("gunne", &configContext)) {
    foundSOB = true;
    foundGun = true;
    GunNE::copyTiles (&tilesSOB, &paletteSOB, &tilessizeSOB, &paletteSizeSOB);
    do {
      int x = c->getInt (&configContext);
      int y = c->getInt (&configContext);
      new GunNE (x, y);
    } while (c->next (&configContext));
  }

  if (c->findID ("gunnw", &configContext)) {
    foundSOB = true;
    foundGun = true;
    GunNW::copyTiles (&tilesSOB, &paletteSOB, &tilessizeSOB, &paletteSizeSOB);
    do {
      int x = c->getInt (&configContext);
      int y = c->getInt (&configContext);
      new GunNW (x, y);
    } while (c->next (&configContext));
  }

  if (c->findID ("gunse", &configContext)) {
    foundSOB = true;
    foundGun = true;
    GunSE::copyTiles (&tilesSOB, &paletteSOB, &tilessizeSOB, &paletteSizeSOB);
    do {
      int x = c->getInt (&configContext);
      int y = c->getInt (&configContext);
      new GunSE (x, y);
    } while (c->next (&configContext));
  }

  if (c->findID ("gunsw", &configContext)) {
    foundSOB = true;
    foundGun = true;
    GunSW::copyTiles (&tilesSOB, &paletteSOB, &tilessizeSOB, &paletteSizeSOB);
    do {
      int x = c->getInt (&configContext);
      int y = c->getInt (&configContext);
      new GunSW (x, y);
    } while (c->next (&configContext));
  }

  if (foundGun && !c->exists ("powerplant")) {
    panic ("Parse error: powerplant");
  }

  bool foundPlatformSOB = false;
  if (c->findID ("platform", &configContext)) {
    do {
      Platform p;

      p.x1 = c->getInt (&configContext);
      p.y = c->getInt (&configContext);
      p.x2 = (Fix32)c->getInt (&configContext) + p.x1;
      p.properties = 0;
      p.homeShip = ~0;

      while (c->moreEls (&configContext)) {
        switch (c->getChar (&configContext)) {
          case 'h':
            p.homeShip = c->getInt (&configContext);
            break;
          case 'g':
            p.properties |= FILLINGSTATION;
            break;
          case 'a':
            p.properties |= WEAPONA;
            break;
          case 'b':
            p.properties |= WEAPONB;
            break;
          case 'r':
            p.properties |= GARAGE;
            break;
          case 's':
            if (!foundPlatformSOB) {
              if (!markerFound) {
                RRMarker::copyTiles (&tilesSOB, &paletteSOB,
                                   &tilessizeSOB, &paletteSizeSOB);
                markerFound = true;
              } 
              foundSOB = true;
              PlatformSOB::copyTiles (&tilesSOB, &paletteSOB,
                                      &tilessizeSOB, &paletteSizeSOB);
              foundPlatformSOB = true;
            }
            p.x1 &= ~0x07;
            p.x2 = p.x1 + PlatformSOB::platformWidth();
            p.y &= ~0x07;
            new PlatformSOB (p.x1, p.y, p.homeShip);
            break;
        }
      }
      platforms.push_back (p);
    } while (c->next (&configContext));
  }

  if (c->exists ("score")) {
    score = c->getInt ("score");
  } else {
    score = 0;
  }

  if (c->findID ("zone", &configContext)) {
    do {
      RRZone z;

      z.x = c->getInt (&configContext);
      z.y = c->getInt (&configContext);
      z.w = c->getInt (&configContext);
      z.h = c->getInt (&configContext);
      z.forceX = c->getFloat (&configContext);
      z.forceY = c->getFloat (&configContext);
      z.properties = 0;

      while (c->moreEls (&configContext)) {
        switch (c->getChar (&configContext)) {
          case 'g':
            z.properties |= GRAVITYFIELD;
            break;
          case 'l':
            z.properties |= LIQUID;
            break;
        }
      }
      zones.push_back (z);
    } while (c->next (&configContext));
  }

  if (c->findID ("waypoint", &configContext)) {
    do {
      WayPoint w;
      
      w.x = c->getInt (&configContext);
      w.y = c->getInt (&configContext);
      w.length = c->getInt (&configContext);
      w.direction = UNIDIRECTIONAL;
      
      while (c->moreEls (&configContext)) {
        switch (c->getChar (&configContext)) {
          case 'h':
            w.horizontal = true;
            break;
          case 'v':
            w.horizontal = false;;
            break;
          case 'u' :
            w.direction = UPWARDS;
            break;
          case 'd' :
            w.direction = DOWNWARDS;
            break;
          case 'l' :
            w.direction = LEFTWARDS;
            break;
          case 'r' :
            w.direction = RIGHTWARDS;
            break;
        }
      }
      wayPoints.push_back (w);
    } while (c->next (&configContext));
  }

  numberWayPoints = wayPoints.size();
  
  if (foundFG) {
    loadBg (info, mapFG, mapsize, tilesFG, tilessizeFG, paletteFG, FRONT_BG);
  }
  if (foundBG) {
    loadBg (info, mapBG, mapsize, tilesBG, tilessizeBG, paletteBG, BACK_BG);
  }
  if (foundSOB) {
    loadBg (info, mapSOB, mapsize, tilesSOB, tilessizeSOB, paletteSOB, SOB_BG);
  } else {
    free (mapSOB);
    free (tilesSOB);
    free (paletteSOB);
  }

  drawMap (collisionMap, collisionTiles, info, width, height,
           0, 0, SCREEN_WIDTH, SCREEN_HEIGHT-32);

  findBlankTile();
}

Field::~Field() {
  if (foundBM) {
    free (bmBuffer);
  }

  if (foundCM) {
    free (collisionMap);
    free (collisionTiles);
  }
  
  if (foundFG) {
    deleteBg (FRONT_BG);
    free (mapFG);
    free (tilesFG);
    free (paletteFG);
  }

  if (foundBG) {
    deleteBg (BACK_BG);
    free (mapBG);
    free (tilesBG);
    free (paletteBG);
  }

  if (foundSOB) {
    deleteBg (SOB_BG);
    free (mapSOB);
    free (tilesSOB);
    free (paletteSOB);
  }

  free (info);
}

int Field::landed (Ship *ship) {
  Fix32 base_x = ship->x;
  Fix32 base_y = ship->y + SPRITESIZE;

  // TODO: endre til pixelnøyaktig kollisjonsdeteksjon?
  // isåfall, endre også getStart() tilsvarende

  for (int i = 0; i < (int)platforms.size(); i++) {
    if ((base_y >= platforms[i].y) &&
        (base_y < platforms[i].y + 10) &&
        (base_x > platforms[i].x1) &&
        (base_x < (platforms[i].x2 - SPRITESIZE)) &&
        (ship->orientation < (DEG270 + DEG22)) &&
        (ship->orientation > (DEG270 - DEG22))) {
      if (platforms[i].homeShip == ship->id) {
        return platforms[i].properties | HOME | LANDED;
      }
      return platforms[i].properties | LANDED;
    }
  }
  return 0;
}

ZoneSum Field::getZoneSum (Mob *mob) {
  
  // TODO: endre til senterpixelsjekk
  ZoneSum sum;
  sum.properties = 0;
  sum.forceX = 0;
  sum.forceY = 0;
  
  Fix32 base_x = mob->x + SPRITESIZE/2;
  Fix32 base_y = mob->y + SPRITESIZE/2;

  for (int i = 0; i < (int)zones.size(); i++) {
    if ((base_y >= zones[i].y) &&
        (base_y < zones[i].y+zones[i].h) &&
        (base_x >= zones[i].x) &&
        (base_x < zones[i].x+zones[i].w)) {
      sum.properties |= zones[i].properties;
      sum.forceX += zones[i].forceX;
      sum.forceY += zones[i].forceY;
    }
  }
  return sum;
}

int Field::getWayPoint (Ship *ship) {

  // TODO: sjekk mot senterpixel basert på linje mellom denne og
  //       forrige posisjon
  //       ta også hensyn til direction
  Fix32 x1 = ship->prevX;
  Fix32 y1 = ship->prevY;
  Fix32 x2 = ship->getCenterX();
  Fix32 y2 = ship->getCenterY();

  for (int i = 0; i < (int)wayPoints.size(); i++) {
    if (wayPoints[i].horizontal) {
      if ( (wayPoints[i].x < x1) &&
           (x1 <= (wayPoints[i].x + wayPoints[i].length)) &&
           ( wayPoints[i].x < x2 ) &&
           (x2 <= (wayPoints[i].x + wayPoints[i].length)) ) {
        switch (wayPoints[i].direction) {
          case UPWARDS :
            if (( y1 > wayPoints[i].y ) && ( wayPoints[i].y >= y2 ))
              return i+1;
            break;
          case DOWNWARDS :
            if (( y2 > wayPoints[i].y) && ( wayPoints[i].y >= y1 ))
              return i+1;
            break;
          default :
          case UNIDIRECTIONAL :
            if ((( y1 > wayPoints[i].y)&&( wayPoints[i].y >= y2)) ||
                (( y2 > wayPoints[i].y)&&( wayPoints[i].y >= y1 )))
              return i+1;
            break;
        }
      }
    } else { // vertical
      if ( (wayPoints[i].y < y1) &&
           (y1 <= (wayPoints[i].y + wayPoints[i].length)) &&
           ( wayPoints[i].y < y2 ) &&
           (y2 <= (wayPoints[i].y + wayPoints[i].length)) ) {
        switch (wayPoints[i].direction) {
          case LEFTWARDS :
            if (( x1 > wayPoints[i].x ) && ( wayPoints[i].x >= x2 ))
              return i+1;
            break;
          case RIGHTWARDS :
            if (( x2 > wayPoints[i].x) && ( wayPoints[i].x >= x1 ))
              return i+1;
            break;
          default :
          case UNIDIRECTIONAL :
            if ((( x1 > wayPoints[i].x)&&( wayPoints[i].x >= x2)) ||
                (( x2 > wayPoints[i].x)&&( wayPoints[i].x >= x1 )))
              return i+1;
            break;
        }
      }
    }
  }
  return 0;
} 

RRPoint Field::getStart (unsigned int id) {
  Ship *ship = retrorocket->ships[id];
  RRPoint start = {-1, -1};

  // use a home platform if present
  for (int i = 0; i < (int)platforms.size(); i++) {
    Platform p = platforms[i];

    if (p.homeShip == id) {
      start.x = p.x1 + ((p.x2 - p.x1) / 2) - SPRITESIZE/2;
      start.y = p.y-SPRITESIZE;
      return start;
    }
  }

  // see if there are starting points
  if (startingPoints.size()) {
    Fix32 minYDistance = 65536;
    int min = 0;

    for (unsigned int i = 0; i < startingPoints.size(); i++) {
      if (ship->y >= startingPoints[i].y) {
        Fix32 yDistance = (startingPoints[i].y - ship->y).abs();
        if (yDistance < minYDistance) {
          minYDistance = yDistance;
          min = i;
        }
      }
    }    
    return startingPoints[min];
  }
  
  panic ("No starting point");
  return start;
}


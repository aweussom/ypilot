/*
  system_qt4.h
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

#ifndef SYSTEM_QT_H
#define SYSTEM_QT_H

#include <QObject>
#include <QtGui>
#include <QObject>

#include "retrorocket.h"
#include "botrocket.h"
// #include "bot.h"
///////////////////////////////////////////////////////////////////////////////

class BSDSocket : public Network {
  
  int32_t serverSocket;

  int32_t *slaveSocketsWrite;
  int32_t masterSocketWrite;

  int32_t *slaveSocketsRead;
  int32_t masterSocketRead;

protected:
  void sendPacketToAll (Packet *packet);
  void sendPacketToMaster (Packet *packet);
  void sendPacketToUser (int userid, Packet *packet);
  void sendPacketBroadcast (Packet *packet);

public:
  BSDSocket();
  ~BSDSocket();
  bool init (bool isMaster, char *fieldFilename, int min, int max,
             int aiPlayers);
  void advance();
};

///////////////////////////////////////////////////////////////////////////////

class RetrorocketWindow : public QMainWindow {

  Q_OBJECT

  QAction *startAction;
  RetroRocket * botrocket;
public slots:
  void start();
  void quit();
  void advance();

public:
  int advanceCounter;

  RetrorocketWindow (int checkahead, int maxsteps, QWidget *parent = 0);

  void closeEvent (QCloseEvent *event);
  void keyPressEvent (QKeyEvent *event);
  void keyReleaseEvent (QKeyEvent *event);

  void waitVBL();

};

///////////////////////////////////////////////////////////////////////////////

class FieldBox : public QGraphicsView {

private:
  QGraphicsScene *scene;

public:
  QGraphicsPixmapItem *background;
  QGraphicsPixmapItem *sob;
  QImage sobImage;
  QGraphicsPixmapItem *foreground;

  QGraphicsPixmapItem *sprites[MAX_SPRITES];

  FieldBox (QWidget *parent = 0);
  ~FieldBox();
  void loadBg (QPixmap pm, int bg);
  void createSprite (QPixmap pm, int spritenum);
  void moveSprite (int x, int y, int spritenum);
  void rotateSprite (int spritenum, Fix32 angle);
  void setSOBTile (int x, int y, int tile, int pal);
};

///////////////////////////////////////////////////////////////////////////////

class MapBox : public QWidget {
  QPixmap pixmap;
  bool newClick;
  int clickX, clickY;

public:
  int fieldWidth, fieldHeight;
  int mapWidth, mapHeight;

  MapBox (QWidget *parent = 0);
  void drawMap (u8 *buffer, int width, int height, int threshold);
  void paintEvent (QPaintEvent *event);
  void mousePressEvent (QMouseEvent *event);
  bool click (int *x, int *y);
  void resetClick() { newClick = false; }
};

///////////////////////////////////////////////////////////////////////////////

class TextBox : public QWidget {

  QPainter painter;
  QPixmap *pixmap;
  QFont monoFont;
  int charWidth;
  int charHeight;

public:
  QPen currentPen;
  vector<QRect> updates;

  TextBox (QWidget *parent = 0);

  void paintEvent (QPaintEvent *event);

  void outputText (int x, int y, const char *str);
  void clearText (int from, int to);
};

///////////////////////////////////////////////////////////////////////////////

#endif

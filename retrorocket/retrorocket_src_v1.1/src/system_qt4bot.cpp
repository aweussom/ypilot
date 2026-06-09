/*
  system_qt4.cpp
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
// System code for qt4 on Unix
// Used for debugging purposes
//
///////////////////////////////////////////////////////////////////////////////

#include "system_qt4bot.h"
#include <pwd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define TIMER_PERIOD 17
#define ACTION_UPDATE_RATE 1
#define TEXT_UPDATE_RATE 10

#define PORT      32000

char serverIP[16];

///////////////////////////////////////////////////////////////////////////////

FieldBox *fieldBox;
MapBox *mapBox;
TextBox *textBox;
unsigned int keysPressed;
Ship *focus;
RetrorocketWindow *window;

BSDSocket *bsdsocket;

///////////////////////////////////////////////////////////////////////////////

void initNet() {
}

Network *openNetwork (bool isMaster, char *fieldFilename, int min, int max,
                      int aiPlayers, bool autogen) {
  // bsdsocket = new BSDSocket();
//   bsdsocket->autogen = autogen;
//   if (bsdsocket->init (isMaster, fieldFilename, min, max, aiPlayers)) {
//     if (bsdsocket->numberOfShips - bsdsocket->aiPlayers == 1) {
      NoNetwork *nn = new NoNetwork;
      nn->numberOfShips = max;
      nn->aiPlayers = aiPlayers;
//       delete bsdsocket;
      return nn;
//     }
//   } else {
//     delete bsdsocket;
//     return NULL;
//   }
//   return bsdsocket;
}

BSDSocket::BSDSocket() {}

BSDSocket::~BSDSocket() {
  if (slaveSocketsRead && slaveSocketsWrite) {
    if (master) {
      for (int i = 0; i < numberOfShips-1; i++) {
        shutdown (slaveSocketsRead[i], 2);
        shutdown (slaveSocketsWrite[i], 2);

        close (slaveSocketsRead[i]);
        close (slaveSocketsWrite[i]);
      }
      free (slaveSocketsRead);
      free (slaveSocketsWrite);
    }
  }
}

bool start;
void waitForPlayers (Menu *menu) {
  start = true;
}
bool BSDSocket::init (bool isMaster, char *fieldFilename, int min, int max, int aiPlayers) {
  this->fieldFilename = fieldFilename;

  this->aiPlayers = aiPlayers;

  slaveSocketsRead = NULL;
  slaveSocketsWrite = NULL;

  if (isMaster) {
    start = false;

    Menu netMenu ("Players");
    int players = 0;
    netMenu.setBack();
    netMenu.addItem ("Number: ", min, max-aiPlayers, &players);
    netMenu.addItem ("Start", waitForPlayers);
    netMenu.presentMenuAsync();
    while (!start && !netMenu.isDone()) {
      netMenu.advance();
      waitVBL();
    }
    if (!start) return false;

    retrorocket->localShip = 0;
    numberOfShips = players+aiPlayers;
    printf ("DB: %d\n", numberOfShips);
    master = true;

    if (players > 1) {
      clearText();
      infoText ("Waiting...");

      serverSocket = socket (PF_INET, SOCK_STREAM, 0);

      if(serverSocket == -1) {
        panic ("can not create socket");
      }

      struct sockaddr_in sockAddr;
      bzero(&sockAddr, sizeof(sockAddr));
 
      sockAddr.sin_family = AF_INET;
      sockAddr.sin_port = htons (PORT);
      sockAddr.sin_addr.s_addr = htonl (INADDR_ANY);
 
      if (bind (serverSocket,
                (struct sockaddr*)&sockAddr,
                sizeof(sockAddr)) == -1) {
        panic ("error bind failed %d", errno);
      }
 
      if (listen (serverSocket, 10) == -1) {
        panic ("error listen failed");
      }
 
      slaveSocketsRead = (int32_t*)malloc ((players-1) * sizeof (int32_t));
      slaveSocketsWrite = (int32_t*)malloc ((players-1) * sizeof (int32_t));

      for (int i = 1; i < players; i++) {
        slaveSocketsWrite[i-1] = accept (serverSocket, NULL, NULL);
 
        if(slaveSocketsWrite[i-1] < 0) {
          panic ("error accept failed");
        }

        // get non-blocking fd
        if ((slaveSocketsRead[i-1] = dup (slaveSocketsWrite[i-1])) < 0) {
          panic ("dup %d", errno);
        }

        if (fcntl (slaveSocketsRead[i-1], F_SETFL, O_NONBLOCK) < 0) {
          panic ("fcntl failed");
        }

        write (slaveSocketsWrite[i-1], &i, sizeof (int));
      }

      fprintf (stderr, "DB1");

      sendNewGamePacket (players, aiPlayers, autogen, fieldFilename);
    }
  } else {

    master = false;

    masterSocketWrite = socket(PF_INET, SOCK_STREAM, 0);
 
    if (masterSocketWrite == -1) {
      panic ("cannot create socket");
    }
 
    struct sockaddr_in sockAddr;
    bzero (&sockAddr, sizeof(sockAddr));

    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(PORT);
    int32_t error = inet_pton(AF_INET, serverIP,
                              (void *) &sockAddr.sin_addr);
 
    if (error <= 0) {
      panic ("address error");
    }
 
    if (connect (masterSocketWrite,
                 (struct sockaddr*)&sockAddr,
                 sizeof(sockAddr)) == -1) {
      panic ("connect failed");
    }

    // receive init data
    int readSize = read (masterSocketWrite,
                         &retrorocket->localShip, sizeof (int));
    if (readSize < (int)sizeof (int)) {
      panic ("read %d %d", sizeof (int), readSize);
    }

    // get non-blocking fd
    if ((masterSocketRead = dup (masterSocketWrite)) < 0) {
      panic ("dup");
    }
    if (fcntl (masterSocketRead, F_SETFL, O_NONBLOCK) < 0) {
      panic ("fcntl failed");
    }

    newGamePacketReceived = false;
    while (!checkForNewGamePacket()) {
      this->advance();
      waitVBL();
    }

  }

  return true;
}

int packetCounter = 0;

void BSDSocket::sendPacketBroadcast (Packet *packet) {
  if (master) {
    int size = packet->size();
    for (int i = 1; i < numberOfShips-aiPlayers; i++) {
      if (write (slaveSocketsWrite[i-1], &size, sizeof (int)) <= 0) {
        panic ("write");
      }
      if (write (slaveSocketsWrite[i-1], packet, packet->size()) <= 0) {
        panic ("write");
      }
    }
  } else {
    int all = -1;
    int size = packet->size();
    if (write (masterSocketWrite, &all, sizeof (int)) <= 0) panic ("write");
    if (write (masterSocketWrite, &size, sizeof (int)) <= 0) panic ("write");
    if (write (masterSocketWrite, packet, packet->size()) <= 0) panic ("write");
  }
}

void BSDSocket::sendPacketToAll (Packet *packet) {
  if (master) {
    int size = packet->size();
    for (int i = 1; i < numberOfShips-aiPlayers; i++) {
      fprintf (stderr, "DB5 %d %d %d\n", numberOfShips, aiPlayers, i);
      if (write (slaveSocketsWrite[i-1], &size, sizeof (int)) <= 0) {
        fprintf (stderr, "DB6");
        panic ("write");
      }
      if (write (slaveSocketsWrite[i-1], packet, packet->size()) <= 0) {
        fprintf (stderr, "DB7");
        panic ("write");
      }
    }
  } else {
    int all = -1;
    int size = packet->size();
    if (write (masterSocketWrite, &all, sizeof (int)) <= 0) panic ("write");
    if (write (masterSocketWrite, &size, sizeof (int)) <= 0) panic ("write");
    if (write (masterSocketWrite, packet, packet->size()) <= 0) panic ("write");
  }
}

void BSDSocket::sendPacketToUser (int userid, Packet *packet) {
  printf ("packet %d type %d:\n", packetCounter++, packet->packettype);

  if (master) {
    int size = packet->size();
    if (write (slaveSocketsWrite[userid-1], &size, sizeof (int)) <= 0) {
      panic ("write");
    }
    if (write (slaveSocketsWrite[userid-1], packet, packet->size()) <= 0) {
      panic ("write");
    }
  } else {
    int all = userid;
    int size = packet->size();
    if (write (masterSocketWrite, &all, sizeof (int)) <= 0) panic ("write");
    if (write (masterSocketWrite, &size, sizeof (int)) <= 0) panic ("write");
    if (write (masterSocketWrite, packet, packet->size()) <= 0) panic ("write");
  }
}

void BSDSocket::sendPacketToMaster (Packet *packet) {
  printf ("packet %d type %d:\n", packetCounter++, packet->packettype);

  if (!master) {
    int all = 0;
    int size = packet->size();
    if (write (masterSocketWrite, &all, sizeof (int)) <= 0) panic ("write");
    if (write (masterSocketWrite, &size, sizeof (int)) <= 0) panic ("write");
    if (write (masterSocketWrite, packet, packet->size()) <= 0) panic ("write");
  }
}

void BSDSocket::advance() {
  if (master) {
    int all;
    int size;
    for (int i = 1; i < numberOfShips; i++) {
      if (read (slaveSocketsRead[i-1], &all, sizeof (int)) > 0) {
        // data available
        int readSize;
        do {
          readSize = read (slaveSocketsWrite[i-1], &size, sizeof (int));
        } while (readSize < 0);
        if (readSize < (int)sizeof (int)) {
          panic ("read A %d %d %d", sizeof (int), readSize, errno);
        }
        Packet *packet = (Packet *)malloc (size);
        do {
          readSize = read (slaveSocketsWrite[i-1], packet, size);
        } while (readSize < 0);
        if (readSize < size) {
          panic ("read B %d %d %d", size, readSize, errno);
        }
        packetQueue.push_back (packet);
        if (all == -1) {
          // send back to all other users
          for (int j = 1; j < numberOfShips; j++) {
            if (j != i) {
              if (write (slaveSocketsWrite[j-1], &size, sizeof (int)) <= 0) {
                panic ("write");
              }
              if (write (slaveSocketsWrite[j-1], packet, packet->size()) <= 0) {
                panic ("write");
              }
            }
          }
        } else if (all != 0) {
          // send to correct user
          if (write (slaveSocketsWrite[all-1], &size, sizeof (int)) <= 0) {
            panic ("write");
          }
          if (write (slaveSocketsWrite[all-1], packet, packet->size()) <= 0) {
            panic ("write");
          }
        }
      }
    }
  } else {
    int size;
    if (read (masterSocketRead, &size, sizeof (int)) > 0) {
      // data available
      Packet *packet = (Packet *)malloc (size);

      int s;
      do {
        s = read (masterSocketWrite, packet, size);
      } while (s < 0);
      if (s < size) panic ("read S %d %d %d", size, s, errno);

      packetQueue.push_back (packet);
    }
  }

  Network::advance();
}

///////////////////////////////////////////////////////////////////////////////

RetrorocketWindow::RetrorocketWindow (int checkahead, int maxsteps, 
				      QWidget *parent) : QMainWindow (parent) {
  fieldBox = new FieldBox();
  mapBox = new MapBox();
  textBox = new TextBox();

  QMenu *gameMenu = menuBar()->addMenu (tr("&Game"));
  startAction = gameMenu->addAction(tr("&Start"));
  QAction *quitAction = gameMenu->addAction(tr("&Quit"));
  connect (startAction, SIGNAL(triggered()), this, SLOT(start()));
  connect (quitAction, SIGNAL(triggered()), this, SLOT(quit()));

  QGroupBox *ver = new QGroupBox();
  QGroupBox *hor = new QGroupBox();

  QHBoxLayout *horLayout = new QHBoxLayout;
  QVBoxLayout *verLayout = new QVBoxLayout;
  verLayout->addWidget (fieldBox);
  verLayout->addWidget (mapBox);
  ver->setLayout (verLayout);
  horLayout->addWidget (ver);
  horLayout->addWidget (textBox);
  hor->setLayout (horLayout);

  setCentralWidget (hor);

  grabKeyboard();

  advanceCounter = 0;
  focus = NULL;

  if (chdir ("./"DATADIR)) {
    printf ("Panic: Datadir not present\n");
    exit (1);
  }

  botrocket = new BotRocket(checkahead,maxsteps);
  retrorocket = botrocket;
}

void RetrorocketWindow::closeEvent (QCloseEvent *event) {
  quit();
}

FieldBox::FieldBox (QWidget *parent) : QGraphicsView (parent) {
  setFixedSize (SCREEN_WIDTH, SCREEN_HEIGHT);
  scene = new QGraphicsScene();
  setScene (scene);
  setInteractive (false);
  for (int i = 0; i < MAX_SPRITES; i++) {
    sprites[i] = NULL;
  }
  background = NULL;
  foreground = NULL;
  sob = NULL;
  setBackgroundBrush (QBrush (QColor ("#000000")));
  setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOff);

  setViewportUpdateMode (QGraphicsView::NoViewportUpdate);
  setRenderHints (0);
}

FieldBox::~FieldBox() {
  delete scene;
}

void FieldBox::loadBg (QPixmap pm, int bg) {
  switch (bg) {
    case BACK_BG:
      background = scene->addPixmap (pm);
      background->setPos (0, 0);
      background->setVisible (true);
      background->setZValue (0);
      break;
    case FRONT_BG:
      foreground = scene->addPixmap (pm);
      foreground->setPos (0, 0);
      foreground->setVisible (true);
      foreground->setZValue (3);
      break;
    case SOB_BG:
      sob = scene->addPixmap (pm);
      sob->setPos (0, 0);
      sob->setVisible (true);
      sob->setZValue (1);
      break;
  }
}

void FieldBox::createSprite (QPixmap pm, int spritenum) {
  sprites[spritenum] = scene->addPixmap (pm);
  sprites[spritenum]->setPos (0, 0);
  sprites[spritenum]->setVisible (true);
  sprites[spritenum]->setZValue (2);
}

void FieldBox::moveSprite (int x, int y, int spritenum) {
  sprites[spritenum]->setPos (x, y);
}

void FieldBox::rotateSprite (int spritenum, Fix32 angle) {
  sprites[spritenum]->setTransform (QTransform().
                                    translate (SPRITESIZE/2, SPRITESIZE/2).
                                    rotate ((angle.toFloat()/DEG360)*360).
                                    translate(-SPRITESIZE/2, -SPRITESIZE/2));
}

void RetrorocketWindow::start() {
  botrocket->start();
  QTimer *timer = new QTimer (this);
  connect (timer, SIGNAL(timeout()), this, SLOT(advance()));
  timer->start (TIMER_PERIOD);

  startAction->setEnabled (false);


}

void RetrorocketWindow::quit() {
  delete fieldBox;
  delete mapBox;
  delete textBox;
  delete retrorocket;
  exit (0);
}

void RetrorocketWindow::waitVBL() {
  advanceCounter++;

  // update action
  if ((advanceCounter % ACTION_UPDATE_RATE) == 0) {
    if (focus) {
      fieldBox->centerOn (focus->x.toInt(), focus->y.toInt());
    }
    fieldBox->viewport()->update();
  }

  // update text
  if ((advanceCounter % TEXT_UPDATE_RATE) == 0) {
    for (unsigned int i = 0; i < textBox->updates.size(); i++) {
      textBox->update (textBox->updates[i]);
    }
    textBox->updates.clear();
  }  

  QCoreApplication::processEvents();  
  usleep (TIMER_PERIOD*1000);
}

void RetrorocketWindow::advance() {
  advanceCounter++;

  // update action
  if ((advanceCounter % ACTION_UPDATE_RATE) == 0) {
    if (focus) {
      fieldBox->centerOn (focus->x.toInt(), focus->y.toInt());
    }
    fieldBox->viewport()->update();
  }

  // update text
  if ((advanceCounter % TEXT_UPDATE_RATE) == 0) {
    for (unsigned int i = 0; i < textBox->updates.size(); i++) {
      textBox->update (textBox->updates[i]);
    }
    textBox->updates.clear();
  }  

  botrocket->advance();
}

///////////////////////////////////////////////////////////////////////////////

MapBox::MapBox (QWidget *parent) : QWidget(parent) {
  setFixedSize (SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
  newClick = false;
//   hide();
}

void MapBox::drawMap (u8 *buffer, int width, int height, int threshold) {
  fieldWidth = width;
  fieldHeight = height;
  QImage image = QImage (width, height, QImage::Format_ARGB32);
  QRgb on = qRgb(80, 80, 0);
  QRgb off = qRgb(0, 0, 0);

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      if (buffer[y*width+x] > threshold) {
        image.setPixel (x, y, on);
      } else {
        image.setPixel (x, y, off);
      }
    }
  }
  pixmap = QPixmap::fromImage (image);
  pixmap = pixmap.scaled (SCREEN_WIDTH/2, SCREEN_HEIGHT/2, Qt::KeepAspectRatio);
  mapWidth = pixmap.width();
  mapHeight = pixmap.height();
  update();
}

void MapBox::paintEvent (QPaintEvent *event) {
  QPainter painter (this);
  painter.drawPixmap (QPointF (0, 0), pixmap);
}

void MapBox::mousePressEvent (QMouseEvent *event) {
  newClick = true;
  clickX = event->x();
  clickY = event->y();
}

bool MapBox::click (int *x, int *y) {
  if (newClick) {
    *x = clickX;
    *y = clickY;
    newClick = false;
    return true;
  } else {
    return false;
  }
}

///////////////////////////////////////////////////////////////////////////////

TextBox::TextBox (QWidget *parent): QWidget(parent), monoFont ("Courier") {
  monoFont.setStyleHint (QFont::TypeWriter);
  monoFont.setFixedPitch (true);
  monoFont.setPointSize (8);

  QFontMetrics fm (monoFont);
  charWidth = fm.width ("X");
  charHeight = fm.height() - 3;

  pixmap = new QPixmap (COLUMNS*charWidth, ROWS*charHeight + 1);

  painter.begin (pixmap);
  painter.setFont (monoFont);
  painter.setBrush (QBrush (QColor ("#000000")));

  clearText (0, ROWS);

  setFixedSize (COLUMNS * charWidth, ROWS * charHeight + 1);
}

void TextBox::paintEvent (QPaintEvent *event) {
  QRect rect = event->rect();
  QPainter screenPainter (this);
  screenPainter.drawPixmap (rect, *pixmap, rect);
}

void RetrorocketWindow::keyPressEvent (QKeyEvent *event) {
  if (!event->isAutoRepeat()) {
    switch (event->key()) {
      case Qt::Key_Return:
        keysPressed |= START;
        break;
      case Qt::Key_Backspace:
        keysPressed |= SELECT;
        break;
      case Qt::Key_Left:
        keysPressed |= LEFT;
        break;
      case Qt::Key_Right:
        keysPressed |= RIGHT;
        break;
      case Qt::Key_Up:
        keysPressed |= UP;
        break;
      case Qt::Key_Down:
        keysPressed |= DOWN;
        break;
      case Qt::Key_L:
        keysPressed |= L;
        break;
      case Qt::Key_R:
        keysPressed |= R;
        break;
      case Qt::Key_A:
        keysPressed |= A;
        break;
      case Qt::Key_B:
        keysPressed |= B;
        break;
      case Qt::Key_X:
        keysPressed |= X;
        break;
      case Qt::Key_Y:
        keysPressed |= Y;
        break;
      default:
        event->ignore();
        break;
    }
  }
}

void RetrorocketWindow::keyReleaseEvent (QKeyEvent *event) {
  if (!event->isAutoRepeat()) {
    switch (event->key()) {
      case Qt::Key_Return:
        keysPressed &= ~START;
        break;
      case Qt::Key_Backspace:
        keysPressed &= ~SELECT;
        break;
      case Qt::Key_Left:
        keysPressed &= ~LEFT;
        break;
      case Qt::Key_Right:
        keysPressed &= ~RIGHT;
        break;
      case Qt::Key_Up:
        keysPressed &= ~UP;
        break;
      case Qt::Key_Down:
        keysPressed &= ~DOWN;
        break;
      case Qt::Key_L:
        keysPressed &= ~L;
        break;
      case Qt::Key_R:
        keysPressed &= ~R;
        break;
      case Qt::Key_A:
        keysPressed &= ~A;
        break;
      case Qt::Key_B:
        keysPressed &= ~B;
        break;
      case Qt::Key_X:
        keysPressed &= ~X;
        break;
      case Qt::Key_Y:
        keysPressed &= ~Y;
        break;
      default:
        event->ignore();
        break;
    }
  }
}

void TextBox::outputText (int x, int y, const char *str) {
  painter.setPen (currentPen);
  painter.drawText (QPoint (x*charWidth, (y+1)*charHeight), str);
  updates.push_back (QRect (x * charWidth, (y-1) * charHeight,
                            (x+strlen (str))*charWidth,
                            (y+1)*charWidth));
}

void TextBox::clearText (int from, int to) {
  QPen pen;
  pen.setColor ("#000000");
  painter.setPen (pen);
  painter.drawRect (0, (from)*charHeight,
                    COLUMNS*charWidth, (to+1)*charHeight);
  updates.push_back (QRect (0, (from)*charHeight,
                            (COLUMNS+1)*charWidth, (to+1)*charHeight));
}

///////////////////////////////////////////////////////////////////////////////
// General
int main(int argc, char **argv) {
  if (argc < 4) {
    printf ("Panic: args\n");
    exit (1);
  }
  strncpy (serverIP, argv[1], 16);
//   bool client = (atoi(argv[2]) > 0);
  int checkahead = atoi(argv[2]);
  int maxsteps = atoi(argv[3]);
  QApplication app (argc, argv);
  window = new RetrorocketWindow(checkahead,maxsteps);
  retrorocket->bot = true;
  window->show();
  retrorocket->gameDone = false;
  window->start();

//   RetrorocketWindow window2 = new RetrorocketWindow(!client);
//   window2->show();
//   window2->start();

  return app.exec();
}

void waitVBL() {
  window->waitVBL();
}

void initIntroGfx() {
  if (fieldBox->background) deleteBg (BACK_BG);
  if (fieldBox->foreground) deleteBg (FRONT_BG);
  if (fieldBox->sob) deleteBg (SOB_BG);
  for (int i = 0; i < MAX_SPRITES; i++) {
    deleteSprite (i);
  }
}

void initGameGfx() {
  initIntroGfx();
}

char *getUserName() {
  struct passwd *pwbuf = getpwuid (getuid());
  return pwbuf->pw_name;
}

// Sprite
void setFocus (Ship *foc) {
  focus = foc;
}
void createSprite (void *pal, void *gfx, int spritenum) {
  QImage image = QImage (SPRITESIZE, SPRITESIZE, QImage::Format_ARGB32);
  for (int y = 0; y < SPRITESIZE; y++) {
    for (int x = 0; x < SPRITESIZE; x++) {
      int p = Mob::getPixel ((u8*)gfx, x, y);
      if (p != 0) {
        int red = Field::getRed (p, (u16*)pal) << 3;
        int green = Field::getGreen (p, (u16*)pal) << 3;
        int blue = Field::getBlue (p, (u16*)pal) << 3;
        image.setPixel (x, y, qRgb(red, green, blue));
      } else {
        image.setPixel (x, y, qRgba(0, 0, 0, 0));
      }        
    }
  }
  fieldBox->createSprite (QPixmap::fromImage (image), spritenum);
}
void createParticleSprite (int spritenum) {
  QImage image = QImage (1, 1, QImage::Format_ARGB32);
  image.setPixel (0, 0, qRgb(255, 255, 255));
  fieldBox->createSprite (QPixmap::fromImage (image), spritenum);
}
void deleteSprite (int spritenum) {
  if (fieldBox->sprites[spritenum]) {
    delete fieldBox->sprites[spritenum];
    fieldBox->sprites[spritenum] = NULL;
  }
}
void moveSprite (Fix32 x, Fix32 y, int spritenum) {
  if (fieldBox->sprites[spritenum]) {
    fieldBox->sprites[spritenum]->setVisible (true);
    fieldBox->moveSprite (x.toInt(), y.toInt(), spritenum);
  }
}
void hideSprite (int spritenum) {
  if (fieldBox->sprites[spritenum]) {
    fieldBox->sprites[spritenum]->setVisible (false);
  }
}

void drawSOBsOnMap() {}

void setSpriteRotEnable (int spritenum) {}
void rotateSprite (int spritenum, Fix32 angle) {
  if (fieldBox->sprites[spritenum]) {
    fieldBox->rotateSprite (spritenum, angle.toInt());
  }
}
void hideMapSprite (int spritenum) {}
void showMapSprite (int spritenum) {}

// Background
void loadBg (int *info,
             const unsigned short *map,
             int sizeofMap, const unsigned char *tiles,
             int sizeofTiles, const unsigned short *pal, int bg) {
  int width = info[1];
  int height = info[2];

  QImage image = QImage (width, height, QImage::Format_ARGB32);
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int p = Field::getPixel ((u16*)map, (u8*)tiles, info,
                               x, y, width, height, -1);
      if (p != 0) {
        int red = Field::getRed (p, (u16*)pal) << 3;
        int green = Field::getGreen (p, (u16*)pal) << 3;
        int blue = Field::getBlue (p, (u16*)pal) << 3;
        image.setPixel (x, y, qRgb(red, green, blue));
      } else {
        image.setPixel (x, y, qRgba(0, 0, 0, 0));
      }        
    }
  }
  fieldBox->loadBg (QPixmap::fromImage (image), bg);
}

void deleteBg (int bg) {
  switch (bg) {
    case BACK_BG:
      delete fieldBox->background;
      fieldBox->background = NULL;
      break;
    case FRONT_BG:
      delete fieldBox->foreground;
      fieldBox->foreground = NULL;
      break;
    case SOB_BG:
      delete fieldBox->sob;
      fieldBox->sob = NULL;
      break;
  }
  if (!(fieldBox->background || fieldBox->foreground || fieldBox->sob)) {
  }
}

void loadBgPal (int bg, int palSlot, const unsigned short *pal) {
}
#define TILE_PALPOS 12
void setBgTile (int bg, Fix32 xp, Fix32 yp, int tile, int palSlot) {
  unsigned short *bgMap = retrorocket->field->mapSOB; // FIXME: choose correct bg

  // adjust tile to accomodate palSlot
  tile |= (palSlot+1) << TILE_PALPOS;
  
  int tileToSet = retrorocket->field->getTile (xp.toUInt(), yp.toUInt());

  // set tile in map
  bgMap[tileToSet] = tile;
}
void showBG (int bg) {
}
void hideBG (int bg) {
}

// Map
void drawBM (u8 *buffer, int w, int h, int threshold,
             int left, int top, int width, int height, bool bw) {
  mapBox->drawMap (buffer, w, h, threshold);
}
void drawMap (u16 *map, u8 *tiles, int *info, int w, int h,
              int left, int top, int width, int height) {
  u8 *buffer = (u8*)malloc (w * h);
  if (!buffer) panic ("Out of memory");

  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      buffer[y*w+x] = Field::getCPixel (map, tiles, info, (int)x, (int)y, w, h);
    }
  }
  mapBox->drawMap (buffer, w, h, 0);
  free (buffer);
}
void hideMap() {
//   mapBox->hide();
}
void showMap() {
//   mapBox->show();
}

// Text
void clearText() {
  clearText (0, ROWS);
}
void clearText (int from, int to) {
  textBox->clearText (from, to);
  setTextColor (0);
}
void setTextColor (int c) {
  QColor currentColor;

  switch (c) {
    case WHITE:
      currentColor = QColor ("#ffffff");
      break;
    case RED:
      currentColor = QColor ("#ff0000");
      break;
    case GREEN:
      currentColor = QColor ("#00ff00");
      break;
    case BLUE:
      currentColor = QColor ("#0000ff");
      break;
    case MAGENTA:
      currentColor = QColor ("#ff00ff");
      break;
    case CYAN:
      currentColor = QColor ("#00ffff");
      break;
    case YELLOW:
      currentColor = QColor ("#ffff00");
      break;
  }

  textBox->currentPen.setColor (currentColor);
}
void outputText (int x, int y, const char *fmt, ...) {
  va_list ap;
  va_start (ap, fmt);

  char line[LINESIZE];
  vsprintf (line, fmt, ap);

  textBox->outputText (x, y, line);

  va_end (ap);
}

// File I/O
void getFileList (const char *ext, vector<char*>*filenames) {
  dirent *dp;
  DIR* dir;

  dir = opendir ("."); 

  if (dir == NULL) {
    panic ("Can't open directory");
  } else {
    while ((dp = readdir (dir))) {
      if (matchExt (dp->d_name, ext)) {
        char *filename = (char*)malloc (strlen (dp->d_name) + 1);
        if (!filename) panic ("Out of memory");
        strcpy (filename, dp->d_name);
        filenames->push_back (filename);
      }
    }
  }
  closedir (dir);
}

// Sound
void playSound (u8 *data, int size, int volume,  Fix32 x, Fix32 y,
                int rate) {}
void playSound (u8 *data, int size, int volume,
                int rate, int panning) {}
void playSoundThrust (u8 *data, int size) {}
void modifySoundThrust (int rate, int volume) {}

// Music
void loadMod (const char *filename, 
              int randomStartPos, int randomPatterns, int blankPos) {}
void freeMod() {}
void playMod (int pos) {}
void playRandomMod() {}
void stopMod() {}

// Network
Network *openNetworkMaster (char *fieldFilename, int min, int max,
                            int aiPlayers, bool autogen) {
  return openNetwork (true, fieldFilename, min, max, aiPlayers, autogen);
}
Network *openNetworkSlave (char *fieldFilename) {
  return openNetwork (false, fieldFilename, 0, 0, 0, false);
}
void setWirelessChannel (int i) {}

// Input
unsigned int sysGetInput() {
  return keysPressed;
}
void readLine (void (*cb)(char *s), const char *str) {
  cb ((char *)"debug");
}
void getMapClick (int *x, int *y) {
  mapBox->resetClick();
  while (!mapBox->click (x, y)) {
    waitVBL();
  }
  *x = *x * mapBox->fieldWidth / mapBox->mapWidth;
  *y = *y * mapBox->fieldHeight / mapBox->mapHeight;
}

bool getPointer() {
  return false;
}

bool getPointer (int *x, int *y) {
  return false;
}

// Other
int startCritical() {
  return 0;
}
void endCritical (int ime) {}


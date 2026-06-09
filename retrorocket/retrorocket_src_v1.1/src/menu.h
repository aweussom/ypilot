/*
  menu.h
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
// Menu system
//
///////////////////////////////////////////////////////////////////////////////

#ifndef MENU_H
#define MENU_H

#include "retrorocket.h"

///////////////////////////////////////////////////////////////////////////////

#define TITLECOLOR    BLUE
#define ITEMCOLOR     WHITE
#define CHOSENCOLOR   YELLOW
#define BUTTONCOLOR   GREEN

#define MENUITEMSIZE  (COLUMNS - SCROLLSTARTX - 2)

#define SCROLLSTARTX  2
#define SCROLLSTARTY  3
#define TITLEX        1
#define TITLEY        1

#define TYPERATE      5
#define TYPEDELAY     20

#define DEC_BUTTON_POS (COLUMNS - 8)

#define INCREMENTSTART  1
#define INCREMENTGROWTH 1.2

/** possible events that can be passed to actOn() in widgets */
enum EventType {
  STYLUS_DOWN,
  STYLUS_UP,
  STYLUS_MOVE,
  KEYPAD_DOWN,
  KEYPAD_UP
};

///////////////////////////////////////////////////////////////////////////////

class Widget {

protected:

  int actKey; // key code for current event
  int actTimer; // autorepeat timer for active event
  
public:
  int actEvent; // active event (0 for no event or KEYPAD_DOWN for
                //               autorepeating last key event)

  Widget() { actEvent = 0; }
  virtual ~Widget() {}

  virtual void redraw() {}

  /** act on stylus event, no default processing */
  virtual bool actOn (int e, int x, int y) { return false; }

  /** act on key event, sets up the autorepeat system */
  virtual void actOn (int e, int key);

  /** manages autorepeat */
  virtual void advance();

  /** performs key event, called every autorepeat */
  virtual void performEvent(int key) {}
};

///////////////////////////////////////////////////////////////////////////////

class ScrollBox;

/** Simple scrollbar with stylus support */
class ScrollBar : public Widget {
  ScrollBox *parent;
  int x;
  int y;
  int height;
  bool pressed;
  
public:
  ScrollBar (int x, int y, int height, ScrollBox *parent);
  bool actOn (int e, int x, int y);
  void redraw();
};
  
///////////////////////////////////////////////////////////////////////////////

/** Simple stylus push button which generates keyevent
    for its parent when pushed */
class Button : public Widget {
  const char *text;
  Widget *parent;
  int key;

public:
  int y;

  Button (const char *text, int key, int y, Widget *parent);
  bool actOn (int e, int x, int y);
  void redraw (int x);
};

///////////////////////////////////////////////////////////////////////////////

class Item;
class Menu;

/** Scroll box, consisting of several items in a list
    Takes care of managing and placing items */
class ScrollBox : public Widget {
  int y;
  Menu *parent;
  ScrollBar scrollBar;
  Button quitButton;
  Button backButton;
  Button adjustButton;
  Button selectButton;
  
public:
  string suffixToStrip;
  int height;
  vector<Item*> items;
  int choice;
  int scrollPos;
  void (*callback)(Menu *menu);

  ScrollBox (Menu *parent, int y, int height);
  void redraw();
  bool actOn (int e, int x, int y);
  void actOn (int e, int key);
  void performEvent(int key);
  void advance();

  void doCallback();
  void addItem (Item *item);
  void clearItems();

  /** After items have been removed/added, call this to make sure
      all indexes and pointers are valid */
  void update();
};
  
///////////////////////////////////////////////////////////////////////////////

class Menu;

/** Base class for all scroll box items */
class Item : public Widget {

public:
  const char *text;
  void (*action)(Menu *menu);
  long int actionArg;
  Menu *parent;

  Item (const char *text, Menu *parent,
        void (*cb)(Menu *menu) = NULL, long int arg = 0);
  virtual ~Item() {}
  virtual string getString() = 0;
  virtual bool isSelectable() { return false; }
  virtual bool isAdjustable() { return false; }
  virtual void update() {}
  virtual bool isSubMenuItem() { return false; }
  bool hitsBoundingBox (int x, int y);
};

//-----------------------------------------------------------------------------

/** Item representing a sub menu */
class SubMenuItem : public Item {
public:
  Menu *subMenu;
  
  SubMenuItem (const char *text, Menu *parent,
               Menu *subMenu, 
               void (*cb)(Menu *menu) = NULL, long int arg = 0);
  bool actOn (int e, int x, int y);
  void performEvent (int key);
  bool isSubMenuItem() { return true; }
  bool isSelectable() { return true; }
  
  string getString();
};

//-----------------------------------------------------------------------------

/** Item representing a variable that can be adjusted */
class VarItem : public Item {
  int start;
  int end;
  int *now;
  float increment;
  bool pressed;

public:
  VarItem (const char *text, Menu *parent,
           int start, int end, int* ptr,
           void (*cb)(Menu *menu) = NULL, long int arg = 0);

  bool actOn (int e, int x, int y);
  void performEvent(int key);
  bool isAdjustable() { return true; }
  void update();
  string getString();
};

//-----------------------------------------------------------------------------

/** Item representing an item that generates a callback (and nothing else) */
class ActionItem : public Item {
public:
  ActionItem (const char *text, Menu *parent,
              void (*cb)(Menu *menu), long int arg = 0);
  
  void performEvent(int key);
  bool actOn (int e, int x, int y);
  bool isSelectable() { return true; }
  string getString();
};

//-----------------------------------------------------------------------------

/** Multiple choice item */
class MultiItem : public Item {
  int *multiNow;
  vector<const char *> *multiList;
  bool pressed;

public:
  MultiItem (const char *text, Menu *parent,
             int* multiNow,  vector<const char *> * multiList,
             void (*cb)(Menu *menu) = NULL, long int arg = 0);

  bool actOn (int e, int x, int y);
  bool isAdjustable() { return true; }
  void performEvent(int key);
  void update();
  string getString();
};

///////////////////////////////////////////////////////////////////////////////

/** Class taking care of displaying a menu */
class Menu {
  ScrollBox scrollBox;
  int linesToReserve;

  static bool lastPointerHeld;
  static int lastPointerX;
  static int lastPointerY;
  static int lastKeyInput;

  const char *title;
  
public:
  bool done;
  bool hidden;
  Menu* activeSubMenu;
  bool canQuit;
  bool canBack;

  /** linesToReserve is number of lines at the bottom not to be used by menu */
  Menu(const char *title, int linesToReserve = 0);
  ~Menu();

  /** remove given suffix when displaying item */
  void setSuffixToStrip (string suffix);

  int getNumberOfItems();
  /** returns current choice */
  int getChoice();
  /** selects an item */
  void setChoice (int choice);
  /** menu is done, user pressed back or quit */
  bool isDone();
  void clearItems();
  /** returns a pointer to the currently chosen item */
  Item *getChosenItem();

  /** add sub menu item */
  void addItem (const char *text, Menu *subMenu,
                void (*cb)(Menu *menu) = NULL, long int arg = 0);
  /** add action item */
  void addItem (const char *text, void (*cb)(Menu *menu) = NULL,
                long int arg = 0);
  /** add variable item */
  void addItem (const char *text, int start, int end, int* ptr,
                void (*cb)(Menu *menu) = NULL, long int arg = 0);
  /** add multiple choice item */
  void addItem (const char *text,
                int* multinow,  vector<const char *> * multilist,
                void (*cb)(Menu *menu) = NULL, long int arg = 0);

  void hide();
  void show();

  /** give user possibility to quit menu */
  void setQuitable();
  /** give user possibility to go back */
  void setBack();

  const char *getItemText (int i);
  const char *getCurrentItemText();
  void updateMenu();
  /** must be called every vbl */
  void advance();
  /** show the menu for the first time */
  void presentMenuAsync (int choice = 0);
  void redraw();
  /** remove menu */
  void remove();
  /** sets a callback that is called every time user changes chosen item */
  void setCallback (void (*cb)(Menu *menu));
  /** deletes given menu and all its submenus */
  static void freeAllMenus (Menu *menu);
};

///////////////////////////////////////////////////////////////////////////////

#endif

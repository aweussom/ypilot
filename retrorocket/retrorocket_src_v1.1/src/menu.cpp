/*
  menu.cpp
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

bool Menu::lastPointerHeld = false;
int Menu::lastPointerX = -1;
int Menu::lastPointerY = -1;
int Menu::lastKeyInput = -1;

Menu::Menu (const char *title, int linesToReserve)
  : scrollBox (this, SCROLLSTARTY, ROWS - linesToReserve - SCROLLSTARTY) {

  done = true;
  hidden = true;
  canBack = false;
  canQuit = false;
  activeSubMenu = NULL;
  lastPointerX = -1;
  lastPointerY = -1;
  lastPointerHeld = false;
  lastKeyInput = -1;
  this->linesToReserve = linesToReserve;
  this->title = title;
}

Menu::~Menu() {
  clearItems();
}

void Menu::setSuffixToStrip (string suffix) {
  scrollBox.suffixToStrip = suffix;
}

int Menu::getNumberOfItems() { return scrollBox.items.size(); }
int Menu::getChoice() { return scrollBox.choice; }
void Menu::setChoice (int choice) {
  scrollBox.choice = choice;
  scrollBox.update();
  scrollBox.redraw();
}
bool Menu::isDone() { return done; }
void Menu::clearItems() { scrollBox.clearItems(); };
Item *Menu::getChosenItem() { return scrollBox.items[scrollBox.choice]; }

void Menu::addItem (const char *text, Menu *subMenu,
              void (*cb)(Menu *menu), long int arg) {
  subMenu->setBack();
  if (canQuit) subMenu->setQuitable();
  SubMenuItem *item = new SubMenuItem (text, this, subMenu, cb, arg);
  scrollBox.addItem (item);
}
void Menu::addItem (const char *text, void (*cb)(Menu *menu), long int arg) {
  ActionItem *item = new ActionItem (text, this, cb, arg);
  scrollBox.addItem (item);
}
void Menu::addItem (const char *text, int start, int end, int* ptr,
              void (*cb)(Menu *menu), long int arg) {
  VarItem *item = new VarItem (text, this, start, end, ptr, cb, arg);
  scrollBox.addItem (item);
}
void Menu::addItem (const char *text,
              int* multinow,  vector<const char *> * multilist,
              void (*cb)(Menu *menu), long int arg) {
  MultiItem *item = new MultiItem (text, this, multinow, multilist, cb, arg);
  scrollBox.addItem (item);
}

void Menu::hide() {
  hidden = true;
  if (activeSubMenu) activeSubMenu->hide();
}

void Menu::show() {
  lastPointerX = -1;
  lastPointerY = -1;
  lastPointerHeld = getPointer();
  lastKeyInput = msb (sysGetInput());
  scrollBox.actEvent = 0;
  hidden = false;
  if (activeSubMenu) activeSubMenu->show();
  else redraw();
}

void Menu::setQuitable() {
  canQuit = true;
}
void Menu::setBack() {
  canBack = true;
}

const char *Menu::getItemText (int i) {
  return scrollBox.items[i]->text;
}

const char *Menu::getCurrentItemText() {
  return scrollBox.items[scrollBox.choice]->text;
}

void Menu::updateMenu() {
  scrollBox.update();
}
void Menu::advance() {
  if (!done && !hidden) {

    if (activeSubMenu) {
      // update submenu instead
      activeSubMenu->advance();

      if (activeSubMenu->isDone()) {
        if (activeSubMenu->getChoice() == -1) {
          // Back
          activeSubMenu = NULL;
          scrollBox.actEvent = 0;
          redraw();
        } else {
          // Quit
          activeSubMenu = NULL;
          remove();
        }
      }

    } else {
      // this menu is the active one

      // get user input
      int pointerX, pointerY;
      int keyInput = msb(sysGetInput());
      bool pointerHeld = getPointer(&pointerX, &pointerY);
      pointerX /= 8;
      pointerY /= 8;
      
      int event = 0;

      // generate events for scrollbox
      if (keyInput != lastKeyInput) {
        event = keyInput ? KEYPAD_DOWN : KEYPAD_UP;
        scrollBox.actOn (event, keyInput);
      } else if (pointerHeld != lastPointerHeld) {
        event = pointerHeld ? STYLUS_DOWN : STYLUS_UP;
        scrollBox.actOn (event, pointerX, pointerY);
      } else if (pointerHeld &&
                 ((pointerX!=lastPointerX)||(pointerY!=lastPointerY))) {
        event = STYLUS_MOVE;
        scrollBox.actOn (event, pointerX, pointerY);
      }    
      lastPointerX = pointerX;
      lastPointerY = pointerY;
      lastPointerHeld = pointerHeld;
      lastKeyInput = keyInput;

      // advance scrollbox
      scrollBox.advance();
    }
  }
}
  
void Menu::presentMenuAsync (int choice) {
  done = false;
  hidden = false;
  activeSubMenu = NULL;
  lastKeyInput = msb (sysGetInput());
  scrollBox.choice = choice;
  scrollBox.scrollPos = 0;
  updateMenu();
  redraw();
}

void Menu::redraw() {
  clearText (0, SCROLLSTARTY-1);
  setTextColor (TITLECOLOR);
  outputText (TITLEX, TITLEY, title);
  scrollBox.redraw();
  scrollBox.doCallback();
}

void Menu::remove() {
  clearText (0, ROWS-linesToReserve);
  done = true;
}

void Menu::setCallback (void (*cb)(Menu *menu)) {
  scrollBox.callback = cb;
}

void Menu::freeAllMenus (Menu *menu) {
  for (unsigned int i = 0; i < menu->scrollBox.items.size(); i++) {
    if (menu->scrollBox.items[i]->isSubMenuItem()) {
      SubMenuItem *item = (SubMenuItem*)menu->scrollBox.items[i];
      freeAllMenus (item->subMenu);
    }
  }
  delete (menu);
}

///////////////////////////////////////////////////////////////////////////////

void Widget::advance() {
  // check if active event should autorepeat
  if (actEvent) {
    if ((actTimer == 0) ||
        (actTimer == TYPEDELAY) ||
        ((actTimer > TYPEDELAY) && !((actTimer-TYPEDELAY) % TYPERATE))){
      performEvent(actKey);
    }
    actTimer++;
  }
}

void Widget::actOn (int e, int key) {
  if (e == KEYPAD_DOWN) {
    actEvent = e;
    actTimer = 0;
    actKey = key;
  } else if (e == KEYPAD_UP) {
    actEvent = 0;
    performEvent (0);
  }
}

///////////////////////////////////////////////////////////////////////////////

Button::Button (const char *text, int key, int y, Widget *parent) {
  this->text = text;
  this->y = y;
  this->parent = parent;
  this->key = key;
}

bool Button::actOn (int e, int x, int y) {
  if ((e == STYLUS_UP) && (x >= 0) && (x < (int)strlen (text)) && (y == 0)) {
    parent->performEvent (key);
    return true;
  }
  return false;
}

void Button::redraw (int x) {
  setTextColor (TITLECOLOR);
  outputText (x, y, text);
}

///////////////////////////////////////////////////////////////////////////////

ScrollBar::ScrollBar (int x, int y, int height, ScrollBox *parent) {
  this->parent = parent;
  this->x = x;
  this->y = y;
  this->height = height;
  pressed = false;
}

bool ScrollBar::actOn (int e, int x, int y) {
  // pressed up button
  if ((e == STYLUS_DOWN) && (y == this->y) && (x == this->x)) {
    parent->actOn (KEYPAD_DOWN, L);
    pressed = true;
    return true;
  }
  // pressed down button
  if ((e == STYLUS_DOWN) && (y == this->y + height - 1) && (x == this->x)) {
    parent->actOn (KEYPAD_DOWN, R);
    pressed = true;
    return true;
  }

  // pressed in scrolling area
  if (((e == STYLUS_DOWN) || (e == STYLUS_MOVE)) &&
      (y > this->y) && (y < this->y + height - 1) &&
      (x == this->x)) {
    int newScrollPos = (((Fix32)y *
                        (Fix32)((int)parent->items.size() -
                                (int)parent->height)) /
                        (Fix32)(height-3)).toUInt();
    parent->scrollPos = newScrollPos;
    if (parent->choice < parent->scrollPos) {
      parent->choice = parent->scrollPos;
    }
    if (parent->choice > parent->scrollPos + parent->height - 1) {
      parent->choice = parent->scrollPos + parent->height - 1;
    }
    parent->redraw();
    pressed = true;
    return true;
  }
  
  // pressed outside or depressed, stop generating autorepeats
  if (pressed && ((e == STYLUS_UP) || ((e == STYLUS_MOVE) &&
                                       ((y < this->y) ||
                                        (y >= this->y + height) ||
                                        (x != this->x))))) {
    parent->actOn (KEYPAD_UP, 0);
    pressed = false;
    return true;
  }
  return false;
}

void ScrollBar::redraw() {
  setTextColor (BUTTONCOLOR);
  outputText (x, y, "^");
  int barPos = (((Fix32)parent->scrollPos /
                 (Fix32)((int)parent->items.size() - (int)parent->height)) *
                (Fix32)(height-3)).toUInt();
  outputText (x, y + barPos + 1, "*");
  outputText (x, y + height - 1, "v");
}

///////////////////////////////////////////////////////////////////////////////

ScrollBox::ScrollBox (Menu *parent, int y, int height)
  : scrollBar (COLUMNS - 1, y, height - 2, this),
    quitButton ("X:quit", X, y + height - 1, this),
    backButton ("B:back", B, y + height - 1, this),
    adjustButton ("<>:adjust", 0, y + height - 1, this),
    selectButton ("A:select", A, y + height - 1, this) {

  this->parent = parent;
  this->y = y;
  this->height = height - 2;
  choice = 0;
  scrollPos = 0;
  callback = NULL;
  suffixToStrip = "";
}

void ScrollBox::clearItems() {
  for (int i = 0; i < (int)items.size(); i++) {
    delete items[i];
  }
  items.clear();
}

void ScrollBox::update() {
  if (choice >= (int)items.size()) choice = items.size() - 1;
  if ((choice < 0) && (items.size() > 0)) choice = 0;
  for (int i = 0; i < (int)items.size(); i++) {
    items[i]->update();
  }

  if (choice < scrollPos) scrollPos = choice;
  if (choice > scrollPos + height - 1) scrollPos = choice - height + 1;
  if (scrollPos < 0) scrollPos = 0;

  redraw();
}

void ScrollBox::redraw() {  
  if (!parent->hidden && !parent->done) {
    clearText (y, y+height-1+3);

    // redraw menu buttons
    int x = SCROLLSTARTX-1;
    if (parent->canQuit) {
      quitButton.redraw (x);
      x += 10;
    }
    if (parent->canBack) {
      backButton.redraw (x);
      x += 10;
    }

    if ((choice >= 0) && items.size()) {
      // redraw items
      for (int i = scrollPos;
           (i < (int)items.size()) && (i < (height + scrollPos));
           i++) {
        if (i == choice) {
          setTextColor(CHOSENCOLOR);
          outputText (SCROLLSTARTX-1, y+i-scrollPos, ">");
        } else {
          setTextColor(ITEMCOLOR);
        }
        string itemText = stripSuffix (items[i]->getString(), suffixToStrip);
        outputText (SCROLLSTARTX, y+i-scrollPos, itemText.c_str());
      }

      // redraw item buttons
      if (items[choice]->isSelectable()) {
        selectButton.redraw (x);
        x += 10;
      }
      if (items[choice]->isAdjustable()) {
        adjustButton.redraw (x);
      }
    }

    // redraw scrollbar
    if ((height >= 3) && ((int)items.size() > height)) scrollBar.redraw();
  }
}

void ScrollBox::advance() {
  Widget::advance();
  if ((choice >= 0) && items.size()) items[choice]->advance();
  scrollBar.advance();
  quitButton.advance();
  backButton.advance();
  selectButton.advance();
  adjustButton.advance();
}

void ScrollBox::doCallback() {
  if (callback && !parent->hidden && !parent->done &&
      !parent->activeSubMenu) {
    callback (parent);
  }
}

bool ScrollBox::actOn (int e, int x, int y) {
  bool acted = false;

  // pass event to all items
  for (int i = scrollPos;
       (i < (int)items.size()) && (i < (height + scrollPos));
       i++) {
    int relX = x - SCROLLSTARTX;
    int relY = y - this->y - (i - scrollPos);
    if (items[i]->actOn(e, relX, relY)) {
      doCallback();
      acted |= true;
    }
    // check if this item is selected
    if ((e == STYLUS_DOWN) || (e == STYLUS_MOVE)) {
      if (items[i]->hitsBoundingBox (relX, relY) && choice != i) {
        choice = i;
        redraw();
        doCallback();
        acted |= true;
      }
    }
  }
  acted |= scrollBar.actOn (e, x, y);

  // pass event to all buttons
  int buttonX = SCROLLSTARTX-1;
  if (parent->canQuit) {
    acted |= quitButton.actOn (e, x - buttonX, y - quitButton.y);
    buttonX += 10;
  }
  if (parent->canBack) {
    acted |= backButton.actOn (e, x - buttonX, y - backButton.y);
    buttonX += 10;
  }
  if ((choice >= 0) && items.size()) {
    if (items[choice]->isSelectable()) {
      acted |= selectButton.actOn (e, x - buttonX, y - selectButton.y);
      buttonX += 10;
    }
    if (items[choice]->isSelectable()) {
      acted |= adjustButton.actOn (e, x - buttonX, y - adjustButton.y);
    }
  }

  return acted;
}

void ScrollBox::actOn (int e, int key) {
  Widget::actOn (e, key);
}

void ScrollBox::performEvent (int key) {
  switch (key) {
    case UP:
      choice--;
      if (choice < 0) choice = items.size()-1;
      break;
    case DOWN:
      choice++;
      if (choice >= (int)items.size()) choice = 0;      
      break;
    case L:
      if (scrollPos > 0) scrollPos--;
      if (choice >= scrollPos+height) choice = scrollPos + height - 1;
      break;
    case R:
      if (scrollPos < (int)items.size()-height) scrollPos++;
      if (choice < scrollPos) choice = scrollPos;
      break;
    case X:
      if (parent->canQuit) {
        actEvent = 0;
        parent->remove();
      }
      break;
    case B:
      if (parent->canBack) {
        choice = -1;
        actEvent = 0;
        parent->remove();
      }
      break;
    default:
      break;
  }
  // adjust scrollpos if outside legal values
  if (choice < scrollPos) scrollPos = choice;
  if (choice > scrollPos + height - 1) scrollPos = choice - height + 1;
  if (scrollPos < 0) scrollPos = 0;

  // redraw in case this is a real event (key != 0)
  if (key) redraw();

  // send event to chosen item
  if ((choice >= 0) && items.size()) items[choice]->performEvent (key);

  // do callback
  if (key && (choice >= 0) && items.size()) doCallback();
}

void ScrollBox::addItem (Item *item) {
  items.push_back(item);
}

///////////////////////////////////////////////////////////////////////////////

Item::Item (const char *text, Menu *parent, void (*cb)(Menu *menu),
            long int arg) {
  this->text = text;
  this->action = cb;
  this->actionArg = arg;
  this->parent = parent;
}

bool Item::hitsBoundingBox (int x, int y) {
  return (x >= 0) && (x < (int)getString().size()) && (y == 0);
}

//-----------------------------------------------------------------------------

SubMenuItem::SubMenuItem (const char *text, Menu *parent, Menu *subMenu, 
                          void (*cb)(Menu *menu), long int arg)
  : Item(text, parent, cb, arg) {
  this->subMenu = subMenu;
}

bool SubMenuItem::actOn (int e, int x, int y) {
  if ((e == STYLUS_UP) && hitsBoundingBox (x, y)) {
    performEvent (A);
    return true;
  }
  return false;
}

void SubMenuItem::performEvent(int key) {
  if (key == A) {
    if (action) action (parent);
    subMenu->presentMenuAsync();
    parent->activeSubMenu = subMenu;
  }
}

string SubMenuItem::getString() {
  return text;
}

//-----------------------------------------------------------------------------

static inline bool hitsDecButton (int x, int y) {
  return (y == 0) && (x == DEC_BUTTON_POS);
}

static inline bool hitsIncButton (int x, int y) {
  return (y == 0) && (x == DEC_BUTTON_POS + 2);
}

//-----------------------------------------------------------------------------

VarItem::VarItem (const char *text, Menu *parent, int start, int end, int* ptr,
                  void (*cb)(Menu *menu), long int arg)
  : Item(text, parent, cb, arg) {
  this->start = start;
  this->end = end;
  this->now = ptr;
  if (*now > end) *now = end;
  if (*now < 0) *now = 0;
  increment = INCREMENTSTART;
  pressed = false;
}

bool VarItem::actOn (int e, int x, int y) {
  // hits dec button
  if ((e == STYLUS_DOWN) && hitsDecButton (x, y)) {
    Widget::actOn (KEYPAD_DOWN, LEFT);
    pressed = true;
    return true;
  }
  // hits inc button
  if ((e == STYLUS_DOWN) && hitsIncButton (x, y)) {
    Widget::actOn (KEYPAD_DOWN, RIGHT);
    pressed = true;
    return true;
  }
  // stop repeating
  if (pressed && ((e == STYLUS_UP) || ((e == STYLUS_MOVE) &&
                                       !hitsIncButton (x, y) &&
                                       !hitsDecButton (x, y)))) {
    Widget::actOn (KEYPAD_UP, 0);
    pressed = false;
    return true;
  }
  return false;
}

void VarItem::performEvent(int key) {
  switch (key) {

    case 0: // key up
      increment = INCREMENTSTART;
      break;

    case RIGHT:
      *now = (*now) + (int)increment;
      if ((*now) > end) {
        *now = end;
      }
      increment = increment * INCREMENTGROWTH;

      if (action) action (parent);
      parent->redraw();
      break;

    case LEFT:
      *now = (*now) - (int)increment;
      if ((*now) < start) {
        *now = start;
      }
      increment = increment * INCREMENTGROWTH;

      if (action) action (parent);
      parent->redraw();
      break;
  }
}

string VarItem::getString() {
  char ctmp[LINESIZE];
  sprintf (ctmp, "%d", *now);
  string tmp = string (text).append (" ").append (ctmp);
  tmp.resize (DEC_BUTTON_POS, ' ');
  tmp.append ("- +");
  return tmp;
}

void VarItem::update() {
  if (*now < start) (*now) = start;
  if (*now > end) (*now) = end;
}

//-----------------------------------------------------------------------------

ActionItem::ActionItem (const char *text, Menu *parent,
                        void (*cb)(Menu *menu), long int arg)
  : Item(text, parent, cb, arg) {

}
  
bool ActionItem::actOn (int e, int x, int y) {
  if ((e == STYLUS_UP) && hitsBoundingBox (x, y)) {
    performEvent (A);
    return true;
  }
  return false;
}

void ActionItem::performEvent(int key) {
  if (key == A) {
    if (action) action (parent);
  }
}

string ActionItem::getString(){
  return text;
}

//-----------------------------------------------------------------------------

MultiItem::MultiItem (const char *text, Menu *parent, 
                      int* multiNow,  vector<const char *> * multiList,
                      void (*cb)(Menu *menu), long int arg)
  : Item(text, parent, cb, arg) {
  
  this->multiNow = multiNow;
  this->multiList = multiList;
  if (*multiNow >= (int)multiList->size()) *multiNow = multiList->size() - 1;
  if (*multiNow < 0) *multiNow = 0;
  pressed = false;
}

bool MultiItem::actOn (int e, int x, int y) {
  // hits dec button
  if ((e == STYLUS_DOWN) && hitsDecButton (x, y)) {
    Widget::actOn (KEYPAD_DOWN, LEFT);
    pressed = true;
    return true;
  }
  // hits inc button
  if ((e == STYLUS_DOWN) && hitsIncButton (x, y)) {
    Widget::actOn (KEYPAD_DOWN, RIGHT);
    pressed = true;
    return true;
  }
  // stop repeating
  if (pressed && ((e == STYLUS_UP) || ((e == STYLUS_MOVE) &&
                                       !hitsIncButton (x, y) &&
                                       !hitsDecButton (x, y)))) {
    Widget::actOn (KEYPAD_UP, 0);
    pressed = false;
    return true;
  }
  return false;
}

void MultiItem::performEvent(int key) {
  switch (key) {
    case RIGHT:
      if (*multiNow < (int)multiList->size() - 1) (*multiNow)++;
      if (action) action (parent);
      parent->redraw();
      break;
    case LEFT:
      if (*multiNow > 0) (*multiNow)--;
      if (action) action (parent);
      parent->redraw();
      break;
  }
}

string MultiItem::getString() {
  string tmp = string (text);
  if (multiList->size()) {
    tmp = tmp.append (" ").append ((*multiList)[*multiNow]);
  } else {
    tmp = tmp.append (" <none>");
  }
  tmp.resize (DEC_BUTTON_POS, ' ');
  tmp.append ("< >");
  return tmp;
}

void MultiItem::update() {
  if (*multiNow < 0) (*multiNow) = 0;
  if (*multiNow >= (int)multiList->size()) (*multiNow) = multiList->size() - 1;
}

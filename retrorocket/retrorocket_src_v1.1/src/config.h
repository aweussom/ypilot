/*
  config.h
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
// Configuration file handling
//
///////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_H
#define CONFIG_H

using namespace std;

#include <vector>
#include <string>
#include <map>
#include <stdexcept>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

extern "C" void panic (const char *fmt, ...);
int str2CRC (string str);

#define CONFIG_LINESIZE        256
#define CONFIG_INCLUDE_KEYWORD "include"
#define CONFIG_WHITESPACE      " \t\n"
#define CONFIG_COMMENT_CHAR    '#'

/** Used for holding the context of a config lookup
    for functions needing to share state */
struct ConfigContext {
  vector<vector<string> > *curLines;
  int curLine;
  int curEl;
  const char *curID;
};

/** Config object.  Create one of these to parse a config file
    Config files are pure ASCII where each line starts with an ID
    followed by a number of data values. 
 */
class Config {
  map<string,vector<vector<string> > > data;

public:
  /** creates empty config object */
  Config() {}
  /** loads and parses given file */
  Config (const char *filename);

  /** load and parse given file */
  bool load (const char *filename);
  /** save current data to file */
  bool save (const char *filename);

  // functions to add values to new IDs
  void putString (const char *id, const char *str) {
    ConfigContext c;
    newID (id, &c);
    addString (str, &c);
  }
  void putInt (const char *id, int i) {
    ConfigContext c;
    newID (id, &c);
    addInt (i, &c);
  }
  void putFloat (const char *id, float f) {
    ConfigContext c;
    newID (id, &c);
    addFloat (f, &c);
  }
  void putChar (const char *id, char c) {
    ConfigContext context;
    newID (id, &context);
    addChar (c, &context);
  }

  // functions to add values to the current ID
  // remember to call newID() first
  void addString (const char *str, ConfigContext *c);
  void addInt (int i, ConfigContext *c);
  void addFloat (float f, ConfigContext *c);
  void addChar (char c, ConfigContext *context);

  void newID (const char *id, ConfigContext *c);
  void deleteID (const char *id);

  /** return CRC of current object */
  int getCRC ();
  
  /** true if given ID exists */
  bool exists (const char *id) {
    ConfigContext c;
    return findID (id, &c);
  }

  /** find ID (for subsequent calls to the context dependent get methods
      call this to set current ID
   */
  bool findID (const char *id, ConfigContext *c) {
    if (data.count (id) > 0) {
      c->curLines = &data[id];
      c->curLine = 0;
      c->curEl = 0;
      c->curID = id;
      return true;
    } else {
      return false;
    }
  }

  /** return number of lines with given ID */
  int countID (const char *id) {
    if (data.count (id) > 0) {
      return data[id].size();
    } else {
      panic ("Parse error: %s", id);
    }
    return -1;
  }

  // methods parsing the next data item for the current ID
  // call findID() first
  const char *getString (ConfigContext *c) {
    try {
      vector<string> *elements = &c->curLines->at (c->curLine);
      return (elements->at (c->curEl++)).c_str();
    } catch (out_of_range o) {
      panic ("Parse error: %s", c->curID);
    }
    return NULL;
  }
  int getInt (ConfigContext *c) {
    return strtol (getString (c), NULL, 10);
  }
  int getHex (ConfigContext *c) {
    return strtol (getString (c), NULL, 16);
  }
  float getFloat (ConfigContext *c) {
    return strtod (getString (c), NULL);
  }
  char getChar (ConfigContext *c) {
    return *getString (c);
  }

  // methods parsing the first data item of the given ID
  const char *getString (const char *id) {
    ConfigContext c;
    const char *r = NULL;
    if (findID (id, &c)) r = getString (&c);
    else panic ("Parse error: %s", id);
    return r;
  }
  int getInt (const char *id) {
    ConfigContext c;
    int r = -1;
    if (findID (id, &c)) r = getInt (&c);
    else panic ("Parse error: %s", id);
    return r;
  }
  int getHex (const char *id) {
    ConfigContext c;
    int r = -1;
    if (findID (id, &c)) r = getHex (&c);
    else panic ("Parse error: %s", id);
    return r;
  }
  float getFloat (const char *id) {
    ConfigContext c;
    float r = -1;
    if (findID (id, &c)) r = getFloat (&c);
    else panic ("Parse error: %s", id);
    return r;
  }
  char getChar (const char *id) {
    ConfigContext c;
    char r = '\0';
    if (findID (id, &c)) r = getChar (&c);
    else panic ("Parse error: %s", id);
    return r;
  }

  // true if there are more data elements for the current ID
  bool moreEls (ConfigContext *c) {
    return (int)(c->curLines->at (c->curLine)).size() > c->curEl;
  }

  // get next line that has the same ID as the current one
  bool next (ConfigContext *c) {
    c->curLine++;
    c->curEl = 0;
    return (int)c->curLines->size() > c->curLine;
  }
};

#endif

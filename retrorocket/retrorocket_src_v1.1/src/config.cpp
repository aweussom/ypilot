/*
  config.cpp
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

#include "config.h"

Config::Config (const char *filename) {
  if (!load (filename)) {
    panic ("Can't open %s", filename);
  }
}

bool Config::load (const char *filename) {
  FILE *fp;

  if (!(fp = fopen (filename, "rb"))) {
    return false;
  }

  char line[CONFIG_LINESIZE];
  while (fgets (line, CONFIG_LINESIZE, fp)) {
    vector<vector<string> > *lines;

    char *id = strtok (line, CONFIG_WHITESPACE);
    if (id) {
      if (id[0] == CONFIG_COMMENT_CHAR) {
        continue;
      }
      if (!strcmp (id, CONFIG_INCLUDE_KEYWORD)) {
        if (!load (strtok (NULL, CONFIG_WHITESPACE))) {
          panic ("Parse error: " CONFIG_INCLUDE_KEYWORD);
        }
        continue;
      } 
      lines = &data[id];
      vector<string> elements;
      char *el;
      while ((el = strtok (NULL, CONFIG_WHITESPACE))) {
        if (el[0] == CONFIG_COMMENT_CHAR) {
          break;
        }
        elements.push_back (el);
      }
      lines->push_back (elements);
    }
  }

  fclose (fp);

  return true;
}

bool Config::save (const char *filename) {
  FILE *fp;

  if (!(fp = fopen (filename, "wb"))) {
    return false;
  }

  map<string,vector<vector<string> > >::iterator dataIter = data.begin();
  while (dataIter != data.end()) {
    string id = dataIter->first;
    vector<vector<string> > *curLines = &dataIter->second;
    vector<vector<string> >::iterator lineIter = curLines->begin();
    while (lineIter != curLines->end()) {

      fprintf (fp, "%s", id.c_str());

      vector<string> *elements = &(*lineIter);
      vector<string>::iterator elIter = elements->begin();
      while (elIter != elements->end()) {
        fprintf (fp, " %s", (*elIter).c_str());
        elIter++;
      }
      fprintf (fp, "\n");
      lineIter++;
    }
    dataIter++;
  }

  fclose (fp);

  return true;
}

void Config::addString (const char *str, ConfigContext *c) {
  // TODO: er det trygt å sende inn en streng som går ut av skop?
  vector<string> *elements = &c->curLines->at (c->curLine);
  elements->push_back (str);
}

void Config::addInt (int i, ConfigContext *c) {
  char str[CONFIG_LINESIZE];
  sprintf (str, "%d", i);
  addString (str, c);
}

void Config::addFloat (float f, ConfigContext *c) {
  char str[CONFIG_LINESIZE];
  sprintf (str, "%f", f);
  addString (str, c);
}

void Config::addChar (char c, ConfigContext *context) {
  char str[CONFIG_LINESIZE];
  sprintf (str, "%c", c);
  addString (str, context);
}

void Config::newID (const char *id, ConfigContext *c) {
  c->curLines = &data[id];
  vector<string> elements;
  c->curLines->push_back (elements);
  c->curLine = c->curLines->size() - 1;
  c->curEl = 0;
}

void Config::deleteID (const char *id) {
  if (exists (id)) {
    map<string,vector<vector<string> > >::iterator i = data.find (id);
    data.erase (i);
  }
}

int Config::getCRC () {

  int CRC = 0;
  map<string,vector<vector<string> > >::iterator dataIter = data.begin();

  while (dataIter != data.end()) {
    string id = dataIter->first;
    vector<vector<string> > *curLines = &dataIter->second;
    vector<vector<string> >::iterator lineIter = curLines->begin();
    while (lineIter != curLines->end()) {
      CRC += str2CRC (id);
      
      vector<string> *elements = &(*lineIter);
      vector<string>::iterator elIter = elements->begin();
      while (elIter != elements->end()) {
        CRC += str2CRC (*elIter);
        elIter++;
      }
      lineIter++;
    }
    dataIter++;
  }
  return CRC;
}

int str2CRC (string str) { 
  int CRC = 0;
  string::iterator strIter = str.begin();

  while (strIter != str.end()) {
    CRC += (int) *strIter;
    strIter++;
  }
  return CRC;
}


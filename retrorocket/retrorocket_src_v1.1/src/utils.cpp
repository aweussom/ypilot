/*
  utils.cpp
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
#include <sys/stat.h>

int getFileSize (const char *filename) {
  FILE *fp = fopen (filename, "r");
  struct stat mystat;
  fstat (fileno (fp), &mystat);
  return mystat.st_size;
}

Fix32 getCurrentRanking() {
  Config c;
  c.load (HIGHSCOREFILE);
  ConfigContext configContext;
  if (c.findID (retrorocket->fieldFilename, &configContext)) {
    return (Fix32)c.getFloat (&configContext);
  } else {
    return DEFAULT_RANKING;
  }
}

void waitKey (int key) {
  if(retrorocket->bot)
    return;
  
  // wait for key release
  do {
    waitVBL();
  } while (sysGetInput() || getPointer());
  
  // wait for key press
  do {
    waitVBL();
  } while ( (!(sysGetInput() & key)) && (!getPointer()) );
 
  // wait until pointer register as released
  if (getPointer()) {
    do {
      waitVBL();
    } while (getPointer());
  }
}

bool matchExt (const char *filename, const char *ext) {
  int lenExt = strlen (ext), lenFN = strlen (filename);
  return ( ! strcmp ( filename + lenFN - lenExt, ext ) );
}

void message (const char *fmt, ...) {
  message ((int)(ROWS/2), fmt);
}

void message (int y, const char *fmt, ...) {
  va_list ap;
  va_start (ap, fmt);

  char line[LINESIZE];
  vsprintf (line, fmt, ap);

  outputText ((COLUMNS - strlen (line)) / 2,
              y,
              line);

  setTextColor (BLUE);
  const char *msg = "(Press A to continue)";
  outputText ((COLUMNS - strlen (msg)) / 2, y + 1, msg);

  waitKey (A);
  clearText();

  va_end (ap);
}

void infoText (const char *fmt, ...) {
  va_list ap;
  va_start (ap, fmt);

  char line[LINESIZE];
  vsprintf (line, fmt, ap);

  outputText ((COLUMNS - strlen (line)) / 2,
              ROWS / 2,
              line);

  va_end (ap);
}

int readBinary (u8 **buffer, const char *filename) {
  FILE *fp;
  
  // get file size
  int fileSize = getFileSize (filename);

  if (!(*buffer = (u8*)malloc (fileSize))) {
    panic ("Out of memory");
  }

  if ((fp = fopen (filename, "rb"))) {
    if ( fileSize > (int)fread ( *buffer, 1, fileSize, fp ) )
      panic ( "File read error" );
  } else {
    panic ( "Can not open %s", filename );
  }
  fclose (fp);

  if (retrorocket) {
    u8 *bufferptr = *buffer;
    for (int i=0; i < fileSize; bufferptr++, i++) {
      retrorocket->myCRC += (int)*bufferptr;
    }
  }

  return fileSize;
}

void rotatePoint (Fix32 x, Fix32 y, Fix32 angle, Fix32 *xx, Fix32 *yy ) {
  Fix32 c = angle.cosinus_ds();
  Fix32 s = angle.sinus_ds();
  *xx = c * x - s * y;
  *yy = s * x + c * y;
}

void panic (const char *fmt, ...) {
  va_list ap;
  va_start (ap, fmt);

  clearText();

  char line[LINESIZE];
  vsprintf (line, fmt, ap);

  const char *panicMsg = "Panic!";

  setTextColor (RED);

  outputText ((COLUMNS - strlen (panicMsg)) / 2,
              (ROWS / 2) - 1,
              panicMsg);

  if (strlen (line) >= COLUMNS) {
    line[COLUMNS-1] = 0;
    outputText (0, ROWS / 2, line);
  } else {
    outputText ((COLUMNS - strlen (line)) / 2,
                ROWS / 2,
                line);
  }

  va_end (ap);

  while (true);
}

// strips suffix from str, if it is the suffix of string
string stripSuffix (string str, string suffix) {
  if (str.substr (str.length() - suffix.length(), str.length())
      == suffix) {
    return str.substr( 0, str.length() - suffix.length());
  } else {
    return str;
  }
}


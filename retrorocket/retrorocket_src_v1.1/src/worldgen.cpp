/*
  worldgen.cpp
  Copyright 2008 Kyrre Glette, Asbjørn Djupdal and Morten Hartmann

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

///////////////////////////////////////////////////////////////////////////////

void WorldGen::previewAutogen (Menu *menu) {
  if (retrorocket->worldgen.autogenBuffer) {
    drawBM (retrorocket->worldgen.autogenBuffer,
            retrorocket->worldgen.width, retrorocket->worldgen.height,
            retrorocket->worldgen.seaLevel,
            0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - 16, false);
    retrorocket->hideSplash();
    showMap();
  }
}

void WorldGen::createNewAutogen (Menu *menu) {
  retrorocket->fieldSeed = rand();
  srand (retrorocket->fieldSeed);
  retrorocket->worldgen.width = retrorocket->worldgen.width_tmp;
  retrorocket->worldgen.height = retrorocket->worldgen.height_tmp;
  retrorocket->worldgen.generate();
  previewAutogen (NULL);
}

void WorldGen::generate() {
  if (autogenBuffer) free (autogenBuffer);
  autogenBuffer = (u8*)malloc (height * width);
  if (!autogenBuffer) panic ("Out of memory");
  createBMField (autogenBuffer, width, height, grainy);
}

void WorldGen::placeAutogen (Menu *menu) {
  if (retrorocket->worldgen.autogenBuffer) {
    initGameGfx();
    retrorocket->worldgen.removeThinLinesFilter (retrorocket->worldgen.autogenBuffer,
                                          retrorocket->worldgen.width,
                                          retrorocket->worldgen.height);
    retrorocket->worldgen.platforms.clear();

    drawBM (retrorocket->worldgen.autogenBuffer,
            retrorocket->worldgen.width, retrorocket->worldgen.height,
            retrorocket->worldgen.seaLevel,
            0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - 16);
    showMap();

    for (int i = 0; i < retrorocket->worldgen.players; i++) {
      clearText();
      outputText (2, ROWS-1, "Place platform %d with stylus", i + 1);
      RRPoint p;
      int x, y;
      getMapClick (&x, &y);
      p.x = x;
      p.y = y;
      retrorocket->worldgen.platforms.push_back (p);
    }

    hideMap();

    RetroRocket::playAutogen (menu);
  }
}

void WorldGen::createBMField (u8* buffer, int xSize, int ySize, int grainy) {
  int cornerValueRange=180;

  int ulc = rand() % cornerValueRange;
  int urc = rand() % cornerValueRange;
  int llc = rand() % cornerValueRange;
  int lrc = rand() % cornerValueRange;

  plotRecursive (buffer, grainy, 0,
                 FPoint(0,0,ulc), FPoint(xSize-1,0,urc),
                 FPoint(0,ySize-1,llc), FPoint(xSize-1,ySize-1,lrc), xSize);
  addBorder(buffer, xSize, ySize);
}

///////////////////////////////////////////////////////////////////////////////

void WorldGen::plotRecursive(u8* buffer,int graininessLevel,int iteration,FPoint ul,FPoint ur,FPoint ll,FPoint lr, int xSize) //upper left, up.right, lower left, l.right
{
  //do some plotting for preview
  /*plotPoint(buffer,ul); plotPoint(buffer,ur); plotPoint(buffer,ll); plotPoint(buffer,lr);*/

  //termination criterium
  int width=ur.x-ul.x;
  int height=ll.y-ul.y;
  if(width<=1 && height<=1) {
    int c=ul.c;						//take any of the corners - they're at the same place now
    if(c<0) c=0;
    if(c>255) c=255;				//clamp to the dynamic range of the buffer
    buffer[ul.y*xSize+ul.x]=c; 		//plot pixel
    return;
  }

  //add a graininess constant to this term later
  int grainy=((rand()%graininessLevel)-graininessLevel/2)/k_ipow(2,iteration);

  //create new "middle" points
  int midX=ul.x+width/2;
  int midY=ul.y+height/2;
  FPoint ml(ul.x,midY,(ul.c+ll.c)/2); //mid left
  FPoint mr(ur.x,midY,(ur.c+lr.c)/2); //mid right
  FPoint mt(midX,ul.y,(ul.c+ur.c)/2); //mid top
  FPoint mb(midX,ll.y,(ll.c+lr.c)/2); //mid bot
  FPoint mm(midX,midY,(ul.c+ll.c+ur.c+lr.c)/4+grainy); //middle

  plotRecursive(buffer,graininessLevel,iteration+1,ul,mt,ml,mm, xSize); //upper left box
  plotRecursive(buffer,graininessLevel,iteration+1,mt,ur,mm,mr, xSize); //upper right box
  plotRecursive(buffer,graininessLevel,iteration+1,ml,mm,ll,mb, xSize); //lower left box
  plotRecursive(buffer,graininessLevel,iteration+1,mm,mr,mb,lr, xSize); //lower right box
}

void WorldGen::addBorder(u8* buffer,int width,int height)
{
  for(int x=0;x<width;x++) {
    buffer[x]=255;
    buffer[width*height-1-x]=255;
  }
  for(int y=0;y<height;y++) {
    buffer[y*width+0]=255;
    buffer[y*width+width-1]=255;
  }
}

void WorldGen::removeThinLinesFilter(u8* buffer,int width,int height)
{
  //horizontal remove
  for(int x=1;x<width*height-1;x++) {
    if(buffer[x-1]<=seaLevel && buffer[x+1]<=seaLevel)
      buffer[x]=buffer[x-1];
  }
  //vertical remove
  for(int x=width;x<width*height-width;x++) {
    if(buffer[x-width]<=seaLevel && buffer[x+width]<=seaLevel)
      buffer[x]=buffer[x-width];
  }

  //horizontal add
  for(int x=1;x<width*height-1;x++) {
    if(buffer[x-1]>seaLevel && buffer[x+1]>seaLevel)
      buffer[x]=buffer[x-1];
  }
  //vertical add
  for(int x=width;x<width*height-width;x++) {
    if(buffer[x-width]>seaLevel && buffer[x+width]>seaLevel)
      buffer[x]=buffer[x-width];
  }
}


/*
  Iphone implementation of system...

 */
#include "retrorocket.h"
#include "system.h"

// General
//int main (int argc, char **argv){return 1;}
void waitVBL(){}
void initIntroGfx(){}
void initGameGfx(){}
char *getUserName(){return "";}

// Sprite
void setFocus (Ship *focus){}
void createSprite (void *pal, void *gfx, int spritenum){}
void createParticleSprite (int spritenum){}
void deleteSprite (int spritenum){}
void moveSprite (Fix32 x, Fix32 y, int spritenum){}
void hideSprite (int spritenum){}
void setSpriteRotEnable (int spritenum){}
void rotateSprite (int spritenum, Fix32 angle){}
void drawSOBsOnMap(){}
void hideMapSprite (int spritenum){}
void showMapSprite (int spritenum){}

// Action backgrounds
void loadBg (int *info,
             const unsigned short *map,
             int sizeofMap, const unsigned char *tiles,
             int sizeofTiles, const unsigned short *pal, int bg){}
void deleteBg (int bg){}
void loadBgPal (int bg, int palSlot, const unsigned short *pal){}
void setBgTile (int bg, Fix32 x, Fix32 y, int tile, int palSlot){}
void showBG (int bg){}
void hideBG (int bg){}

// Map
void drawBM (u8 *buffer, int w, int h, int threshold,
             int left, int top, int width, int height, bool bw){}
void drawMap (u16 *map, u8 *tiles, int *info, int w, int h,
              int left, int top, int width, int height){}
void hideMap(){}
void showMap(){}

// Text
void clearText(){}
void clearText (int from, int to){}
void setTextColor (int r, int g, int b){}
void setTextColor (int c){}
void outputText (int x, int y, const char *fmt, ...){}

// File I/O
int getFileSize (const char *filename){return 1;}
void getFileList (const char *ext, vector<char*>*filenames){}

// Sound
void playSound (u8 *data, int size, int volume,  Fix32 x, Fix32 y,
                int rate){}
void playSound (u8 *data, int size, int volume,
                int rate, int panning){}
void playSoundThrust (u8 *data, int size){}
void modifySoundThrust (int rate, int volume){}

// Music
void loadMod (const char *filename, 
              int randomStartPos, int randomPatterns, int blankPos){}
void freeMod(){}
void playMod (int pos){}
void playRandomMod(){}
void stopMod(){}

// Network
Network *openNetworkMaster (char *fieldFilename, int min, int max,
                            bool autogen){return NULL;}
Network *openNetworkSlave (char *fieldFilename){return NULL;}
void setWirelessChannel (int i){}

// Input
unsigned int sysGetInput(){return 1;}
void readLine (void (*cb)(char *s), const char *str){}
void getMapClick (int *x, int *y){}
bool getPointer (int *x, int *y){return false;}
bool getPointer() {return false;}

// Other
int startCritical(){return 1;}
void endCritical (int ime){}

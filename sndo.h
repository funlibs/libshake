#ifndef SNDO_H
#define SNDO_H

int  sndoInit(float suggestedLatency);
int  sndoLoad(char* fileName);
void sndoPlay(int soundId);
void sndoTerminate();

#endif

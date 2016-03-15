#ifndef SNDO_H
#define SNDO_H

#include "portaudio.h"
#include "wave.h"

int sndoInit(char* fileName);
void sndoTerminate();

#endif

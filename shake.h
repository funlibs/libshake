#ifndef SHAKE_H
#define SHAKE_H

int  shakeInit(float suggestedLatency);
int  shakeLoad(char* fileName);
void shakePlay(int soundId);
void shakeTerminate();

#endif

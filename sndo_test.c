#include "sndo.h"
#ifdef WIN32
#include <windows.h>
void doSleep(int duration) { Sleep(duration * 1000); }
#else
#include <unistd.h>
void doSleep(int duration) { sleep(duration); }
#endif
int main(int argc, char* argv[])
{

    sndoInit();
    int soundId = sndoLoad("test.wav");
    sndoPlay(soundId);
    doSleep(3);
    sndoPlay(soundId);
    doSleep(2);
    sndoPlay(soundId);
    doSleep(1);
    sndoPlay(soundId);
    doSleep(2);
    sndoPlay(soundId);
    doSleep(3);
    sndoPlay(soundId);
    doSleep(1);
    sndoPlay(soundId);
    doSleep(2);

    sndoTerminate();

}

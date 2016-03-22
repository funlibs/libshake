#include "sndo.h"
#include <unistd.h>

int main(int argc, char* argv[])
{

    sndoInit();
    int soundId = sndoLoad("test.wav");
    sndoPlay(soundId);
    sleep(3);
    sndoPlay(soundId);
    sleep(2);
    sndoPlay(soundId);
    sleep(1);
    sndoPlay(soundId);
    sleep(2);
    sndoPlay(soundId);
    sleep(3);
    sndoPlay(soundId);
    sleep(1);
    sndoPlay(soundId);
    sleep(2);

    sndoTerminate();

}

#include "shake.h"
#ifdef WIN32
#include <windows.h>
void doSleep(int duration) { Sleep(duration * 1000); }
#else
#include <unistd.h>
void doSleep(int duration) { sleep(duration); }
#endif
int main(int argc, char* argv[])
{

    shakeInit(0.2);
    int soundId = shakeLoad("test.wav");
    shakePlay(soundId);
    doSleep(3);
    shakePlay(soundId);
    doSleep(2);
    shakePlay(soundId);
    doSleep(1);
    shakePlay(soundId);
    doSleep(2);
    shakePlay(soundId);
    doSleep(3);
    shakePlay(soundId);
    doSleep(1);
    shakePlay(soundId);
    doSleep(2);

    shakeTerminate();

}

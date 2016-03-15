#include "sndo.h"
#include <unistd.h>

int main(int argc, char* argv[])
{
    sndoInit(argv[1]);
    sleep(3);
    sndoTerminate();

}

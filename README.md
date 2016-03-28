LibShake
========
[![Build Status](https://travis-ci.org/ssbx/libshake.svg?branch=master)](https://travis-ci.org/ssbx/libshake)
[![Build status](https://ci.appveyor.com/api/projects/status/9fd4q1t27bej41ox/branch/master?svg=true)](https://ci.appveyor.com/project/ssbx/libshake/branch/master)

Very limited mixer using PortAudio and [LibWave](https://github.com/ssbx/libwave). Will only mix wav files with 16 bits/sample at 44100hz for 2 channels.

Build
-----
First init Git submodules:
```sh
git submodule init
git submodule update
```

LibShake is build with CMake for all platforms.

You can also use the fake configure script provided to set it up on unix:
```sh
./configure
make
```

Demo
----
```sh
./build/shake_example
```

Doc
---
Depends on Doxygen.
```sh
make shake_doc
```

Example
-------
LibShake use portaudio asynchronous callback to play files.

```c
[...]

shakeInit(0.20); // 0.20 suggested latency

int soundId1 = shakeLoad("mywavfile1.wav"); // wav must be 16bits/samples 44100samples/seconds 2 channels
int soundId2 = shakeLoad("mywavfile2.wav"); // wav must be 16bits/samples 44100samples/seconds 2 channels

shakePlay(soundId1);
sleep(1);

shakePlay(soundId2);
sleep(1);

shakePlay(soundId1);
shakePlay(soundId1);
sleep(4);

shakeTerminate();

[...]

```

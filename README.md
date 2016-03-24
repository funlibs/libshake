LibShake
========
[![Build Status](https://travis-ci.org/ssbx/libshake.svg?branch=master)](https://travis-ci.org/ssbx/libshake)
[![Build status](https://ci.appveyor.com/api/projects/status/9fd4q1t27bej41ox/branch/master?svg=true)](https://ci.appveyor.com/project/ssbx/libshake/branch/master)

Very limited mixer. Only load wav files with 16 bits/sample at 44100hz for 2 channels.

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
$ ./configure
$ cd build && make all
```

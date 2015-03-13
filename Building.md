# Needed dependencies #

ddcn needs the following dependencies (in parentheses the version which has been tested, others might or might not work):
  * [Ariba](http://ariba-underlay.org/) (0.7.0)
  * [MCPO](http://ariba-underlay.org/) (0.5.1)
  * cmake
  * Qt 4 core, gui, dbus
  * OpenSSL (0.9.8, 1.0.0)
  * GCC (not necessary for building, but the only supported compiler for running ddcn)

It so far only has been tested on Ubuntu 10.10 and Fedora 15, on x86 and x86\_64.

# Building #

Assuming that your current directory is the source directory of ddcn, containing the directories "ddcn\_service", "ddcn\_gcc", "ddcn\_control" etc., the following steps will compile ddcn:

```
mkdir build
cd build
cmake ..
make
```

Optionally, you can install the binaries system wide using
```
sudo make install
```

Otherwise, the binaries have been created in build/bin.
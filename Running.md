# Calling the compiler #

The project provides a drop-in replacement for the GNU gcc compiler (or more precisely, for the `gcc` and `g++` programs from that compiler suite) which can compile C and C++. The replacement for `gcc` is `bin/ddcn_gcc`, while `ddcn_g++` replaces the C++ compiler. `ddcn_g++` is not located in the output directory but in the `ddcn_gcc/` source directory. This utility is a simple shell script which calls the C compiler replacement with the environment variable `DDCN_LANGUAGE` set to `c++`.
Note: However, compiling fails if no local network node is running because everything actually happens inside of this central process. Only command line parameters and `stdin` data are transferred.

In order to compile an existing project with one of these compilers you have to make sure their paths are both somewhere in your `PATH` variable. Then you can just select different `CC` and `CXX` program names. Assuming the sources are saved in `/home/spovnet/ddcn` while the output files are in `/home/spovnet/ddcn/build` as described in the readme, you can compile an autotools project with the following lines:

```
export PATH=/home/spovnet/ddcn/build/bin:/home/spovnet/ddcn/ddcn_gcc:$PATH
CC=ddcn_gcc CXX=ddcn_g++ ./configure
make -j20
```

Note the huge value passed to the `-j` option - this is not dangerous but even necessary to let ddcn pass some of the processes to other peers in the network as it always will try to compile them first and only delegates them if it has more jobs waiting than the user specified maximum value of parallel jobs.
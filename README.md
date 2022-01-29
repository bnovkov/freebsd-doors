# General
This repository contains a completely new, Illumos-compatible implementation of the Solaris Doors IPC mechanism for FreeBSD.  
The patch is compatible with commit [85b7c566f153](https://github.com/freebsd/freebsd-src/commit/85b7c566f1538f9a2e85f76bf5b41380701987a7).

## A brief overview of Solaris Doors
Solaris doors are a form of lightweight inter-process communication which allows a thread to invoke a function from another process, essentially mimicking a local remote procedure call. A serving process may expose its service by creating a door which is then exported in the filesystem. A client may then invoke the exposed procedure using a "door call" operation, during which the kernel chooses a thread in the serving process which will execute the procedure. The serving thread will invoke a "door return" operation upon returning which will transfer any results to the calling thread.

For more details refer to the following sources:
- [Revolving door](https://github.com/robertdfrench/revolving-door)
  - A great tutorial and overview, along with a collection of good sources
- [Oracle® Solaris 11.2 Programming Interfaces Guide](https://docs.oracle.com/cd/E36784_01/html/E36861/gmhhn.html)

## Building
```
# Obtain a copy of FreeBSD-CURRENT (14.0 used in implementation)
$ git clone https://github.com/freebsd/freebsd-src
$ git checkout 85b7c566f1538f9a2e85f76bf5b41380701987a7

$ cd freebsd-src
$ git apply diff 
OR
$ patch < diff

$ make -jN buildkernel KERNCONF=DOORS-DEV
$ make installkernel KERNCONF=DOORS-DEV

```

## Doors library
The `libdoor` folder contains sources of a (tentative) library which exposes common door operations and door syscalls.

## Sample code and tests
Several tests are located in the `doors-test` folder, along with an example of a basic client and server programs in `doors-test/basic`

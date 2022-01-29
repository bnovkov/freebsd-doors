#pragma once

#include <door.h>
#include <unistd.h>
#include <sys/types.h>

#define ASSERT(expr, msg)                                               \
  do {                                                                  \
    if(!(expr)){                                                        \
      printf("Assertion failed at line %d in %s: %s", __LINE__, __FILE__, (msg)); \
      exit(-1);                                                         \
    }                                                                   \
  }while(0)                                                             \



#define DOOR_ECHO 0
#define DOOR_HASH_ECHO 1
#define DOOR_LEAK_CHECK 2
#define DOOR_FD_RECV 3
#define DOOR_FD_SEND 4
#define DOOR_UNREF_TEST 5
#define DOOR_UNREF_MULTI_TEST 6

pid_t door_create_helper(int proc_no, void *udata, u_int attributes, const char *path);

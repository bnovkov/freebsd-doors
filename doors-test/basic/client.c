#include <stdio.h>
#include <string.h>
#include <door.h>
#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys/mman.h>

int main()
{
  const char* test = "asdf";
  const char* path = "/home/bojan/server.door";



	int door_fd = open(path, O_RDONLY);
	if (door_fd == -1){
    perror("Could not open door");
    return -1;
  }

  door_arg_t *args = (door_arg_t*)calloc(1, sizeof(door_arg_t));
  if(!args){
    close(door_fd);
    perror("args malloc");
    return -1;
  }
  printf("Args: %p\n", args);

  
  char *rbuf = (char *)calloc(64, 1);
  if(!rbuf){
    close(door_fd);
    free(args);
    perror("rbuf malloc");
    return -1;
  }
  
  args->data_ptr = (char *)calloc(4096 * 6, 1);
  args->data_size = 4096 * 6;
  printf("Data: %p\n", args->data_ptr);
  
  strlcpy(args->data_ptr, test, 4096);
  memset(args->data_ptr + 4096 - 12, 'A', 12);
  //  args->rbuf = rbuf;
  //args->rsize = 64;
  
  printf("Client: invoking door_call()\n");
  int error = door_call(door_fd, args);
  if(error){
    perror("door_call");
    return -1;
  }

  printf("%p\n", args->rbuf);
  if(args->rbuf){
    printf("Response: %s\n", args->rbuf);
    munmap(args->rbuf, args->rsize);
  }
  printf("Client: passed door_call()\n");

  free(args);

  return 0;
}

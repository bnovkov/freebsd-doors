#include "door_helper.h"

#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/stat.h>
#include <pthread.h>

static void door__argument_copy_rbuf_thread(void) {
  const char *path = "/home/bojan/parallel_echo.door";
  const char *test_value = "123456";


	int door_fd = open(path, O_RDONLY);
	if (door_fd == -1){
    perror("Could not open door");
    return;
  }

    door_arg_t *args = (door_arg_t*)calloc(1, sizeof(door_arg_t));
    if(!args){
      close(door_fd);
      perror("args malloc");
      exit(-1);
    }


    char *rbuf = (char *)calloc(64, 1);
    if(!rbuf){
      close(door_fd);
      free(args);
      perror("rbuf malloc");
      return;
    }

    args->data_ptr = (char *)calloc(strlen(test_value) + 1, 1);
    args->data_size = strlen(test_value) + 1;

    strlcpy(args->data_ptr, test_value, args->data_size);

    args->rbuf = rbuf;
    args->rsize = 64;

    int error = door_call(door_fd, args);
    if(error){
      perror("door_call");
      return;
    }

    ASSERT(strncmp(args->rbuf, test_value, args->rsize) == 0, "Received invalid value from door server");


    args->rbuf  = NULL;
    free(rbuf);
    free(args);
  return;
}

static void parallel_echo_test(void){
  int thread_num = 10;
  pthread_t threads[10];

  for (int i=0; i<thread_num; i++){
    int error = pthread_create(&threads[i], NULL, door__argument_copy_rbuf_thread, NULL);
    if(error) {
      perror("thread_create");
      return ;
    }
  }

  for(int i=0; i<thread_num; i++){
    pthread_join(threads[i], NULL);
  }
}

static void*  __default_thread(void* args){
  int error;

  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

  error = door_return(NULL, 0, NULL, 0);
  if (error){
    perror("door_return");
    exit(-1);
  }

  return NULL;
}

static void parallel_server_create(door_info_t *info){
  int thread_num = 10;
  pthread_t threads[10];

  for (int i=0; i<thread_num; i++){
    int error = pthread_create(&threads[i], NULL, __default_thread, NULL);
    if(error) {
      perror("thread_create");
      return ;
    }
  }
}

int main(void){

  const char *parallel_echo_path = "/home/bojan/parallel_echo.door";

  door_server_create(parallel_server_create);

  pid_t pid_echo = door_create_helper(DOOR_ECHO, NULL, 0, parallel_echo_path);
  if(pid_echo < 0){
    perror("door_create_helper parallel_echo");
    exit(-1);
  }

  sleep(1);


  printf("Running parallel echo test...");
  parallel_echo_test();

  kill(pid_echo, SIGINT);

}

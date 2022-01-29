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

#include "fnv/fnv.h"



void door__argument_copy_rbuf(void) {
  const char *path = "/home/bojan/echo.door";
  const char *test_value = "123456";


	int door_fd = open(path, O_RDONLY);
	if (door_fd == -1){
    perror("Could not open door");
    exit(-1);
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
    exit(-1);
  }

  args->data_ptr = (char *)calloc(strlen(test_value) + 1, 1);
  args->data_size = strlen(test_value) + 1;

  strlcpy(args->data_ptr, test_value, args->data_size);

  args->rbuf = rbuf;
  args->rsize = 64;

  int error = door_call(door_fd, args);
  if(error){
    perror("door_call");
    exit(-1);
  }


  ASSERT(strncmp(args->rbuf, test_value, args->rsize) == 0, "Received invalid value from door server");


  args->rbuf  = NULL;
  free(rbuf);
  free(args);


  return;
}


void door__argument_copy_no_rbuf(void) {
  const char *path = "/home/bojan/echo.door";
  const char *test_value = "123456";


	int door_fd = open(path, O_RDONLY);
	if (door_fd == -1){
    perror("Could not open door");
    exit(-1);
  }

  door_arg_t *args = (door_arg_t*)calloc(1, sizeof(door_arg_t));
  if(!args){
    close(door_fd);
    perror("args malloc");
    exit(-1);
  }



  args->data_ptr = (char *)calloc(strlen(test_value) + 1, 1);
  args->data_size = strlen(test_value) + 1;

  strlcpy(args->data_ptr, test_value, args->data_size);


  int error = door_call(door_fd, args);
  if(error){
    perror("door_call");
    exit(-1);
  }


  ASSERT(args->rbuf, "A result buffer was not allocated");
  ASSERT(strncmp(args->rbuf, test_value, args->rsize) == 0, "Received invalid value from door server");


  munmap(args->rbuf, args->rsize);
  args->rbuf  = NULL;
  free(args);


  return;
}


void door__argument_transfer_rbuf_shared_mapping(void) {
  const char *path = "/home/bojan/hash_echo.door";


	int door_fd = open(path, O_RDONLY);
	if (door_fd == -1){
    perror("Could not open door");
    exit(-1);
  }

  door_arg_t *args = (door_arg_t*)calloc(1, sizeof(door_arg_t));
  if(!args){
    close(door_fd);
    perror("args malloc");
    exit(-1);
  }


  size_t data_size = 4096 * 6;

  args->data_ptr = (char *)malloc(4096 * 6);
  args->data_size = data_size;


  char *rbuf = (char *)calloc(sizeof(Fnv64_t), 1);
  if(!rbuf){
    close(door_fd);
    perror("rbuf malloc");
    exit(-1);
  }

  args->rbuf = rbuf;
  args->rsize = sizeof(Fnv64_t);

  int urand_fd = open("/dev/urandom", O_RDONLY);
  if(urand_fd < 0){
    perror("/dev/urandom open");
    exit(-1);
  }
  /*
  size_t total_read = 0;

  while(total_read < data_size){
    int bytes_read = read(urand_fd, args->data_ptr + total_read, ((data_size - total_read) < 4096 ? (data_size - total_read) : 4096));
    if(bytes_read < 0){
      perror("/dev/urandom read");
      exit(-1);
    }

    total_read += bytes_read;
  }
  */
  size_t cnt = 0;
  char c = 'A';
  while(cnt < data_size){
    if(cnt && ((cnt & 0xFFF ) == 0)){
        c++;
    }
    args->data_ptr[cnt] = c;

    cnt++;
  }

  Fnv64_t hval = fnv_64a_buf((void *)args->data_ptr, data_size, 0);

  int error = door_call(door_fd, args);
  if(error){
    perror("door_call");
    exit(-1);
  }

  ASSERT(args->rbuf, "Previously allocated rbuf was somehow changed to NULL");
  ASSERT(args->rbuf == rbuf, "Previously allocated rbuf was somehow changed");
  ASSERT(args->rsize == sizeof(Fnv64_t), "Previously allocated rbuf size was changed");
  ASSERT(hval == *((Fnv64_t *)args->rbuf), "Received invalid FNV1a hash value from door server");

  free(args->rbuf);
  args->rbuf  = NULL;
  free(args);


  return;
}

void door__argument_transfer_rbuf_shared_mapping_leak_check(void) {
  const char *path = "/home/bojan/hash_echo.door";


	int door_fd = open(path, O_RDONLY);
	if (door_fd == -1){
    perror("Could not open door");
    exit(-1);
  }

  door_arg_t *args = (door_arg_t*)calloc(1, sizeof(door_arg_t));
  if(!args){
    close(door_fd);
    perror("args malloc");
    exit(-1);
  }


  size_t data_size = 4096 * 6;

  args->data_ptr = (char *)malloc(4096 * 6);
  args->data_size = data_size;


  char *rbuf = (char *)calloc(sizeof(Fnv64_t), 1);
  if(!rbuf){
    close(door_fd);
    perror("rbuf malloc");
    exit(-1);
  }

  args->rbuf = rbuf;
  args->rsize = sizeof(Fnv64_t);

  int urand_fd = open("/dev/urandom", O_RDONLY);
  if(urand_fd < 0){
    perror("/dev/urandom open");
    exit(-1);
  }
  /*
  size_t total_read = 0;

  while(total_read < data_size){
    int bytes_read = read(urand_fd, args->data_ptr + total_read, ((data_size - total_read) < 4096 ? (data_size - total_read) : 4096));
    if(bytes_read < 0){
      perror("/dev/urandom read");
      exit(-1);
    }

    total_read += bytes_read;
  }
  */
  size_t cnt = 0;
  char c = 'A';
  while(cnt < data_size){
    if(cnt && ((cnt & 0xFFF ) == 0)){
        c++;
    }
    args->data_ptr[cnt] = c;

    cnt++;
  }

  Fnv64_t hval = fnv_64a_buf((void *)args->data_ptr, data_size, 0);

  int error = door_call(door_fd, args);
  if(error){
    perror("door_call");
    exit(-1);
  }

  ASSERT( *((int *)args->rbuf) == 0, "Door procedure found leaks in shared mapping");

  free(args->rbuf);
  args->rbuf  = NULL;
  free(args);


  return;
}


void door__argument_fd_recv(void) {
  const char *dpath = "/home/bojan/fd_recv.door";

	int server = open(dpath, O_RDONLY);
	if (server == -1){
    perror("Could not open door");
    return;
  }


	door_arg_t args = {0};



	char* path = "secret_data";
	int file = open(path, O_RDONLY);
  if(file < 0){
    perror("Opening secret_data");
    return;
  }


  // This tells the doors API that we'd like to pass a descriptor back to the client
	door_desc_t w_descriptor;
	w_descriptor.d_attributes = DOOR_DESCRIPTOR;
	w_descriptor.d_data.d_desc.d_descriptor = file;

  args.desc_ptr = &w_descriptor;
  args.desc_num = 1;

	int result = door_call(server, &args);
	if (result) {
    perror("door_call");
      return;
  }

	ASSERT(strncmp(args.rbuf, "1234", 4) == 0, "Invalid file contents received");

	return ;
}

void door__argument_fd_send(void) {
  const char *dpath = "/home/bojan/fd_send.door";

	int server = open(dpath, O_RDONLY);
	if (server == -1){
    perror("Could not open door");
    return;
  }

  door_arg_t args = {0};


	int result = door_call(server, &args);
	if (result) {
    perror("door_call");
    return;
  }

  ASSERT(args.desc_ptr, "No desc_ptr present");

  int fd = args.desc_ptr->d_data.d_desc.d_descriptor;
  char buf[512] = {0};

  size_t r = read(fd, buf, 128);
  if(r <= 0){
    perror("recvd fd read");
    exit(-1);
  }

  ASSERT(strncmp(buf, "4321", 4) == 0, "Invalid file contents");
}

void door__unref(void) {
  const char *dpath = "/home/bojan/unref.door";
  struct stat   st;

	int server = open(dpath, O_RDONLY);
	if (server == -1){
    perror("Could not open door");
    return;
  }
  close(server);

  sleep(1);

  int error = stat("unref_success", &st);
  ASSERT(error == 0, "Unref file not created");

  remove("unref_success");

  server = open(dpath, O_RDONLY);
	if (server == -1){
    perror("Could not open door");
    return;
  }
  close(server);

  error = stat("unref_success", &st);
  ASSERT(error == -1 && errno == ENOENT, "Unref called twice on a single-unref door");

  return;
}

void door__unref_multi(void) {
  const char *dpath = "/home/bojan/unref_multi.door";
  struct stat   st;

  for (int i=0; i<5; i++){

    char path[32];

    int server = open(dpath, O_RDONLY);
    if (server == -1){
      perror("Could not open door");
      return;
    }
    close(server);

    bzero(path, 32);
    snprintf(path, 32, "unref_success%d", i);


    sleep(1);
    printf("%d\n", i);
    int error = stat(path, &st);
    ASSERT(error == 0, "Unref file not created");

    remove(path);
  }
  return;
}

void door__argument_copy_rbuf_private(void) {
  const char *path = "/home/bojan/echo_private.door";
  const char *test_value = "123456";

	int door_fd = open(path, O_RDONLY);
	if (door_fd == -1){
    perror("Could not open door");
    exit(-1);
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
    exit(-1);
  }

  args->data_ptr = (char *)calloc(strlen(test_value) + 1, 1);
  args->data_size = strlen(test_value) + 1;

  strlcpy(args->data_ptr, test_value, args->data_size);

  args->rbuf = rbuf;
  args->rsize = 64;

  int error = door_call(door_fd, args);
  if(error){
    perror("door_call");
    exit(-1);
  }


  ASSERT(strncmp(args->rbuf, test_value, args->rsize) == 0, "Received invalid value from door server");


  args->rbuf  = NULL;
  free(rbuf);
  free(args);


  return;
}



int main(void){
 const char *echo_path = "/home/bojan/echo.door";
  const char *echo_private_path = "/home/bojan/echo_private.door";
const char *hash_path = "/home/bojan/hash_echo.door";
 const char *fd_recv_path = "/home/bojan/fd_recv.door";
  const char *fd_send_path = "/home/bojan/fd_send.door";
  const char *unref_path = "/home/bojan/unref.door";
  const char *unref_multi_path = "/home/bojan/unref_multi.door";

  pid_t pid_echo = door_create_helper(DOOR_ECHO, NULL, 0, echo_path);
  if(pid_echo < 0){
    perror("door_create_helper echo");
    exit(-1);
  }
  pid_t pid_hash = door_create_helper(DOOR_HASH_ECHO, NULL, 0, hash_path);
  if(pid_hash < 0){
   perror("door_create_helper hash");
    exit(-1);
   }
  pid_t pid_echof = door_create_helper(DOOR_FD_RECV, NULL, 0, fd_recv_path);
  if(pid_echof < 0){
    perror("door_create_helper fd_recv");
    exit(-1);
  }

  pid_t pid_echofsend = door_create_helper(DOOR_FD_SEND, NULL, 0, fd_send_path);
  if(pid_echofsend < 0){
    perror("door_create_helper fd_send");
    exit(-1);
  }

  pid_t pid_unref = door_create_helper(DOOR_UNREF_TEST, NULL, DOOR_UNREF, unref_path);
  if(pid_unref < 0){
    perror("door_create_helper unref");
    exit(-1);
  }

  pid_t pid_unref_multi = door_create_helper(DOOR_UNREF_MULTI_TEST, NULL, DOOR_UNREF_MULTI, unref_multi_path);
  if(pid_unref < 0){
    perror("door_create_helper unref_multi");
    exit(-1);
  }

  pid_t pid_echo_private = door_create_helper(DOOR_ECHO, NULL, DOOR_PRIVATE, echo_private_path);
  if(pid_unref < 0){
    perror("door_create_helper echo_private");
    exit(-1);
  }

  printf("Running shared argument mapping test with random data...\n");
  door__argument_transfer_rbuf_shared_mapping();

  printf("Running basic argument copy test...\n");
  door__argument_copy_rbuf();

  printf("Running basic argument copy test without allocating a result buffer...\n");
  door__argument_copy_no_rbuf();

  printf("Running basic file descriptor argument passing test..\n");
  door__argument_fd_recv();

  printf("Running basic file descriptor result passing test..\n");
  door__argument_fd_send();

  printf("Running basic unref test..\n");
  door__unref();

  printf("Running basic unref_multi test..\n");
  door__unref_multi();

  printf("Running basic private thread pool call..\n");
  door__argument_copy_rbuf_private();
kill(pid_hash, SIGINT);
  kill(pid_echo, SIGINT);
  kill(pid_echof, SIGINT);
  kill(pid_echofsend, SIGINT);
  kill(pid_unref, SIGINT);
  kill(pid_unref_multi, SIGINT);
  kill(pid_echo_private, SIGINT);
  printf("All done!\n");
}

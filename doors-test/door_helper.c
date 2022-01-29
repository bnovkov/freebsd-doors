#include "door_helper.h"


#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>
#include <pthread.h>

#include <signal.h>

#include "fnv/fnv.h"


static void *door_shared_map_leak_check(void *udata, char *argp, size_t arg_size, door_desc_t *dp, u_int n_desc){

  int leak_present = 1;
  int error;
  char *buf_start_page = (char *)((vm_offset_t)argp & (vm_offset_t)~0xFFF);
  char *buf_end_page = (char *)((vm_offset_t)(argp + arg_size) & (vm_offset_t)~0xFFF) + 4096;


  for(char *b = buf_start_page; b < argp; b++){
    if(*b != 0){
      goto out;
    }
  }

  for(char *b = (argp + arg_size); b < buf_end_page; b++){
    if(*b != 0){ 
      goto out;
    }
  }

 leak_present = 0;
 out:
	error = door_return((char *)&leak_present, sizeof(int), NULL, 0);
  if (error){
    perror("door_return");
  }

  return NULL;
}

static void *door_hash_echo(void *udata, char *argp, size_t arg_size, door_desc_t *dp, u_int n_desc){
  Fnv64_t hval = fnv_64a_buf((void *)argp, arg_size, 0);

	int error = door_return((char *)&hval, sizeof(Fnv64_t), NULL, 0);
  if (error){
    perror("door_return");
  }

  return NULL;
}


static void *door_echo(void *udata, char *argp, size_t arg_size, door_desc_t *dp, u_int n_desc){

	int error = door_return(argp, arg_size, NULL, 0);
  if (error){
    perror("door_return");
  }

  return NULL;
}


static void *door_fd_recv(void *udata, char *argp, size_t arg_size, door_desc_t *dp, u_int n_desc){
	door_desc_t* w_descriptor = dp;
  int file = w_descriptor->d_data.d_desc.d_descriptor;

  char data[128];
  bzero(data, 128);
  read(file, data, 128);

  int error = door_return(data, strlen(data)+3, NULL, 0);
  if (error){
    perror("door_return");
  }

  return NULL;

}

static void *door_fd_send(void *udata, char *argp, size_t arg_size, door_desc_t *dp, u_int n_desc){


  char* path = "secret_data2";
	int file = open(path, O_RDONLY);
  if(file < 0){
    perror("Opening secret_data2");
    return NULL;
  }


  door_desc_t w_descriptor;
	w_descriptor.d_attributes = DOOR_DESCRIPTOR;
	w_descriptor.d_data.d_desc.d_descriptor = file;

  int error = door_return(NULL, 0, &w_descriptor, 1);
  if (error){
    perror("door_return");
  }

  return NULL;

}

static void *door_unref(void *udata, char *argp, size_t arg_size, door_desc_t *dp, u_int n_desc){

  if(argp == DOOR_UNREF_DATA){
    char* path = "unref_success";
    remove(path);
    int file = open(path, O_CREAT, 0777);
    if(file < 0){
      perror("creating unref_success");
      return NULL;
    }
    close(file);
  }

  int error = door_return(NULL, 0, NULL, 0);
  if (error){
    perror("door_return");
  }

  return NULL;

}

static int unref_multi_count = 0;

static void *door_unref_multi(void *udata, char *argp, size_t arg_size, door_desc_t *dp, u_int n_desc){

  if(argp == DOOR_UNREF_DATA){
    char path[32];

    bzero(path, 32);
    snprintf(path, 32, "unref_success%d", unref_multi_count);
    int file = open(path, O_CREAT, 0777);
    if(file < 0){
      perror("creating unref_success");
      return NULL;
    }
    close(file);

    unref_multi_count++;
  }

  int error = door_return(NULL, 0, NULL, 0);
  if (error){
    perror("door_return");
  }

  return NULL;
}



static server_procedure *test_procedures[] = {
  door_echo,
  door_hash_echo,
  door_shared_map_leak_check,
  door_fd_recv,
  door_fd_send,
  door_unref,
  door_unref_multi
};

static const char *current_path;

static void cleanup(int sig){
  remove(current_path);
}

static void*  private_thread(void* args){
  int error;
  int fd = *(int *)args;

  free(args);


  error = door_bind(fd);
  if (error){
    perror("door_bind");
    exit(-1);
  }

  return NULL;
}


pid_t door_create_helper(int proc_no, void *udata, u_int attributes, const char *path){

  pid_t pid;
  pid = fork();

  if(pid == 0){

    current_path = path;
    remove(current_path);

    int d_fd =	door_create(test_procedures[proc_no], NULL, attributes);
    if(d_fd < 0){
      perror("door_create");
      return -1;
    }

    if(attributes & DOOR_PRIVATE){
      pthread_t tid;
      int *arg_fd = (int *)malloc(sizeof(int));
      *arg_fd = d_fd;

      int error = pthread_create(&tid, NULL, private_thread, (void *)arg_fd);
      if(error) {
        free(arg_fd);
        perror("private_thread_create");
        // TODO: exit?
        return -1;
      }
    }

    int fd = open(path, O_RDWR|O_CREAT|O_EXCL, 0400);
    if (fd < 0){
      perror("Could not create a new file for the door");
      return -1;
    }


    int error =	door_attach(d_fd, path);
    if(error){
      perror("door_attach");
      return -1;
    }

    sleep(500);
    cleanup(0);
    exit(0);
  }

  return pid;
  }

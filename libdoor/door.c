#include <pthread.h>
#include <pthread_np.h>

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <door.h>



struct default_thread_args{
  int private;
  int fd;
};

static door_server_func_t *server_create_func = NULL;

static int get_stack_info(void **stack_base, size_t *size){
  pthread_attr_t attr;

  pthread_attr_init(&attr);

  int error = pthread_attr_get_np(pthread_self(), &attr);
  if(error){
    perror("pthread_attr_get_np");
    return -1;
  }


  error =   pthread_attr_getstackaddr(&attr, stack_base);
  if(error){
    perror("pthread_attr_getstackaddr");
    return -1;

  }

  error =   pthread_attr_getstacksize(&attr, size);
  if(error){
    perror("pthread_attr_getstacksize");
    return -1;

  }

  return 0;
}


static int __door_unref(int door_fd){
  void *stack_base;
  size_t stack_size;

  int error = get_stack_info(&stack_base, &stack_size);
  if(error){
    return error;
  }

  error = __syscall(SYS_door, DOOR_UNREFSYS, door_fd, stack_base, stack_size);
  if(error){
    perror("door_unref");
    return -1;
  }

  return 0;
}


static void*  __door_default_thread(void* args){
  int error;

  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    error = door_return(NULL, 0, NULL, 0);
    if (error){
      perror("door_return");
      exit(-1);
    }

  return NULL;
}

static void*  __door_unref_thread(void* args){
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

  int error = __door_unref((int)args);
  if (error){
    return NULL;
  }

  return NULL;
}

static void*  __door_private_thread(void* args){
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

  int error = door_bind((int)args);
  if (error){
    return NULL;
  }

  error = door_return(NULL, 0, NULL, 0);
  if (error){
     perror("door_return");
     exit(-1);
  }
  
  return NULL;
}

static void __door_private_thread_create(int fd){
  pthread_t private_thread_id;

  int error = pthread_create(&private_thread_id, NULL, __door_private_thread, fd);
  if(error) {
    perror("default_thread_create");
    // TODO: exit?
    return;
  }
  return;
}

static void __door_default_server_create(door_info_t *dip){
  pthread_t default_thread_id;

  int error = pthread_create(&default_thread_id, NULL, __door_default_thread, NULL);
  if(error) {
    perror("default_thread_create");
    // TODO: exit?
    return;
  }
  return;
}


int door_create(server_procedure* procedure, void* udata, u_int attributes){
  pthread_t default_thread_id;

  int unref = attributes & DOOR_UNREF;
  int unref_multi = attributes & DOOR_UNREF_MULTI;
  int private = attributes & DOOR_PRIVATE;

  int fd = __syscall(SYS_door, DOOR_CREATE, procedure, udata, attributes);
  if(fd < 0) {
    return -1;
  }

  if(private){
    __door_private_thread_create(fd);
  }else if(server_create_func == NULL){
    __door_default_server_create(NULL);
  }else {
    server_create_func(NULL);
  }
  if(unref || unref_multi){
    pthread_t unref_thread_id;

    int error = pthread_create(&unref_thread_id, NULL, __door_unref_thread, (void *)fd);
    if(error) {
      perror("unref_thread_create");
      return -1;
    }
  }


  return fd;
}

int door_call(int fd, door_arg_t *args){
  return  __syscall(SYS_door, DOOR_CALL, fd, args);
}


int door_return(char *data_ptr, size_t data_size, door_desc_t *desc, size_t num_desc){
  void* stack_base;
  size_t stack_size;

  int error = get_stack_info(&stack_base, &stack_size);
  if(error){
    perror("door_return: get_stack_info");
    return error;
  }

  error =  __syscall(SYS_door, DOOR_RETURN, data_ptr, data_size, desc, num_desc, stack_base, stack_size);
  if(error == DOOR_POOL_DEPLETED){
    error = 0;
    server_create_func(NULL);
  }

  return error;
}

int door_attach(int fd, const char* path){
  return __syscall(SYS_door, DOOR_ATTACH, fd, path);
}

int door_detach(const char* path){
  return __syscall(SYS_door, DOOR_DETACH, path);
}

int door_revoke(int fd){
  return __syscall(SYS_door, DOOR_REVOKE, fd);
}

int door_info(int fd, struct door_info *info){
  return __syscall(SYS_door, DOOR_INFO, fd, info);
}

int door_setparam(int fd, int param, size_t val){
  return __syscall(SYS_door, DOOR_SETPARAM, fd, param, val);
}

int door_getparam(int fd, int param, size_t *out){
  return __syscall(SYS_door, DOOR_GETPARAM, fd, param, out);
}

int door_bind(int fd){
  void* stack_base;
  size_t stack_size;

  int error = get_stack_info(&stack_base, &stack_size);
  if(error){
    return error;
  }

  return __syscall(SYS_door, DOOR_BIND, fd, stack_base, stack_size);
}

int door_unbind(void){
  return __syscall(SYS_door, DOOR_UNBIND);
}

// TODO: locking
door_server_func_t *door_server_create(door_server_func_t *new){
  door_server_func_t *old = server_create_func;
  server_create_func = new;

  return old;
}

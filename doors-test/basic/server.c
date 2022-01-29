#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <door.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>

const char *response = "Well hello there!\n";

void *
echo_server_proc(void *cookie, char *argp, size_t arg_size, door_desc_t *dp, u_int n_desc)
{
  printf("Hello from the server thread!\n");
  

//  if(argp){
//    printf("Got arg: %s\n", argp);
//  }

<<<<<<< HEAD
  int a = *((int *)argp) + 1;
  printf("%d\n", a);

  a = *((int *)(argp + arg_size - 32)) + 1;
  printf("%d\n", a);

  int error = door_return((char *)response, strlen(response), NULL, 0);
=======
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
  error = door_return((char *)response, strlen(response), NULL, 0);
>>>>>>> 3c73a157616f59813f90404dd54073e19a39e607
  if (error){
    perror("door_return");
  }

  return NULL;
}

int main(void)
{
  const char* path = "/home/bojan/server.door";

  int door_fd =	door_create(echo_server_proc, NULL, DOOR_PRIVATE);
 if(door_fd < 0){
   perror("door_create");
   return -1;
 }
 printf("Server: got door fd %d\n", door_fd);

 int fd = open(path, O_RDWR|O_CREAT, 0777);
 if (fd < 0){
   perror("Could not create a new file for the door");
   return -1;
 }


 int error =	door_attach(door_fd, path);
 if(error){
   perror("door_attach");
   return -1;
 }

 printf("Server: attached door_fd\n");

 struct door_info info;

 error = door_info(door_fd, &info);
 if(error){
   perror("door_revoke");
   return -1;
 }

 printf("Door info:\n PID: %d\n Procedure ptr: %p\n User data ptr: %p\n Atrributes %x\n ID: %u\n",
        info.di_target,
        info.di_proc,
        info.di_data,
        info.di_attributes,
        info.di_uniquifier);

 size_t maxdesc_param;

 error = door_getparam(door_fd, DOOR_PARAM_DESC_MAX, &maxdesc_param);
 if(error){
   perror("door_getparam");
   return -1;
 }

 printf("Max door descriptors: %zu\n", maxdesc_param);

 maxdesc_param = 1337;

 error = door_setparam(door_fd, DOOR_PARAM_DESC_MAX, maxdesc_param);
 if(error){
   perror("door_setparam");
   return -1;
 }

 error = door_getparam(door_fd, DOOR_PARAM_DESC_MAX, &maxdesc_param);
 if(error){
   perror("door_getparam");
   return -1;
 }

 printf("Max door descriptors after 'door_setparam': %zu\n", maxdesc_param);


 sleep(500);

 error = door_revoke(door_fd);
  if(error){
   perror("door_revoke");
   return -1;
 }

  error = door_detach(path);
  if(error){
    perror("door_detach");
    return -1;
  }

  sleep(50);

 return 0;
}

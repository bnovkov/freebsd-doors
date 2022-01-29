#pragma once

#include <sys/syscall.h>
#include <unistd.h>



#define __DOOR_CREATE_SYSCALL 581
#define __DOOR_CALL_SYSCALL 582
#define __DOOR_RETURN_SYSCALL 583
#define __DOOR_ATTACH_SYSCALL 584


#define sys__door_create(procedure, udata) __syscall(__DOOR_CREATE_SYSCALL, (procedure), (udata))
#define sys__door_call(fd, args) __syscall(__DOOR_CALL_SYSCALL, fd, args)
#define sys__door_return() __syscall(__DOOR_RETURN_SYSCALL)
#define sys__door_attach(fd, path) __syscall(__DOOR_ATTACH_SYSCALL, (fd), (path))

typedef uint_t door_attr_t;

typedef struct door_desc {
	door_attr_t	d_attributes;	/* Tag for union */

} door_desc_t;

typedef struct door_arg {
	char		*data_ptr;	/* Argument/result data */
	size_t		data_size;	/* Argument/result data size */
	door_desc_t	*desc_ptr;	/* Argument/result descriptors */
	uint_t		desc_num;	/* Argument/result num discriptors */
	char		*rbuf;		/* Result area */
	size_t		rsize;		/* Result size */
} door_arg_t;

typedef void *(server_procedure)(void *udata, char *argp, size_t arg_size, door_desc_t *dp, uint_t n_desc);


int door_create(server_procedure* procedure, void* udata, uint_t attributes);
int door_call(int fd, door_arg_t *args);
int door_return(char *data_ptr, size_t data_size, door_desc_t *desc, uint_t num_desc);
int door_attach(int fd, const char* path);

/* Solaris compatibility */
#define fattach(fd, path) door_attach((fd), (path))

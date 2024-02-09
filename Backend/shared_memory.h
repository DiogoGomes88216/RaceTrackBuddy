#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

char* attach_block(char const *filename, int size);
bool detach_block(char *block);
bool destroy_block(char *filename);

#endif // SHARED_MEMORY_H

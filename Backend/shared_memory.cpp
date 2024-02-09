#include "shared_memory.h"

int get_block(char const *filename, int size)
{
    key_t key;

    key = ftok(filename, 0);
    if(key == -1)
        return -1;

    return shmget(key, size, 0644 | IPC_CREAT);
}

char* attach_block(char const * filename, int size)
{
    int shared_block_id = get_block(filename, size);
    char* result;

    if(shared_block_id == -1)
        return NULL;

    result = (char*) shmat(shared_block_id, NULL, 0);
    if(result == (char*) -1)
        return NULL;

    return result;
}

bool detach_block(char* block)
{
    return (shmdt(block) != -1);
}

bool destroy_block(char* filename)
{
    int shared_block_id = get_block(filename, 0);

    if(shared_block_id == -1)
        return false;

    return (shmctl(shared_block_id, IPC_RMID, NULL) != -1);
}

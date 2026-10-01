#include "malloc.h"

#include <stdlib.h>

void *psyq_malloc_malloc(u32 Size) {
    return malloc(Size);
}

void psyq_malloc_free(void *Ptr) {
    free(Ptr);
}

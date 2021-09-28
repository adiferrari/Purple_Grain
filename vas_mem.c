/**
 * @file vas_mem.c
 * @author Nikita Kretschmar
 * @author Adrian Philipp
 * @author Micha Strobl
 * @author Tim Wennemann
 * @brief handling of memory operations
 * handling of memory operations, e.g. allocation, resize, free
 * @version 0.1
 * @date 2021-09-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef vas_memory_c
#define vas_memory_c

#include "vas_mem.h"
/**
 * @brief handles memory allocation
 * handles memory allocation according to @param size
 * @param size required memory size
 * @return void* 
 */
void *vas_mem_alloc(long size)
{
    
#ifdef MAXMSPSDK
    
    return sysmem_newptrclear(size);

#elif defined(PUREDATA)
    
    return malloc(size);
    
#else
    
    return malloc(size);
    
#endif
    
}
/**
 * @brief handles resizing of memory
 * handles resizing of memory according to to @param size on given @param ptr
 * @param ptr memory pointer
 * @param size memory size
 * @return void* 
 */
void *vas_mem_resize(void *ptr, long size)
{
    
#ifdef MAXMSPSDK
    
    return sysmem_resizeptrclear(ptr, size);
    
#elif defined(PUREDATA)

    void *tmp = realloc(ptr, size);
    memset(tmp, 0, size);
    return tmp;

    
#else

    void *tmp = realloc(ptr, size);
    memset(tmp, 0, size);
    return tmp;
 
#endif
    
}
/**
 * @brief frees memory
 * frees memory at given @param ptr
 * @param ptr memory pointer
 */
void vas_mem_free(void *ptr)
{
    
#ifdef MAXMSPSDK
    if(ptr)
        sysmem_freeptr(ptr);
    
#elif defined(PUREDATA)
    if(ptr)
        free(ptr);
    
#else
    if(ptr)
        free(ptr);
    
#endif
    
}

#endif

/**
* Copyright (C) Mellanox Technologies Ltd. 2001-2016.  ALL RIGHTS RESERVED.
*
* See file LICENSE for terms.
*/

#ifndef UCX_HELLO_WORLD_H
#define UCX_HELLO_WORLD_H

/* Debug mode flag */
#define DEBUG

#include <ucs/memory/memory_type.h>

#include <sys/poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <netdb.h>

#ifdef HAVE_CUDA
#  include <cuda.h>
#  include <cuda_runtime.h>
#endif

/* Macro to make conditional DEBUG more terse (hopefully :) )*/
#ifdef DEBUG
#define debugmsg(_msg) \
    fprintf(stderr, _msg)
#else
#define debugmsg(...) ((void) 0)
#endif


#define CHKERR_ACTION(_cond, _msg, _action) \
    do { \
        if (_cond) { \
            fprintf(stderr, "Failed to %s\n", _msg); \
            _action; \
        } \
    } while (0)

static ucs_memory_type_t test_mem_type = UCS_MEMORY_TYPE_HOST;


#define CUDA_FUNC(_func)                                   \
    do {                                                   \
        cudaError_t _result = (_func);                     \
        if (cudaSuccess != _result) {                      \
            fprintf(stderr, "%s failed: %s\n",             \
                    #_func, cudaGetErrorString(_result));  \
        }                                                  \
    } while(0)


void print_common_help(void);

void *mem_type_malloc(size_t length)
{
    void *ptr;

    switch (test_mem_type) {
    case UCS_MEMORY_TYPE_HOST:
        ptr = malloc(length);
        break;
#ifdef HAVE_CUDA
    case UCS_MEMORY_TYPE_CUDA:
        CUDA_FUNC(cudaMalloc(&ptr, length));
        break;
    case UCS_MEMORY_TYPE_CUDA_MANAGED:
        CUDA_FUNC(cudaMallocManaged(&ptr, length, cudaMemAttachGlobal));
        break;
#endif
    default:
        fprintf(stderr, "Unsupported memory type: %d\n", test_mem_type);
        ptr = NULL;
        break;
    }

    return ptr;
}

void mem_type_free(void *address)
{
    switch (test_mem_type) {
    case UCS_MEMORY_TYPE_HOST:
        free(address);
        break;
#ifdef HAVE_CUDA
    case UCS_MEMORY_TYPE_CUDA:
    case UCS_MEMORY_TYPE_CUDA_MANAGED:
        CUDA_FUNC(cudaFree(address));
        break;
#endif
    default:
        fprintf(stderr, "Unsupported memory type: %d\n", test_mem_type);
        break;
    }
}

void *mem_type_memcpy(void *dst, const void *src, size_t count)
{
    switch (test_mem_type) {
    case UCS_MEMORY_TYPE_HOST:
        memcpy(dst, src, count);
        break;
#ifdef HAVE_CUDA
    case UCS_MEMORY_TYPE_CUDA:
    case UCS_MEMORY_TYPE_CUDA_MANAGED:
        CUDA_FUNC(cudaMemcpy(dst, src, count, cudaMemcpyDefault));
        break;
#endif
    default:
        fprintf(stderr, "Unsupported memory type: %d\n", test_mem_type);
        break;
    }

    return dst;
}

void *mem_type_memset(void *dst, int value, size_t count)
{
    switch (test_mem_type) {
    case UCS_MEMORY_TYPE_HOST:
        memset(dst, value, count);
        break;
#ifdef HAVE_CUDA
    case UCS_MEMORY_TYPE_CUDA:
    case UCS_MEMORY_TYPE_CUDA_MANAGED:
        CUDA_FUNC(cudaMemset(dst, value, count));
        break;
#endif
    default:
        fprintf(stderr, "Unsupported memory type: %d", test_mem_type);
        break;
    }

    return dst;
}

int check_mem_type_support(ucs_memory_type_t mem_type)
{
    switch (test_mem_type) {
    case UCS_MEMORY_TYPE_HOST:
        return 1;
    case UCS_MEMORY_TYPE_CUDA:
    case UCS_MEMORY_TYPE_CUDA_MANAGED:
#ifdef HAVE_CUDA
        return 1;
#else
        return 0;
#endif
    default:
        fprintf(stderr, "Unsupported memory type: %d", test_mem_type);
        break;
    }

    return 0;
}

ucs_memory_type_t parse_mem_type(const char *opt_arg)
{
    if (!strcmp(opt_arg, "host")) {
        return UCS_MEMORY_TYPE_HOST;
    } else if (!strcmp(opt_arg, "cuda") &&
               check_mem_type_support(UCS_MEMORY_TYPE_CUDA)) {
        return UCS_MEMORY_TYPE_CUDA;
    } else if (!strcmp(opt_arg, "cuda-managed") &&
               check_mem_type_support(UCS_MEMORY_TYPE_CUDA_MANAGED)) {
        return UCS_MEMORY_TYPE_CUDA_MANAGED;
    } else {
        fprintf(stderr, "Unsupported memory type: \"%s\".\n", opt_arg);
    }

    return UCS_MEMORY_TYPE_LAST;
}

void print_common_help()
{
    fprintf(stderr, "  -p <port>     Set alternative server port (default:13337)\n");
    fprintf(stderr, "  -6            Use IPv6 address in data exchange\n");
    fprintf(stderr, "  -s <size>     Set test string length (default:16)\n");
    fprintf(stderr, "  -m <mem type> Memory type of messages\n");
    fprintf(stderr, "                host - system memory (default)\n");
    if (check_mem_type_support(UCS_MEMORY_TYPE_CUDA)) {
        fprintf(stderr, "                cuda - NVIDIA GPU memory\n");
    }
    if (check_mem_type_support(UCS_MEMORY_TYPE_CUDA_MANAGED)) {
        fprintf(stderr, "                cuda-managed - NVIDIA GPU managed/unified memory\n");
    }
}


#endif /* UCX_HELLO_WORLD_H */

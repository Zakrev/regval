#ifndef _DEBUG_PRINT_H_
#define _DEBUG_PRINT_H_

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>

#define DBG_ALLOC_MEM
#define DBG_LVL 1

#if DBG_LVL >= 1
#define PCERR(...) fprintf(stderr, "CRITICAL ERROR: %s: %d: %s: ", __FILE__, __LINE__, __FUNCTION__); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); perror("ERROR")
#define PERR(...) fprintf(stderr, "ERROR: %s: %d: %s: ", __FILE__, __LINE__, __FUNCTION__); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n")
#else
#define PCERR(...)
#define PERR(...)
#endif

#if DBG_LVL >= 2
#define PINF(...) fprintf(stdout, "NOTICE: %s: %d: %s: ", __FILE__, __LINE__, __FUNCTION__); fprintf(stdout, __VA_ARGS__); fprintf(stdout, "\n"); fflush(stdout)
#define PRINT(...) fprintf(stdout, __VA_ARGS__); fflush(stdout)

#include <sys/time.h>
static double __time_exec_wtime__()
{
    struct timeval t;

    gettimeofday(&t, NULL);

    return (double)t.tv_sec + (double)t.tv_usec * 1E-6;
}
#define PFUNC_START() \
                fprintf(stdout, "SF: %s\n", __FUNCTION__); fflush(stdout); \
                double __time_exec_function__ = __time_exec_wtime__()
                
#define PFUNC_END() \
                fprintf(stdout, "SE: %s: %f sec\n", __FUNCTION__, __time_exec_wtime__() - __time_exec_function__); fflush(stdout)
#else
#define PINF(...)
#define PRINT(...)
#define PFUNC_START()
#define PFUNC_END()
#endif

#endif
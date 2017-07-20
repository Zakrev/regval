#ifndef _DEBUG_PRINT_H_
#define _DEBUG_PRINT_H_

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>

#define DBG_LVL 1

#if DBG_LVL >= 1
#define PCERR(...) fprintf(stderr, "CRITICAL ERROR: %s: %d: %s: ", __FILE__, __LINE__, __FUNCTION__); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); perror("ERROR")
#define PERR(...) fprintf(stderr, "ERROR: %s: %d: %s: ", __FILE__, __LINE__, __FUNCTION__); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n");
#else
#define PCERR(...)
#define PERR(...)
#endif

#if DBG_LVL >= 2
#define PINF(...) fprintf(stdout, "NOTICE: %s: %d: %s: ", __FILE__, __LINE__, __FUNCTION__); fprintf(stdout, __VA_ARGS__); fprintf(stdout, "\n"); fflush(stdout)
#define PRINT(...) fprintf(stdout, __VA_ARGS__); fflush(stdout)
#else
#define PINF(...)
#define PRINT(...)
#endif

#endif
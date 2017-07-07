#ifndef _DEBUG_PRINT_H_
#define _DEBUG_PRINT_H_

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>

#define ENABLE_DEBUG_MSG

#ifdef ENABLE_DEBUG_MSG
#define PINF(...) fprintf(stdout, "NOTICE: %s: %d: %s: ", __FILE__, __LINE__, __FUNCTION__); fprintf(stdout, __VA_ARGS__); fprintf(stdout, "\n"); fflush(stdout)
#define PERR(...) fprintf(stderr, "ERROR: %s: %d: %s: ", __FILE__, __LINE__, __FUNCTION__); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); perror("ERROR")
#define PRINT(...) printf(__VA_ARGS__); fflush(stdout)
#else
#define PERR(...)
#define PINF(...)
#define PRINT(...)
#endif

#endif
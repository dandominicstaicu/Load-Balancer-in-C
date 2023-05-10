// Copyright 2023 <Dan-Dominic Staicu>
// Copyright 2023 SD Homework team
#ifndef UTILS_H_
#define UTILS_H_

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// useful macro for handling error codes
#define DIE(assertion, call_description)                                       \
    do {                                                                       \
        if (assertion) {                                                       \
            fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__);                 \
            perror(call_description);                                          \
            exit(errno);                                                       \
        }                                                            		   \
    } while (0)

// macro for displaying errors in code and ending the function
#define ERROR(msg)							\
	do {									\
		fprintf(stderr, "%s\n", msg);		\
	} while (0)

#endif  // UTILS_H_

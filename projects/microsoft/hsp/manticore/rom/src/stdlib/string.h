// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef STRING_H_
#define STRING_H_

#include <stddef.h>


/* A minimal version to support ROM builds. */
int memcmp(const void*, const void*, size_t);
void* memcpy(void* __restrict, const void* __restrict, size_t);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);


#endif /* STRING_H_ */

#pragma once

#define NAME_MAX 255
#define _POSIX_C_SOURCE 200809L

#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>
#include <erl_nif.h>
#include <membrane/membrane.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "lib.h"

typedef struct _ShmGuard {
  char name[NAME_MAX+1];
} ShmGuard;

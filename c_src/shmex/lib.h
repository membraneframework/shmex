#pragma once

#define NAME_MAX 255
#define _POSIX_C_SOURCE 200809L

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <erl_nif.h>
#include <bunch/bunch.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <stdio.h>

typedef struct {
  char * name;
  ERL_NIF_TERM guard;
  unsigned int size;
  unsigned int capacity;
  void * mapped_memory;
  ERL_NIF_TERM elixir_struct_atom;
} Shmex;

#define SHMEX_ELIXIR_STRUCT_ENTRIES 5
#define SHMEX_ELIXIR_STRUCT_ATOM "Elixir.Shmex"
#define SHM_NAME_PREFIX "/shmex-"
#define SHMEX_ALLOC_MAX_ATTEMPTS 1000

typedef enum ShmexLibResult {
  SHMEX_RES_OK,
  SHMEX_ERROR_SHM_OPEN,
  SHMEX_ERROR_FTRUNCATE,
  SHMEX_ERROR_MMAP,
  SHMEX_ERROR_SHM_MAPPED
} ShmexLibResult;

typedef struct _ShmexGuard {
  char name[NAME_MAX+1];
} ShmexGuard;

void shmex_generate_name(Shmex * payload);
void shmex_init(ErlNifEnv * env, Shmex * payload, unsigned capacity);
void shmex_add_guard(ErlNifEnv * env, ErlNifResourceType* guard_type, Shmex *payload);
void shmex_guard_destructor(ErlNifEnv* env, void * resource);
int shmex_get_from_term(ErlNifEnv * env, ERL_NIF_TERM record, Shmex * payload);
ShmexLibResult shmex_allocate(Shmex * payload);
void shmex_release(Shmex *payload);
ERL_NIF_TERM shmex_make_term(ErlNifEnv * env, Shmex * payload);
ERL_NIF_TERM shmex_make_error_term(ErlNifEnv * env, ShmexLibResult result);
ShmexLibResult shmex_set_capacity(Shmex * payload, size_t capacity);
ShmexLibResult shmex_open_and_mmap(Shmex * payload);
void shmex_unmap(Shmex * payload);

#define PARSE_SHMEX_ARG(position, var_name) \
  BUNCH_PARSE_ARG(position, var_name, Shmex var_name, shmex_get_from_term, &var_name)

#include "lib.h"

// feature test macro for clock_gettime and ftruncate
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#ifdef SHMEX_NIF
#define ALLOC(X) enif_alloc(X)
#define FREE(X) enif_free(X)
#else
#define ALLOC(X) malloc(X)
#define FREE(X) free(X)
#endif

void shmex_generate_name(Shmex *payload) {
  static const unsigned GENERATED_NAME_SIZE = strlen(SHM_NAME_PREFIX) + 21;
  if (payload->name != NULL) {
    FREE(payload->name);
  }
  payload->name = ALLOC(GENERATED_NAME_SIZE);

  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  snprintf(payload->name, GENERATED_NAME_SIZE, SHM_NAME_PREFIX "%.12ld%.8ld",
           ts.tv_sec, ts.tv_nsec);
}

/**
 * Allocates POSIX shared memory given the data (name, capacity) in Shmex
 * struct.
 *
 * If name in Shmex is set to NULL, the name will be (re)genrated until
 * the one that haven't been used is found (at most SHMEX_ALLOC_MAX_ATTEMPTS
 * times).
 *
 * Shared memory can be accessed by using 'shmex_open_and_mmap'.
 * Memory will be unmapped when Shmex struct is freed (by 'shmex_release')
 */
ShmexLibResult shmex_allocate(Shmex *payload) {
  ShmexLibResult result;
  int fd = -1;

  int attempts = 1;
  if (payload->name == NULL) {
    shmex_generate_name(payload);
    attempts = SHMEX_ALLOC_MAX_ATTEMPTS;
  }
  fd = shm_open(payload->name, O_RDWR | O_CREAT | O_EXCL, 0666);
  while (fd < 0) {
    attempts--;
    if (errno != EEXIST || attempts <= 0) {
      result = SHMEX_ERROR_SHM_OPEN;
      goto shmex_create_exit;
    }
    shmex_generate_name(payload);
    fd = shm_open(payload->name, O_RDWR | O_CREAT | O_EXCL, 0666);
  }

  int ftr_res = ftruncate(fd, payload->capacity);
  if (ftr_res < 0) {
    result = SHMEX_ERROR_FTRUNCATE;
    goto shmex_create_exit;
  }

  result = SHMEX_RES_OK;
shmex_create_exit:
  if (fd > 0) {
    close(fd);
  }
  return result;
}

/**
 * Maps shared memory into address space of current process (using mmap)
 *
 * On success sets payload->mapped_memory to a valid pointer. On failure it is
 * set to MAP_FAILED ((void *)-1) and returned result indicates which function
 * failed.
 *
 * Mapped memory has to be released with either 'shmex_release' or
 * 'shmex_unmap'.
 *
 * While memory is mapped the capacity of shm must not be modified.
 */
ShmexLibResult shmex_open_and_mmap(Shmex *payload) {
  ShmexLibResult result;
  int fd = -1;

  fd = shm_open(payload->name, O_RDWR, 0666);
  if (fd < 0) {
    result = SHMEX_ERROR_SHM_OPEN;
    goto shmex_open_and_mmap_exit;
  }

  payload->mapped_memory =
      mmap(NULL, payload->capacity, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (MAP_FAILED == payload->mapped_memory) {
    result = SHMEX_ERROR_MMAP;
    goto shmex_open_and_mmap_exit;
  }

  result = SHMEX_RES_OK;
shmex_open_and_mmap_exit:
  if (fd > 0) {
    close(fd);
  }
  return result;
}

void shmex_unmap(Shmex *payload) {
  if (payload->mapped_memory != MAP_FAILED) {
    munmap(payload->mapped_memory, payload->capacity);
  }
  payload->mapped_memory = MAP_FAILED;
}

/**
 * Sets the capacity of shared memory payload. The struct is updated
 * accordingly.
 *
 * Should not be invoked when shm is mapped into the memory.
 */
ShmexLibResult shmex_set_capacity(Shmex *payload, size_t capacity) {
  ShmexLibResult result;
  int fd = -1;

  if (payload->mapped_memory != MAP_FAILED) {
    result = SHMEX_ERROR_SHM_MAPPED;
    goto shmex_set_capacity_exit;
  }

  fd = shm_open(payload->name, O_RDWR, 0666);
  if (fd < 0) {
    result = SHMEX_ERROR_SHM_OPEN;
    goto shmex_set_capacity_exit;
  }

  int res = ftruncate(fd, capacity);
  if (res < 0) {
    result = SHMEX_ERROR_FTRUNCATE;
    goto shmex_set_capacity_exit;
  }
  payload->capacity = capacity;
  if (payload->size > capacity) {
    // data was discarded with ftruncate, update size
    payload->size = capacity;
  }
  result = SHMEX_RES_OK;
shmex_set_capacity_exit:
  if (fd > 0) {
    close(fd);
  }
  return result;
}

/**
 * Unlinks shared memory segment. Unlinked segment cannot be mapped again and is
 * freed once all its memory mappings are removed (e.g. via `shmex_release`
 * function). This function has to be called **before** `shmex_release`.
 */
ShmexLibResult shmex_unlink(Shmex *payload) {
  if (payload->name != NULL) {
    shm_unlink(payload->name);
    return SHMEX_RES_OK;
  } else {
    return SHMEX_ERROR_INVALID_PAYLOAD;
  }
}

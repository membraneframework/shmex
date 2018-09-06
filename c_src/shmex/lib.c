#include "lib.h"

void shmex_generate_name(Shmex * payload) {
  static const unsigned GENERATED_NAME_SIZE = strlen(SHM_NAME_PREFIX) + 21;
  if (payload->name != NULL) {
    enif_free(payload->name);
  }
  payload->name = enif_alloc(GENERATED_NAME_SIZE);

  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  snprintf(payload->name, GENERATED_NAME_SIZE, SHM_NAME_PREFIX "%.12ld%.8ld", ts.tv_sec, ts.tv_nsec);
}

/**
 * Initializes Shmex C struct. Should be used before allocating shm from C code.
 *
 * Each call should be paired with `shmex_release` call to deallocate resources.
 */
void shmex_init(ErlNifEnv * env, Shmex * payload, unsigned capacity) {
  payload->guard = enif_make_atom(env, "nil");
  payload->size = 0;
  payload->capacity = capacity;
  payload->mapped_memory = MAP_FAILED;
  payload->elixir_struct_atom = enif_make_atom(env, SHMEX_ELIXIR_STRUCT_ATOM);
  payload->name = NULL;
}

/**
 * Initializes Shmex C struct using data from Shmex Elixir struct
 *
 * Each call should be paired with `shmex_release` call to deallocate resources
 */
int shmex_get_from_term(ErlNifEnv * env, ERL_NIF_TERM struct_term, Shmex *payload) {
  const ERL_NIF_TERM ATOM_STRUCT_TAG = enif_make_atom(env, "__struct__");
  const ERL_NIF_TERM ATOM_NAME = enif_make_atom(env, "name");
  const ERL_NIF_TERM ATOM_GUARD = enif_make_atom(env, "guard");
  const ERL_NIF_TERM ATOM_SIZE = enif_make_atom(env, "size");
  const ERL_NIF_TERM ATOM_CAPACITY = enif_make_atom(env, "capacity");

  int result;
  ERL_NIF_TERM tmp_term;

  payload->mapped_memory = MAP_FAILED;

  // Get guard
  result = enif_get_map_value(env, struct_term, ATOM_GUARD, &tmp_term);
  if (!result) {
    return 0;
  }
  payload->guard = tmp_term;

  // Get Elixir struct tag
  result = enif_get_map_value(env, struct_term, ATOM_STRUCT_TAG, &tmp_term);
  if (!result) {
    return 0;
  }
  payload->elixir_struct_atom = tmp_term;

  // Get size
  result = enif_get_map_value(env, struct_term, ATOM_SIZE, &tmp_term);
  if (!result) {
    return 0;
  }
  result = enif_get_uint(env, tmp_term, &payload->size);
  if (!result) {
    return 0;
  }

  // Get capacity
  result = enif_get_map_value(env, struct_term, ATOM_CAPACITY, &tmp_term);
  if (!result) {
    return 0;
  }
  result = enif_get_uint(env, tmp_term, &payload->capacity);
  if (!result) {
    return 0;
  }

  // Get name as last to prevent failure after allocating memory
  result = enif_get_map_value(env, struct_term, ATOM_NAME, &tmp_term);
  if (!result) {
    return 0;
  }
  char atom_tmp[4];
  result = enif_get_atom(env, tmp_term, atom_tmp, 4, ERL_NIF_LATIN1);
  if (result) {
    if (strncmp(atom_tmp, "nil", 3) == 0) {
      payload->name = NULL;
      return 1;
    }

    return 0;
  }

  ErlNifBinary name_binary;
  result = enif_inspect_binary(env, tmp_term, &name_binary);
  if (!result) {
    return 0;
  }
  payload->name = enif_alloc(name_binary.size + 1);
  memcpy(payload->name, (char *) name_binary.data, name_binary.size);
  payload->name[name_binary.size] = '\0';

  return 1;
}

/**
 * Allocates POSIX shared memory given the data (name, capacity) in Shmex struct.
 *
 * If name in Shmex is set to NULL, the name will be (re)genrated until
 * the one that haven't been used is found (at most SHMEX_ALLOC_MAX_ATTEMPTS times).
 *
 * Shared memory can be accessed by using 'shmex_open_and_mmap'.
 * Memory will be unmapped when Shmex struct is freed (by 'shmex_release')
 */
ShmexLibResult shmex_allocate(Shmex * payload) {
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
      result = shmex_ERROR_SHM_OPEN;
      goto shmex_create_exit;
    }
    shmex_generate_name(payload);
    fd = shm_open(payload->name, O_RDWR | O_CREAT | O_EXCL, 0666);
  }

  int ftr_res = ftruncate(fd, payload->capacity);
  if (ftr_res < 0) {
    result = shmex_ERROR_FTRUNCATE;
    goto shmex_create_exit;
  }

  result = shmex_RES_OK;
shmex_create_exit:
  if (fd > 0) {
    close(fd);
  }
  return result;
}

/**
 * Deallocates resources owned by Shmex struct. It does not
 * free the actual shared memory segment, just object representing it.
 *
 * If payload was mapped, unmaps it as well.
 */
void shmex_release(Shmex *payload) {
  if (payload->name != NULL) {
    enif_free(payload->name);
  }

  shmex_unmap(payload);
}

/**
 * Creates Shmex Elixir struct from Shmex C struct
 */
ERL_NIF_TERM shmex_make_term(ErlNifEnv * env, Shmex * payload) {
  ERL_NIF_TERM keys[SHMEX_ELIXIR_STRUCT_ENTRIES] = {
    enif_make_atom(env, "__struct__"),
    enif_make_atom(env, "name"),
    enif_make_atom(env, "guard"),
    enif_make_atom(env, "size"),
    enif_make_atom(env, "capacity")
  };

  ERL_NIF_TERM name_term;
  unsigned name_len = strlen(payload->name);
  void * name_ptr = enif_make_new_binary(env, name_len, &name_term);
  memcpy(name_ptr, payload->name, name_len);

  ERL_NIF_TERM values[SHMEX_ELIXIR_STRUCT_ENTRIES] = {
    payload->elixir_struct_atom,
    name_term,
    payload->guard,
    enif_make_int(env, payload->size),
    enif_make_int(env, payload->capacity)
  };

  ERL_NIF_TERM return_term;
  int res = enif_make_map_from_arrays(env, keys, values, SHMEX_ELIXIR_STRUCT_ENTRIES, &return_term);
  if (res) {
    return return_term;
  } else {
    return bunch_make_error_internal(env, "make_map_from_arrays");
  }
}


/**
 * Creates term describing an error encoded in result (ShmexLibResult)
 */
ERL_NIF_TERM shmex_make_error_term(ErlNifEnv * env, ShmexLibResult result) {
  switch (result) {
    case shmex_RES_OK:
      return bunch_make_error_internal(env, "ok_is_not_error");
    case shmex_ERROR_SHM_OPEN:
      return bunch_make_error_errno(env, "shm_open");
    case shmex_ERROR_FTRUNCATE:
      return bunch_make_error_errno(env, "ftruncate");
    case shmex_ERROR_MMAP:
      return bunch_make_error_errno(env, "mmap");
    case shmex_ERROR_SHM_MAPPED:
      return bunch_make_error_internal(env, "shm_is_mapped");
    default:
      return bunch_make_error_internal(env, "unknown_error");
  }
}

/**
 * Sets the capacity of shared memory payload. The struct is updated accordingly.
 *
 * Should not be invoked when shm is mapped into the memory.
 */
ShmexLibResult shmex_set_capacity(Shmex * payload, size_t capacity) {
  ShmexLibResult result;
  int fd = -1;

  if (payload->mapped_memory != MAP_FAILED) {
    result = shmex_ERROR_SHM_MAPPED;
    goto shmex_set_capacity_exit;
  }

  fd = shm_open(payload->name, O_RDWR, 0666);
  if (fd < 0) {
    result = shmex_ERROR_SHM_OPEN;
    goto shmex_set_capacity_exit;
  }

  int res = ftruncate(fd, capacity);
  if (res < 0) {
    result = shmex_ERROR_FTRUNCATE;
    goto shmex_set_capacity_exit;
  }
  payload->capacity = capacity;
  if (payload->size > capacity) {
    // data was discarded with ftruncate, update size
    payload->size = capacity;
  }
  result = shmex_RES_OK;
shmex_set_capacity_exit:
  if (fd > 0) {
    close(fd);
  }
  return result;
}

/**
 * Maps shared memory into address space of current process (using mmap)
 *
 * On success sets payload->mapped_memory to a valid pointer. On failure it is set to
 * MAP_FAILED ((void *)-1) and returned result indicates which function failed.
 *
 * Mapped memory has to be released with either 'shmex_release' or 'shmex_unmap'.
 *
 * While memory is mapped the capacity of shm must not be modified.
 */
ShmexLibResult shmex_open_and_mmap(Shmex * payload) {
  ShmexLibResult result;
  int fd = -1;

  fd = shm_open(payload->name, O_RDWR, 0666);
  if (fd < 0) {
    result = shmex_ERROR_SHM_OPEN;
    goto shmex_open_and_mmap_exit;
  }

  payload->mapped_memory = mmap(NULL, payload->capacity, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (MAP_FAILED == payload->mapped_memory) {
    result = shmex_ERROR_MMAP;
    goto shmex_open_and_mmap_exit;
  }

  result = shmex_RES_OK;
shmex_open_and_mmap_exit:
  if (fd > 0) {
    close(fd);
  }
  return result;
}

void shmex_unmap(Shmex * payload) {
  if (payload->mapped_memory != MAP_FAILED) {
    munmap(payload->mapped_memory, payload->capacity);
  }
  payload->mapped_memory = MAP_FAILED;
}

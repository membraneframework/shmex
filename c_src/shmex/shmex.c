#include "shmex.h"

ErlNifResourceType *RES_shmex_GUARD_TYPE;

void res_shmex_guard_destructor(ErlNifEnv* env, void * resource) {
  UNUSED(env);

  ShmGuard *guard = (ShmGuard *) resource;
  shm_unlink(guard->name);
}

int load(ErlNifEnv* env, void** priv_data, ERL_NIF_TERM load_info) {
  UNUSED(load_info);
  UNUSED(priv_data);

  int flags = ERL_NIF_RT_CREATE | ERL_NIF_RT_TAKEOVER;
  RES_shmex_GUARD_TYPE =
    enif_open_resource_type(env, NULL, "ShmGuard", res_shmex_guard_destructor, flags, NULL);
  return 0;
}

static void create_guard(ErlNifEnv * env, Shmex *payload) {
  ShmGuard *guard = enif_alloc_resource(RES_shmex_GUARD_TYPE, sizeof(*guard));
  strcpy(guard->name, payload->name);
  payload->guard = enif_make_resource(env, guard);
  enif_release_resource(guard);
}

static ERL_NIF_TERM export_allocate(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  UNUSED(argc);
  MEMBRANE_UTIL_PARSE_shmex_ARG(0, payload);
  ERL_NIF_TERM return_term;

  ShmexLibResult result = shmex_allocate(&payload);

  if (shmex_RES_OK == result) {
    create_guard(env, &payload);
    return_term = membrane_util_make_ok_tuple(env, shmex_make_term(env, &payload));
  } else {
    return_term = shmex_make_error_term(env, result);
  }

  shmex_release(&payload);
  return return_term;
}

static ERL_NIF_TERM export_add_guard(ErlNifEnv * env, int argc, const ERL_NIF_TERM argv[]) {
  UNUSED(argc);
  MEMBRANE_UTIL_PARSE_shmex_ARG(0, payload);

  ShmGuard * guard;
  if (enif_get_resource(env, payload.guard, RES_shmex_GUARD_TYPE, (void **) &guard)) {
    return membrane_util_make_error(env, enif_make_atom(env, "already_guarded"));
  };
  create_guard(env, &payload);
  return membrane_util_make_ok_tuple(env, shmex_make_term(env, &payload));
}

static ERL_NIF_TERM export_set_capacity(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  UNUSED(argc);
  MEMBRANE_UTIL_PARSE_shmex_ARG(0, payload);
  MEMBRANE_UTIL_PARSE_UINT_ARG(1, capacity);
  ERL_NIF_TERM return_term;

  ShmexLibResult result = shmex_set_capacity(&payload, capacity);
  if (shmex_RES_OK == result) {
    return_term = membrane_util_make_ok_tuple(env, shmex_make_term(env, &payload));
  } else {
    return_term = shmex_make_error_term(env, result);
  }
  shmex_release(&payload);
  return return_term;
}

static ERL_NIF_TERM export_read(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  UNUSED(argc);
  MEMBRANE_UTIL_PARSE_shmex_ARG(0, payload);
  MEMBRANE_UTIL_PARSE_UINT_ARG(1, cnt);

  ERL_NIF_TERM return_term;
  if (cnt > payload.size) {
    return_term = membrane_util_make_error_args(env, "cnt", "cnt is greater than payload size");
    goto exit_read;
  }

  ShmexLibResult result = shmex_open_and_mmap(&payload);
  if (shmex_RES_OK != result) {
    return_term = shmex_make_error_term(env, result);
    goto exit_read;
  }

  ERL_NIF_TERM out_bin_term;
  unsigned char * output_data = enif_make_new_binary(env, cnt, &out_bin_term);
  memcpy(output_data, payload.mapped_memory, cnt);

  return_term = membrane_util_make_ok_tuple(env, out_bin_term);
exit_read:
  shmex_release(&payload);
  return return_term;
}

static ERL_NIF_TERM export_write(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  UNUSED(argc);
  MEMBRANE_UTIL_PARSE_shmex_ARG(0, payload);
  MEMBRANE_UTIL_PARSE_BINARY_ARG(1, data);
  ERL_NIF_TERM return_term;

  if (payload.capacity < data.size) {
    shmex_set_capacity(&payload, data.size);
  }

  ShmexLibResult result = shmex_open_and_mmap(&payload);
  if (shmex_RES_OK != result) {
    return_term = shmex_make_error_term(env, result);
    goto exit_write;
  }

  memcpy(payload.mapped_memory, (void *)data.data, data.size);
  payload.size = data.size;
  return_term = membrane_util_make_ok_tuple(env, shmex_make_term(env, &payload));
exit_write:
  shmex_release(&payload);
  return return_term;
}

static ERL_NIF_TERM export_split_at(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  UNUSED(argc);
  MEMBRANE_UTIL_PARSE_shmex_ARG(0, old_payload);
  MEMBRANE_UTIL_PARSE_UINT_ARG(1, split_pos);
  Shmex new_payload;
  shmex_init(env, &new_payload, 4096);

  ERL_NIF_TERM return_term;
  int new_fd = -1;

  ShmexLibResult result = shmex_open_and_mmap(&old_payload);
  if (shmex_RES_OK != result) {
    return_term = shmex_make_error_term(env, result);
    goto exit_split_at;
  }

  int new_size = old_payload.size - split_pos;
  new_payload.capacity = new_size;
  new_payload.size = new_size;

  result = shmex_allocate(&new_payload);
  create_guard(env, &new_payload);
  if (shmex_RES_OK != result) {
    return_term = shmex_make_error_term(env, result);
    goto exit_split_at;
  }

  result = shmex_open_and_mmap(&new_payload);
  if (shmex_RES_OK != result) {
    return_term = shmex_make_error_term(env, result);
    goto exit_split_at;
  }

  memcpy(new_payload.mapped_memory, old_payload.mapped_memory + split_pos, new_size);

  old_payload.size = split_pos;

  return_term = membrane_util_make_ok_tuple(
    env,
    enif_make_tuple2(
      env,
      shmex_make_term(env, &old_payload),
      shmex_make_term(env, &new_payload)
    )
  );

exit_split_at:
  if (new_fd > 0) {
    close(new_fd);
  }
  shmex_release(&old_payload);
  shmex_release(&new_payload);
  return return_term;
}

static ERL_NIF_TERM export_trim_leading(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  UNUSED(argc);
  MEMBRANE_UTIL_PARSE_shmex_ARG(0, payload);
  MEMBRANE_UTIL_PARSE_UINT_ARG(1, offset);
  ERL_NIF_TERM return_term;
  ShmexLibResult result;

  result = shmex_open_and_mmap(&payload);
  if (shmex_RES_OK != result) {
    return_term = shmex_make_error_term(env, result);
    goto exit_trim_leading;
  }

  size_t new_size = payload.size - offset;
  memmove(payload.mapped_memory, payload.mapped_memory + offset, new_size);
  payload.size = new_size;
  return_term = membrane_util_make_ok_tuple(env, shmex_make_term(env, &payload));
exit_trim_leading:
  shmex_release(&payload);
  return return_term;
}

static ERL_NIF_TERM export_concat(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  UNUSED(argc);
  MEMBRANE_UTIL_PARSE_shmex_ARG(0, left);
  MEMBRANE_UTIL_PARSE_shmex_ARG(1, right);
  ERL_NIF_TERM return_term;
  ShmexLibResult result;

  size_t new_capacity = left.size + right.size;
  result = shmex_set_capacity(&left, new_capacity);
  if (shmex_RES_OK != result) {
    return_term = shmex_make_error_term(env, result);
    goto exit_concat;
  }

  result = shmex_open_and_mmap(&left);
  if (shmex_RES_OK != result) {
    return_term = shmex_make_error_term(env, result);
    goto exit_concat;
  }

  result = shmex_open_and_mmap(&right);
  if (shmex_RES_OK != result) {
    return_term = shmex_make_error_term(env, result);
    goto exit_concat;
  }

  memcpy(left.mapped_memory + left.size, right.mapped_memory, right.size);
  left.size = new_capacity;
  return_term = membrane_util_make_ok_tuple(env, shmex_make_term(env, &left));
exit_concat:
  shmex_release(&left);
  shmex_release(&right);
  return return_term;
}

static ErlNifFunc nif_funcs[] = {
  {"allocate", 1, export_allocate, 0},
  {"add_guard", 1, export_add_guard, 0},
  {"set_capacity", 2, export_set_capacity, 0},
  {"read", 2, export_read, 0},
  {"write", 2, export_write, 0},
  {"split_at", 2, export_split_at, 0},
  {"concat", 2, export_concat, 0},
  {"trim_leading", 2, export_trim_leading, 0}
};

ERL_NIF_INIT(Elixir.Shmex.Native.Nif, nif_funcs, load, NULL, NULL, NULL)
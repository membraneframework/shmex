#define SHMEX_CNODE
#include "../../common/lib.h"
#include <ei.h>

#define NAME_MAX 255

void shmex_init(Shmex * payload, unsigned capacity);
int shmex_deserialize(const char* buf, int* idx, Shmex *payload);
void shmex_release(Shmex *payload);
// ERL_NIF_TERM shmex_make_term(ErlNifEnv * env, Shmex * payload);
// ERL_NIF_TERM shmex_make_error_term(ErlNifEnv * env, ShmexLibResult result);

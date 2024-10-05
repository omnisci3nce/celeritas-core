#pragma once

#include "defines.h"
#include "str.h"

// -- Paths
typedef struct path_opt {
  Str8 path;
  bool has_value;
} path_opt;

// TODO: convert to using str8
// TODO: use uppercase code style
path_opt path_parent(arena* a, const char* path);

// --- Threads
typedef struct CelThread CelThread;

CelThread Thread_Create();
void Thread_Destroy(CelThread* thread);

// --- Mutexes
typedef struct CelMutex CelMutex;

CelMutex Mutex_Create();
void Mutex_Destroy(CelMutex* mutex);

/** @brief Blocks until the mutex can be acquired. if returns false then an error occurred and can
 * be checked (TODO) */
bool Mutex_Lock(CelMutex* mutex);

/** @brief Tries to acquire the mutex like `mutex_lock` but returns immediately if the mutex has
 * already been locked */
bool Mutex_TryLock(CelMutex* mutex);

/** @brief Releases a mutex. If it is already unlocked then does nothing */
void Mutex_Unlock(CelMutex* mutex);

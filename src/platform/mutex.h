/**
 * @file mutex.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-04-27
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <stdbool.h>

typedef struct cel_mutex cel_mutex;

cel_mutex mutex_create();

void mutex_destroy(cel_mutex* mutex);

/** @brief Blocks until the mutex can be acquired. if returns false then an error occurred and can be checked (TODO) */
bool mutex_lock(cel_mutex* mutex);

/** @brief Tries to acquire the mutex like `mutex_lock` but returns immediately if the mutex has already been locked */
bool mutex_try_lock(cel_mutex* mutex);

/** @brief Releases a mutex. If it is already unlocked then does nothing */
void mutex_unlock(cel_mutex* mutex);
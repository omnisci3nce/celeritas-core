/**
  A Threadpool has a number of "workers", each which process "tasks"
*/
#pragma once

#include <pthread.h>

#include "darray.h"
#include "defines.h"
#include "ring_queue.h"

#define MAX_NUM_THREADS 16

struct threadpool;
typedef struct threadpool threadpool;

typedef struct task_globals {
  threadpool *pool;
  void *ctx;
} task_globals;

/* function pointer */
typedef bool (*tpool_task_start)(void *, void *);

/* function pointer */
typedef void (*tpool_task_on_complete)(task_globals *, void *);

typedef struct threadpool_worker {
  u16 id;
  pthread_t thread;
  threadpool *pool;  // pointer back to the pool so we can get the mutex and cond
} threadpool_worker;

typedef enum tpool_task_status {
  TASK_STATUS_READY,
} task_status;

typedef struct tpool_task {
  u64 task_id;
  tpool_task_start do_task;
  tpool_task_on_complete on_success;
  tpool_task_on_complete on_failure;
  bool buffer_result_for_main_thread;
  /** @brief a pointer to the parameters data that will be passed into the task. */
  void *params;
  u32 param_size;
  void *result_data;
  u32 result_data_size;
} task;

typedef struct deferred_task_result {
  u64 task_id;
  tpool_task_on_complete callback;
  u32 result_data_size;
  // this gets passed to the void* argument of `tpool_task_on_complete`
  void *result_data;
} deferred_task_result;

#ifndef TYPED_TASK_RESULT_ARRAY
KITC_DECL_TYPED_ARRAY(deferred_task_result)  // creates "deferred_task_result_darray"
#define TYPED_TASK_RESULT_ARRAY
#endif

struct threadpool {
  ring_queue *task_queue;
  pthread_mutex_t mutex;
  pthread_cond_t has_tasks;
  threadpool_worker workers[MAX_NUM_THREADS];
  deferred_task_result_darray *results;
  u64 next_task_id;

  void *context;
};

/**
 * @param pool where to store the created threadpool
 * @param thread_count how many threads to spawn
 * @param queue_size max size of task queue
 */
bool threadpool_create(threadpool *pool, u8 thread_count, u32 queue_size);
void threadpool_destroy(threadpool *pool);

/** @brief set a context variable for the threadpool that task data has access to */
void threadpool_set_ctx(threadpool *pool, void *ctx);

/**
 * @brief Add a task to the threadpool
 */
bool threadpool_add_task(threadpool *pool, tpool_task_start do_task,
                         tpool_task_on_complete on_success, tpool_task_on_complete on_fail,
                         bool buffer_result_for_main_thread, void *param_data, u32 param_data_size,
                         u32 result_data_size);

void threadpool_process_results(threadpool *pool, int num_to_process);

u32 Tpool_GetNumWorkers();  // how many threads are we using
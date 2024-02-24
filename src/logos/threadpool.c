#include "threadpool.h"

#include <pthread.h>

#include "defines.h"
#include "log.h"
#include "ring_queue.h"

static void *worker_factory(void *arg) {
  threadpool_worker *worker = arg;
  //   INFO("Starting job thread %d", worker->id);

  // Run forever, waiting for jobs.
  while (true) {
    pthread_mutex_lock(&worker->pool->mutex);
    pthread_cond_wait(&worker->pool->has_tasks, &worker->pool->mutex);  // wait for work to be ready

    task t;
    if (ring_queue_dequeue(worker->pool->task_queue, &t)) {
      //   DEBUG("Job thread %d picked up task %d", worker->id, t.task_id);
    } else {
      //   WARN("Job thread %d didnt pick up a task as queue was empty", worker->id);
      pthread_mutex_unlock(&worker->pool->mutex);
      break;
    }

    pthread_mutex_unlock(&worker->pool->mutex);

    // Do the work
    bool result = t.do_task(t.params, t.result_data);

    // INFO("Task result was %s", result ? "success" : "failure");
    if (result) {
      pthread_mutex_lock(&worker->pool->mutex);
      if (t.buffer_result_for_main_thread) {
        deferred_task_result dtr = { .task_id = t.task_id,
                                     .callback = t.on_success,
                                     .result_data = t.result_data,
                                     .result_data_size = t.result_data_size };
        deferred_task_result_darray_push(worker->pool->results, dtr);
      } else {
        // call on complete from here.
      }
    } else {
      // TODO
    }
    pthread_mutex_unlock(&worker->pool->mutex);
  }

  return NULL;
}

bool threadpool_create(threadpool *pool, u8 thread_count, u32 queue_size) {
  INFO("Threadpool init");
  pool->next_task_id = 0;
  pool->context = NULL;

  u8 num_worker_threads = thread_count;
  if (thread_count > MAX_NUM_THREADS) {
    ERROR_EXIT("Threadpool has a hard limit of %d threads, you tried to start one with %d",
               MAX_NUM_THREADS, thread_count)
    num_worker_threads = MAX_NUM_THREADS;
  }

  DEBUG("creating task queue with max length %d", queue_size);
  pool->task_queue = ring_queue_new(sizeof(task), queue_size, NULL);

  DEBUG("creating mutex and condition");
  pthread_mutex_init(&pool->mutex, NULL);
  pthread_cond_init(&pool->has_tasks, NULL);

  pool->results = deferred_task_result_darray_new(256);

  DEBUG("Spawning %d threads for the threadpool", thread_count);
  for (u8 i = 0; i < num_worker_threads; i++) {
    pool->workers[i].id = i;
    pool->workers[i].pool = pool;
    if (pthread_create(&pool->workers[i].thread, NULL, worker_factory, &pool->workers[i]) != 0) {
      FATAL("OS error creating job thread");
      return false;
    };
  }

  return true;
}

bool threadpool_add_task(threadpool *pool, tpool_task_start do_task,
                         tpool_task_on_complete on_success, tpool_task_on_complete on_fail,
                         bool buffer_result_for_main_thread, void *param_data, u32 param_data_size,
                         u32 result_data_size) {
  void *result_data = malloc(result_data_size);

  task *work_task = malloc(sizeof(task));
  work_task->task_id = 0;
  work_task->do_task = do_task;
  work_task->on_success = on_success;
  work_task->on_failure = on_fail;
  work_task->buffer_result_for_main_thread = buffer_result_for_main_thread;
  work_task->param_size = param_data_size;
  work_task->params = param_data;
  work_task->result_data_size = result_data_size;
  work_task->result_data = result_data;

  // START critical section
  if (pthread_mutex_lock(&pool->mutex) != 0) {
    ERROR("Unable to get threadpool lock.");
    return false;
  }

  work_task->task_id = pool->next_task_id;
  pool->next_task_id++;

  ring_queue_enqueue(pool->task_queue, work_task);
  DEBUG("Enqueued job");
  pthread_cond_broadcast(&pool->has_tasks);

  if (pthread_mutex_unlock(&pool->mutex) != 0) {
    ERROR("couldnt unlock threadpool after adding task.");
    return false;  // ?
  }
  // END critical section

  return true;
}

void threadpool_process_results(threadpool *pool, int _num_to_process) {
  pthread_mutex_lock(&pool->mutex);
  size_t num_results = deferred_task_result_darray_len(pool->results);
  if (num_results > 0) {
    u32 _size = ((deferred_task_result *)pool->results->data)[num_results].result_data_size;
    deferred_task_result res;
    deferred_task_result_darray_pop(pool->results, &res);
    pthread_mutex_unlock(&pool->mutex);
    task_globals globals = { .pool = pool, .ctx = pool->context };
    res.callback(&globals, res.result_data);
  } else {
    pthread_mutex_unlock(&pool->mutex);
  }
}

void threadpool_set_ctx(threadpool *pool, void *ctx) { pool->context = ctx; }
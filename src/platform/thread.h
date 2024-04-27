/**
 * @file thread.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-04-27
 *
 * @copyright Copyright (c) 2024
 *
 */

typedef struct cel_thread cel_thread;

cel_thread thread_create();
void thread_destroy(cel_thread* thread);

// join
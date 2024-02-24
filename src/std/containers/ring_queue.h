/**
 * @file ring_queue.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-02-24
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once
#include "defines.h"

/**
 * @brief a fixed-size ring queue
 */
typedef struct ring_queue {
  size_t len;
  size_t capacity;
  size_t type_size;
  void* data;
  bool owns_memory;
  int32_t head;
  int32_t tail;
} ring_queue;

ring_queue* ring_queue_new(size_t type_size, size_t capacity, void* memory_block);

void ring_queue_free(ring_queue* queue);

bool ring_queue_enqueue(ring_queue* queue, const void* value);

bool ring_queue_dequeue(ring_queue* queue, void* out_value);

bool ring_queue_peek(const ring_queue* queue, void* out_value);
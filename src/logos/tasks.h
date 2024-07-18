/**
 * Common jobs that get run
 */
#pragma once
#include "defines.h"
#include "logos/threadpool.h"
#include "render_types.h"
#include "str.h"

typedef struct Task {
    char* debug_name;
    void* params;
    bool is_done;
} Task;

// Macro : give Params and Result structs and it creates a function that knows
//         correct sizes

typedef struct Task_ModelLoad_Params {
   Str8 filepath;     // filepath to the model on disk
 } Task_ModelLoad_Params;
typedef struct Task_ModelLoad_Result {
    Model model;
} Task_ModelLoad_Result;

// Internally it will allocate data for each

static bool Task_ModelLoad_Typed(
    Task_ModelLoad_Params* params,
    Task_ModelLoad_Result* result,
    tpool_task_start run_task,
    tpool_task_on_complete on_success,
    tpool_task_on_complete on_failure
) {
    threadpool_add_task(pool, , tpool_task_on_complete on_success, tpool_task_on_complete on_fail, bool buffer_result_for_main_thread, void *param_data, u32 param_data_size, u32 result_data_size)
}

// do task
// success
void model_load_success(task_globals* globals, void* result) {
    Task_ModelLoad_Result* load_res = result;

    // push into render -> renderables ?
}
// fail


// we can define our custom task here that wraps the more verbose function pointers
static Task Task_ModelLoad(
    Task_ModelLoad_Params* params,
    Task_ModelLoad_Result* result
) {
    Task task;
    task.debug_name = "Load Model";
    task.params = params;

    Task_ModelLoad_Typed(params, result, tpool_task_start run_task, tpool_task_on_complete on_success, tpool_task_on_complete on_failure)

    return task;
}

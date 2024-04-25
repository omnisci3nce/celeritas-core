// External API

typedef struct core core;

// Implicit global static core?

void cel_input_update(core* c);
void cel_threadpool_update(core* c);
void cel_render_frame_begin(core* c);
void cel_render_frame_end(core* c);